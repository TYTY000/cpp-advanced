#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef N
#define N 5
#endif
#ifndef FS
#define FS 38
#endif

struct node {
    int data;
    int fibdata;
    struct node* next;
};

int fib(int n) {
    int x, y;
    if (n < 2) {
        return (n);
    } else {
    }
    int a = 0, b = 1, c;
    for (int i = 2; i <= n; i++) {
        c = a + b;
        a = b;
        b = c;
    }
    return b;
}

void processwork(struct node* p) {
    int n;
    n = p->data;
    p->fibdata = fib(n);
}

struct node* init_list(struct node* p) {
    int i;
    struct node* head = NULL;
    struct node* temp = NULL;

    head = malloc(sizeof(struct node));
    p = head;
    p->data = FS;
    p->fibdata = 0;
    for (i = 0; i < N; i++) {
        temp = malloc(sizeof(struct node));
        p->next = temp;
        p = temp;
        p->data = FS + i + 1;
        p->fibdata = i + 1;
    }
    p->next = NULL;
    return head;
}

int main(int argc, char* argv[]) {
    double start, end;
    struct node* p = NULL;
    struct node* temp = NULL;
    struct node* head = NULL;
    struct node* nodes[N + 1];

    printf("Process linked list\n");
    printf(
        "  Each linked list node will be processed by function "
        "'processwork()'\n");
    printf(
        "  Each ll node will compute %d fibonacci numbers beginning with %d\n",
        N, FS);

    p = init_list(p);
    head = p;
    size_t i;
    for (i = 0, p = head; p != NULL; i++, p = p->next) {
        nodes[i] = p;
    }

    start = omp_get_wtime();
#pragma omp parallel
    {
#pragma omp single
        {
            for (p = head; p != NULL; p = p->next) {
#pragma omp task firstprivate(p)
                processwork(p);
            }
        }
    }

    end = omp_get_wtime();
    p = head;
    while (p != NULL) {
        printf("%d : %d\n", p->data, p->fibdata);
        temp = p->next;
        free(p);
        p = temp;
    }
    free(p);

    printf("Compute Time: %f seconds\n", end - start);

    return 0;
}
