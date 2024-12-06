#MPI 
MPI是一个通用框架，需要使用单独的包装编译器mpic++，运行也需要使用单独的运行mpirun并且指定运行参数。
![[prog_structure.gif]]
1.需要额外使用MPI初始化和停止语句来声明并行代码区域；
2.额外注意，MPI中使用的是C风格的接口（指定变量赋值需要取地址）；
3.和CUDA类似，每个进程需要通过指定接口来获取num_thread和threadID；
```c++
int main(int argc, char *argv[]) {
    int numtasks, rank, source = 0, dest, tag = 1;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
//………………………………
	MPI_Finalize();
}
```

4.MPI支持自定义组和拓扑进行内部通信（有点类似计算机网络的子网），可以收发、广播、规约和交换，但都需要指定目标/源ID和组/拓扑，组和拓扑是由对象进行单独管理的（类似pthread的p_cpu_set），保证安全通信；这个对象communicator是动态创建和销毁的，每个对象内部的进程有单独的rank，默认全局的communicator是MPI_COMM_WORLD；

	未定义进程是MPI_PROC_NULL，在收发时如果目标是这个会自动忽略。
```c++
	inbuf[4] =
        {
            MPI_PROC_NULL,
            MPI_PROC_NULL,
            MPI_PROC_NULL,
            MPI_PROC_NULL,
        }
```

	未定义请求是MPI_REQUEST_NULL。
```c++
        MPI_Request reqs[NREPS]{MPI_REQUEST_NULL};
```

#阻塞收发
```c++
        /* determine partner and then send/receive with partner */
        if (taskid < numtasks / 2) {
            partner = numtasks / 2 + taskid;
            MPI_Send(&taskid, 1, MPI_INT, partner, 1, MPI_COMM_WORLD);
            MPI_Recv(&message, 1, MPI_INT, partner, 1, MPI_COMM_WORLD, &status);
        } else if (taskid >= numtasks / 2) {
            partner = taskid - numtasks / 2;
            MPI_Recv(&message, 1, MPI_INT, partner, 1, MPI_COMM_WORLD, &status);
            MPI_Send(&taskid, 1, MPI_INT, partner, 1, MPI_COMM_WORLD);
        }
```

#非阻塞收发
这个是非堵塞的写法，接口中的状态转换成了request，然后需要再waitall中声明几个请求，并给出对应数量的请求和状态。
需要注意的就是最后一个参数从状态换成了请求。
```C++
    MPI_Request reqs[N];
    MPI_Status stats[N];
    // ................
        /* determine partner and then send/receive with partner */
        if (taskid < numtasks / 2)
            partner = numtasks / 2 + taskid;
        else if (taskid >= numtasks / 2)
            partner = taskid - numtasks / 2;
            
		MPI_Irecv(&message, 1, MPI_INT, partner, 1, MPI_COMM_WORLD, &reqs[0]);
        MPI_Isend(&taskid, 1, MPI_INT, partner, 1, MPI_COMM_WORLD, &reqs[1]);

        /* now block until requests are complete */
        MPI_Waitall(2, reqs, stats);
```

> [!NOTE] 注意
> 不要同时抢占初始化前的全局变量的reqs和stats数组！



#组
典型的用法：
使用 MPI_Comm_group 从 MPI_COMM_WORLD 中提取全局组的句柄
使用 MPI_Group_incl 将新组形成为全局组的子集
使用 MPI_Comm_create 为新组创建新通信器
使用 MPI_Comm_rank 确定新通信器中的新等级
使用任何 MPI 消息传递例程进行通信
完成后，使用 MPI_Comm_free 和 MPI_Group_free 释放新的通信器和组（可选）
```c++
// 把进程对半分组
    MPI_Group orig_group, new_group;  // required variables
    MPI_Comm new_comm;                // required variable

    // extract the original group handle
    MPI_Comm_group(MPI_COMM_WORLD, &orig_group);

    //  divide tasks into two distinct groups based upon rank
    if (rank < NPROCS / 2) {
        MPI_Group_incl(orig_group, NPROCS / 2, ranks1, &new_group);
        // 从原来的组中抽几个进程（数组指针附带下标）到新的组中
    } else {
        MPI_Group_incl(orig_group, NPROCS / 2, ranks2, &new_group);
    }

    // create new new communicator and then perform collective communications
    MPI_Comm_create(MPI_COMM_WORLD, new_group, &new_comm);
    MPI_Allreduce(&sendbuf, &recvbuf, 1, MPI_INT, MPI_SUM, new_comm); // 随便一个通信例程

    // get rank in new group
    MPI_Group_rank(new_group, &new_rank);
    printf("rank= %d newrank= %d recvbuf= %d\n", rank, new_rank, recvbuf);
    MPI_Group_free(&new_group);
    MPI_Comm_free(&new_comm);
//……………………
```

#拓扑
```c++
#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3
        // create cartesian virtual topology, get rank, coordinates, neighbor ranks
        MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, reorder, &cartcomm);
        // 在通信器中创建一个二维(*dim = [4,4])的笛卡尔拓扑
        // 不允许头尾环状访问（*periods = [0,0]), 
        // 不允许重新排列进程顺序(reorder = 0)
        // 到新的笛卡尔拓扑通信器中。
        MPI_Comm_rank(cartcomm, &rank);
        MPI_Cart_coords(cartcomm, rank, 2, coords);
        // 将rank转换为2维的拓扑下标数组
        MPI_Cart_shift(cartcomm, 0, 1, &nbrs[UP], &nbrs[DOWN]);
        // 在通信器内找邻居，维度、双向的步长
        MPI_Cart_shift(cartcomm, 1, 1, &nbrs[LEFT], &nbrs[RIGHT]);

        printf("rank= %d coords= %d %d  neighbors(u,d,l,r)= %d %d %d %d\n",
               rank, coords[0], coords[1], nbrs[UP], nbrs[DOWN], nbrs[LEFT],
               nbrs[RIGHT]);

        outbuf = rank;

        // exchange data (rank) with 4 neighbors
        for (i = 0; i < 4; i++) {
            dest = nbrs[i];
            source = nbrs[i];
            MPI_Isend(&outbuf, 1, MPI_INT, dest, tag, MPI_COMM_WORLD, &reqs[i]);
            MPI_Irecv(&inbuf[i], 1, MPI_INT, source, tag, MPI_COMM_WORLD,
                      &reqs[i + 4]);
        }

        MPI_Waitall(8, reqs, stats);
```

#MPI_Comm
> [!NOTE] MPI_COMM_WORLD
> MPI_COMM_WORLD 是一个全局通信器，它是所有 MPI 程序的默认通信组。它包含所有启动时创建的 MPI 进程，是一个类型为 MPI_Comm 的对象。通过这个通信器，进程间可以进行消息传递和通信。
MPI_Comm 类型

MPI_Comm 是通信器类型的定义，它的实例保存了与进程间通信相关的信息。每个进程的 MPI_Comm 对象在 MPI 库内部有所不同，因为每个进程有自己独立的地址空间和通信上下文。

#MPI_Comm对象中保存的信息
对于每个进程，MPI_Comm 对象中保存的信息主要包括：

    进程组信息：包含当前通信器中所有进程的列表及其标识（Global Process ID + rank）。
    
    通信上下文：MPI 中的通信环境。通信上下文定义了消息传递的范围和分界线。它并不仅仅是一个简单的句柄，而是一个包含了各种必要信息的数据结构，用于区分不同通信器的消息，确保消息不会误传到其他通信组。通信上下文中的信息包括了消息的唯一标识符以及用于消息传递的规则和协议。
    
    拓扑信息：并不等同于 MPI_Group，但它确实可以由 MPI_Group 表示。MPI_Group 是 MPI 中定义的一组进程的集合。拓扑信息则是在这些进程组的基础上添加了额外的拓扑结构（如二维网格、环形、树形等）。这使得通信操作更加高效和有序。
    
    操作的属性：通信器中的一些属性，例如错误处理方式、属性键值等。
    
    进程间的关联：包括进程与进程间的映射和通信连接。

#局部操作
在 MPI 中，很多操作确实是针对单个进程的局部操作，但通过 MPI 库的全局协调，这些局部操作能实现进程间的消息传递和数据共享。

    局部操作：每个进程执行自己的局部计算和通信操作。例如，每个进程可以执行局部的计算任务或发送/接收消息。

    MPI库全局协调：MPI 提供了一组丰富的通信函数（如 MPI_Send、MPI_Recv、MPI_Bcast 等），这些函数由 MPI 库全局管理和协调，确保在不同进程间进行有效的通信和数据共享。MPI 库负责处理底层的通信细节，确保消息在正确的进程间传递，并处理同步和数据完整性问题。

    消息传递和数据共享：通过使用 MPI 的通信函数，进程间可以交换数据、同步状态，从而实现复杂的并行计算任务。例如，使用 MPI_Bcast 可以将一个进程的数据广播到所有其他进程，使用 MPI_Reduce 可以对所有进程的数据进行归约计算。

#进程下标的获取
```c++
MPI_Comm_Size(MPI_Comm comm, int *size);
MPI_Comm_Rank(MPI_Comm comm, int *rank);
MPI_Group_Size(MPI_Group group, int *size);
MPI_Group_Rank(MPI_Group group, int *rank);
```