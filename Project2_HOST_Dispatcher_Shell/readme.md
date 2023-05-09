## <center>Project 2：The HOST Dispatcher Shell </center>

<p align="right">曾宸东 520021911345</p>

[TOC]

## 1 Introduction

### 1.1 基本功能

- 实现了四优先级进程调度器
- 实现了内存分配和资源分配算法

### 1.2 运行环境

- Win11 下的 VMware Workstation Pro中运行的Ubuntu20.04



---

## 2 内存分配

本次project中，采用可变分区分配算法。默认内存分配算法为First Fit。此外，我实现了Next Fit, Best Fit, Worst Fit算法。

### 2.1 First Fit

**首次适应算法（First Fit）**将空闲分区链以地址递增的顺序连接；在进行内存分配时，从链首开始顺序查找，直到找到一块分区的大小可以满足需求时，按照该作业的大小，从该分区中分配出内存，将剩下的空闲分区仍然链在空闲分区链中。

<img src="https://img-blog.csdnimg.cn/2020072517214780.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3dlaXhpbl80Mzg4NjU5Mg==,size_16,color_FFFFFF,t_70" alt="First Fit" style="zoom: 80%;" />

```c
if (MabAlgorithm == FIRST_FIT)
    {
        if ((m = memChk(arena, size)) &&
            (m = memSplit(m, size)))
            m->allocated = TRUE;
        return m;
    }
```

**特点：** 该算法倾向于使用内存中低地址部分的空闲区，在高地址部分的空闲区很少被利用，从而保留了高地址部分的大空闲区。显然为以后到达的大作业分配大的内存空间创造了条件。同时，算法开销较小。

**缺点：**低地址部分不断被划分，留下许多难以利用，很小的空闲区，而每次查找又都从低地址部分开始，会增加查找的开销。

### 2.2 Next Fit

**循环首次适应算法（Next Fit）**分配内存时不是从链首进行查找可以分配内存的空闲分区，而是从上一次分配内存的空闲分区的下一个分区开始查找，直到找到可以为该进程分配内存的空闲分区。<center><img src="https://img-blog.csdnimg.cn/2020072517215648.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3dlaXhpbl80Mzg4NjU5Mg==,size_16,color_FFFFFF,t_70" alt="在这里插入图片描述" style="zoom: 80%;" /></center>

```c
if (MabAlgorithm == NEXT_FIT)
    {
        if (next_mab == NULL)
            next_mab = arena;
        // TODO
        if ((next_mab = memChk(next_mab, size)) &&
            (next_mab = memSplit(next_mab, size)))
        {
            next_mab->allocated = TRUE;
            m = next_mab;
            next_mab = next_mab->next;
        }
        return m;
    }
```

**特点：**使内存中的空闲分区分布的更为均匀，减少了查找时的系统开销。同时分配算法开销较小。

**缺点：**缺乏大的空闲分区，从而导致不能装入大型作业。

### 2.3 Best Fit

**最佳适应算法（Best Fit）**寻找能满足进程需要的最小的空闲分区，这样每次找到的空闲分区是和作业大小最接近的，所谓“最佳”。

```c
if (MabAlgorithm == BEST_FIT)
    {
        // TODO
        MabPtr bestmap = NULL;
        int minsize = USER_MEMORY_SIZE;
        next_mab = arena;
        while (m = memChk(next_mab, size))  //找到最小的内存块
        {

            if (m->size <= minsize)
            {
                bestmap = m;
                minsize = m->size;
            }
            next_mab = m->next;
        }
        if (bestmap)						//分配内存
        {
            m = bestmap;
            m = memSplit(m, size);
            m->allocated = TRUE;
            return m;
        }
        else
        {
            return NULL;
        }
    }
```

**特点：**每次分配给文件的都是最合适该文件大小的分区。

**缺点：**孤立的看，这种算法似乎是最优的，但事实上，因为每次分配后剩余的空间一定是最小的，故内存中留下许多难以利用的小的空闲区。且每次遍历整个存储器会带来较大的开销。

### 2.4 Worst Fit

**最坏适应算法(Worst Fit)** 寻找最大的内存空闲区，将该空闲区切分给当前进程

```c
if (MabAlgorithm == WORST_FIT)
    {
        // TODO
        MabPtr worstmap = NULL;
        int maxsize = size;
        next_mab = arena;
        while (m = memChk(next_mab, size))			//找到最大的内存块
        {
            if (m->size >= maxsize)
            {
                worstmap = m;
                maxsize = m->size;
            }
            next_mab = m->next;
        }
        if (worstmap)							//分配内存
        {
            m = worstmap;
            m = memSplit(m, size);
            m->allocated = TRUE;
            return m;
        }
        else
        {
            return NULL;
        }
    }
```

**特点：**给文件分配分区后剩下的空闲区不至于太小，产生碎片的几率最小，对中小型文件分配分区操作有利。

**缺点：**使存储器中缺乏大的空闲区，对大型文件的分区分配不利。同时也有很大的计算开销。

### 2.5 算法的测试

采用 memory.txt 测试集，我测试了这几种算法的差别,下面是各个进程开始运行时分配的资源。进程读取顺序相同，因此可通过观察offset来观察不同算法的差别。

```
 1.First_Fit
     pid arrive  prior    cpu offset Mbytes     prn    scn   modem   cd  status
 381164      0      1      1     64    192      0      0      0      0  RUNNING
 381166      0      1      3    256     32      0      0      0      0  RUNNING
 381177      0      1      1    288    416      0      0      0      0  RUNNING
 381185      0      1      3    704     32      0      0      0      0  RUNNING
 381186      0      1      1    736    160      0      0      0      0  RUNNING
 381187      0      1      3    896    128      0      0      0      0  RUNNING
 381212      7      1      2     64    108      0      0      0      0  RUNNING
 381213      7      1      2    288    256      0      0      0      0  RUNNING
 381214      7      1      2    172     80      0      0      0      0  RUNNING
 
 2.Next_Fit
     pid arrive  prior    cpu offset Mbytes     prn    scn   modem   cd  status
 381365      0      1      1     64    192      0      0      0      0  RUNNING
 381366      0      1      3    256     32      0      0      0      0  RUNNING
 381378      0      1      1    288    416      0      0      0      0  RUNNING
 381398      0      1      3    704     32      0      0      0      0  RUNNING
 381407      0      1      1    736    160      0      0      0      0  RUNNING
 381408      0      1      3    896    128      0      0      0      0  RUNNING
 381409      7      1      2     64    108      0      0      0      0  RUNNING
 381424      7      1      2    288    256      0      0      0      0  RUNNING
 381438      7      1      2    544     80      0      0      0      0  RUNNING
 
 3.Best_Fit
     pid arrive  prior    cpu offset Mbytes     prn    scn   modem   cd  status
 381503      0      1      1     64    192      0      0      0      0  RUNNING
 381505      0      1      3    256     32      0      0      0      0  RUNNING
 381516      0      1      1    288    416      0      0      0      0  RUNNING
 381524      0      1      3    704     32      0      0      0      0  RUNNING
 381537      0      1      1    736    160      0      0      0      0  RUNNING
 381553      0      1      3    896    128      0      0      0      0  RUNNING
 381557      7      1      2    736    108      0      0      0      0  RUNNING
 381559      7      1      2    288    256      0      0      0      0  RUNNING
 381575      7      1      2    544     80      0      0      0      0  RUNNING
 
 4. Worst_Fit
 pid      arrive  prior  cpu   offset Mbytes  prn    scn   modem    cd  status
 381042      0      1      1     64    192      0      0      0      0  RUNNING
 381044      0      1      3    256     32      0      0      0      0  RUNNING
 381056      0      1      1    288    416      0      0      0      0  RUNNING
 381064      0      1      3    704     32      0      0      0      0  RUNNING
 381065      0      1      1    736    160      0      0      0      0  RUNNING
 381066      0      1      3    896    128      0      0      0      0  RUNNING
 381091      7      1      2    288    108      0      0      0      0  RUNNING
 381092      7      1      2    396    256      0      0      0      0  RUNNING
 381094      7      1      2     64     80      0      0      0      0  RUNNING
```

可以看到，在调度后期，这四种算法较好的体现了内存分配上的差别。

### 2.6 算法的选择

为了进一步比较这几种算法优劣，我计算了所有进程从arrive到terminate所用时间和。不过很遗憾，在所有给的测试样例中，呈现出的结果是一样的。可能是因为样例中程序执行时间较短，内存占用时间较少，压力较轻，因而变化不大。

由于测试中，所给进程内存大小一般在0~256Mb之间，而用户内存有960Mb，故基本为中小型文件分配。因此，在本次project中，我采用Worst-Fit分配算法。



---

## 3. 调度器的结构及运行逻辑

### 3.1 调度器的结构

<img src="D:/%E5%AD%A6%E4%B9%A0/%E4%BA%BA%E5%B7%A5%E6%99%BA%E8%83%BD/hwpic/image-20220514210503023.png" alt="image-20220514210503023" style="zoom: 60%;" />



- 调度器采取**四优先级的策略**：

  1. **实时进程**（优先级为0）享有最高优先级，遵循先进先出（FCFS）的原则，且优先于其他低优先级的进程。实时进程一直运行至进程结束为止

  2. **用户进程**（优先级为1，2或3）遵循三级反馈调度器调度。基本的时间片为1秒，同时反馈调度器的时间片也是1秒。需要内存和 io 设备

- **工作调度表：**一个 txt 文件，描述了所有的进程信息

- **实时进程队列：**

  - 用于维护从工作调度表中传来的实时进程
  - 队列中的实时进程必须按照 **FCFS** 算法调度

- **用户进程队列：**

  - 用于维护从工作调度表中传来的用户进程
  - 当队列中的用户进程所需资源（内存、io ）可被满足时，才会被放入进程相对应的优先级队列中去执行

- **优先级队列：**

  - 用于执行用户进程
  - 有 3 个 优先级队列 ，对应优先级分别为 1 、 2 、 3，为**三级反馈队列**
  - 高优先级队列中的进程每执行一个时间片后，优先级降低 1 ，并进入相应优先级
    队列重新排队
  - 最低优先级队列中的进程采用 Round Robin 算法调度

### 3.2 调度器的运行逻辑

一、 程序初始化调度器输入队列（` inputqueue `）、用户进程队列(`userjobqueue`)、实时进程队列(`dispatcherqueues[0]`)、3 个优先级队列( `dispatcherqueues[1、2、3]` )。

二、初始化内存和IO资源模块

三、从工作调度表.txt中读入进程信息，放入`inputqueue`中

四、当任一队列非空或现在有进程在运行（即每经过一个时间片）：

1. 检查`inputqueue->arrivaltime`, 若进程已经到达，检查进程的合法性（**实时进程内存恒定为64Mb 且不占用IO资源；用户进程的内存和IO需求不能超额**，**且优先级合法**），若不合法则将进程丢弃。若合法，实时进程放入`dispatcherqueues[0]`,用户进程放入`userjobqueue`。
2. 将用户进程队列中**资源可满足**的进程出队并放入相应优先级队列，同时为其分配内存资源和IO资源。
3. 若当前有进程正在运行中，则：
   - 该进程的剩余 cpu 时间减少
   - 如果该进程结束，则结束该进程、释放该 PCB 空间、 释放内存和 IO 资源
   - 否则，如果当前进程为**用户进程**且工作队列中有其他进程在排队，则挂起该进程并降低优先级（优先级已经最低除外），并放入相应优先级队列。
   - **注意：若当前进程为实时进程则不打断，使其继续运行**
4. 如果当前进程为空，但有工作队列非空，则：
   - 优先级最高的队列队首进程出队(实时进程队列优先级最高)
   - 开始/继续该进程
   - 将当前进程设置为该进程
5. sleep（1），同时增加计时器
6. 返回四

五、结束



---

## 4 程序框架

本次Project使用C语言进行编程，提供的文件数量较多，先在此整理下整个代码的逻辑框架

- hostd.c 是已经给定的框架，程序的main函数也在其中，负责从进程调度表中读入进程信息。我在其中实现了四优先级队列，通过FCFS调度实时进程，并通过三级反馈队列调度用户进程。同时在进程调度的过程中进行内存管理和资源分配，使进程能正常运行。
- hostd.h 声明了所调用的库，并定义了一些常量，如QUANTUM
- pcb.c, pcb.h 为进程控制块部分，提供了控制进程及维护进程队列的API，如启动，暂停，终止，进程队列入队出队等等。
- mab.c, mab.h 为内存控制模块，实现了不同的内存控制算法（First Fit, Next Fit, Best Fit, Worst Fit）及相关辅助函数
- rsrc.c, rsrc.h 为外设资源管理模块，提供了资源的检查，分配，释放的API
- sigtrap.c 为助教评分及调试时所用的模块



---

## 5 模块描述

### 5.1 hostd.c  hostd.h 调度器模块

​		该模块为调度器模块，为程序的主体。完成声明 pcb 队列（实时进程队列、用户进程队列、优先级队列），从工作调度表中将 pcb 加入队列，控制 pcb 队列中的 pcb 的开始、挂起、结束，检测资源是否满足·······

#### 5.1.1 Important Variables

```c
 	char *inputfile = NULL; // job dispatch file
    FILE *inputliststream;
    PcbPtr inputqueue = NULL;                       // input queue buffer
    PcbPtr userjobqueue = NULL;                     // arrived processes
    PcbPtr dispatcherqueues[N_QUEUES];              // dispatcher queue array
                                             // [0] - real-time, [1]-[3] - feedback
    PcbPtr currentprocess = NULL;                   // current process
    PcbPtr process = NULL;                          // working pcb pointer
    MabPtr rtmemory = memAlloc(&memory, RT_MEMORY_SIZE); // fixed RT memory
    int timer = 0;                                  // dispatcher timer
    int quantum = QUANTUM;                          // current time-slice quantum
    int i;                                          // working index
    MabPtr usrmemory = rtmemory->next;              // usr memory
```

#### 5.1.2 main()

在3.2中已进行描述，不再赘述。

函数实现代码：

```c
int main(int argc, char *argv[])
{
    char *inputfile = NULL; // job dispatch file
    FILE *inputliststream;
    PcbPtr inputqueue = NULL;                            // input queue buffer
    PcbPtr userjobqueue = NULL;                          // arrived processes
    PcbPtr dispatcherqueues[N_QUEUES];                   // dispatcher queue array
                                                         // [0] - real-time, [1]-[3] - feedback
    PcbPtr currentprocess = NULL;                        // current process
    PcbPtr process = NULL;                               // working pcb pointer
    MabPtr rtmemory = memAlloc(&memory, RT_MEMORY_SIZE); // fixed RT memory
    int timer = 0;                                       // dispatcher timer
    int quantum = QUANTUM;                               // current time-slice quantum
    int i;                                               // working index
    MabPtr usrmemory = rtmemory->next;                   // usr memory
    int totalruntime = 0;
    //  0. Parse command line

    i = 0;
    while (++i < argc)                  //读取参数信息，确定采用的内存分配算法
    {
        if (!strcmp(argv[i], "-mf"))
        {
            MabAlgorithm = FIRST_FIT;
        }
        else if (!strcmp(argv[i], "-mn"))
        {
            MabAlgorithm = NEXT_FIT;
        }
        else if (!strcmp(argv[i], "-mb"))
        {
            MabAlgorithm = BEST_FIT;
        }
        else if (!strcmp(argv[i], "-mw"))
        {
            MabAlgorithm = WORST_FIT;
        }
        else if (!strcmp(argv[i], "-mnr"))
        {
            memFree(rtmemory); // dont preallocate RT memory
            rtmemory = NULL;
        }
        else if (!inputfile)
        {
            inputfile = argv[i];
        }
        else
        {
            PrintUsage(stdout, argv[0]);
        }
    }
    if (!inputfile)
        PrintUsage(stdout, argv[0]);
    char *ans_file = InitAnsFile(inputfile);

    //  1. Initialize dispatcher queues (all others already initialised) ;

    for (i = 0; i < N_QUEUES; dispatcherqueues[i++] = NULL)  
        ;       //N_QUEUES=4

    //  2. Initialise memory and resource allocation structures
    //     (already done)

    //  3. Fill dispatcher queue from dispatch list file;

    if (!(inputliststream = fopen(inputfile, "r")))
    { // open it
        SysErrMsg("could not open dispatch list file:", inputfile);
        exit(2);
    }

    while (!feof(inputliststream))
    { // put processes into input_queue
        process = createnullPcb();
        if (fscanf(inputliststream, "%d, %d, %d, %d, %d, %d, %d, %d",
                   &(process->arrivaltime), &(process->priority),
                   &(process->remainingcputime), &(process->mbytes),
                   &(process->req.printers), &(process->req.scanners),
                   &(process->req.modems), &(process->req.cds)) != 8)
        {
            free(process);
            continue;
        }
        process->status = PCB_INITIALIZED;
        process->ans_file = ans_file;
        inputqueue = enqPcb(inputqueue, process);
    }
    //  ==================================================================================================================
    //  NOTE: Before implement this, please make sure you have implemented the memory allocation algorithms in mab.c !!! |
    //  ==================================================================================================================

    //  4. Start dispatcher timer;
    //     (already set to zero above)

    //  5. While there's anything in any of the queues or there is a currently running process:
    while (dispatcherqueues[0] || dispatcherqueues[1] || dispatcherqueues[2] ||
           dispatcherqueues[3] || inputqueue || userjobqueue || currentprocess) //任一队列非空
    {
        //      i. Unload any pending processes from the input queue:
        //         While (head-of-input-queue.arrival-time <= dispatcher timer)
        //         dequeue process from input queue and and enqueue on either
        //           a. Real-time queue so check out parameters before enqueueing
        //           b. user job queue - check out parameters before enqueueing
        //           c. unknown priority
        while (inputqueue && inputqueue->arrivaltime <= timer)
        {
            if (inputqueue->priority == 0)
            {
                if (((abs(inputqueue->req.cds) + abs(inputqueue->req.modems) + abs(inputqueue->req.scanners) + abs(inputqueue->req.printers)) != 0) || inputqueue->mbytes > 64)
                { //检查实时进程是否合法，不合法则直接抛弃
                    deqPcb(&inputqueue);
                }
                else
                {
                    inputqueue->memoryblock = rtmemory;
                    dispatcherqueues[0] = enqPcb(dispatcherqueues[0], deqPcb(&inputqueue));     //加入实时进程队列
                }
            }
            else if (inputqueue->priority <= 3) //用户进程
            {
                if (memChkMax(inputqueue->mbytes) && rsrcChkMax(inputqueue->req))
                {
                    userjobqueue = enqPcb(userjobqueue, deqPcb(&inputqueue));       //加入用户进程队列
                }
                else
                {
                    deqPcb(&inputqueue);
                }
            }
            else //非法优先级，直接抛弃
            {
                deqPcb(&inputqueue);
            }
        }
        // TODO

        //     ii. Unload pending processes from the user job queue:
        //         While (head-of-user-job-queue.mbytes && resources can be allocated
        //           a. dequeue process from user job queue
        //           b. allocate memory to the process
        //           c. allocate i/o resources to process
        //           d. enqueue on appropriate feedback queue

        // TODO
        while (userjobqueue && (memChk(usrmemory, userjobqueue->mbytes)) && rsrcChk(&resources, userjobqueue->req))
        {   //当用户进程队列非空,分配内存和IO资源，放入优先级队列
            userjobqueue->memoryblock = memAlloc(usrmemory, userjobqueue->mbytes);
            rsrcAlloc(&resources, userjobqueue->req);
            int num = userjobqueue->priority;
            dispatcherqueues[num] = enqPcb(dispatcherqueues[num], deqPcb(&userjobqueue));
        }
        //    iii. If a process is currently running;
        //          a. Decrement process remainingcputime;
        //          b. If times up:
        //             A. Send SIGINT to the process to terminate it;
        //             B. Free memory and resources we have allocated to the process;
        //             C. Free up process structure memory
        //         c. else if a user process and other processes are waiting in feedback queues:
        //             A. Send SIGTSTP to suspend it;
        //             B. Reduce the priority of the process (if possible) and enqueue it on
        //                the appropriate feedback queue;;

        // TODO
        if (currentprocess) //如果当前有程序正在运行
        {
            currentprocess->remainingcputime -= QUANTUM;
            if (currentprocess->remainingcputime <= 0) //该程序运行结束了，终止进程并释放资源
            {
                totalruntime += (timer - currentprocess->arrivaltime);
                terminatePcb(currentprocess);

                if (currentprocess->priority > 0)
                {

                    memFree(currentprocess->memoryblock);
                    rsrcFree(&resources, currentprocess->req);
                }
                free(currentprocess);
                currentprocess = NULL;
            }
            else if (currentprocess->priority != 0 && (dispatcherqueues[1] || dispatcherqueues[2] || dispatcherqueues[3]))
            {  //如果当前程序没结束且为用户进程，且三级反馈队列非空，将其入队
                suspendPcb(currentprocess);
                if (currentprocess->priority <= 2)
                {
                    dispatcherqueues[currentprocess->priority + 1] = enqPcb(dispatcherqueues[currentprocess->priority + 1], currentprocess);
                    currentprocess->priority += 1;
                }
                else
                {
                    dispatcherqueues[currentprocess->priority] = enqPcb(dispatcherqueues[currentprocess->priority], currentprocess);
                }
                currentprocess = NULL;
            }
        }
        //     iv. If no process currently running &&  queues are not empty:
        //         a. Dequeue process from RR queue
        //         b. If already started but suspended, restart it (send SIGCONT to it)
        //              else start it (fork & exec)
        //         c. Set it as currently running process;

        // TODO
        if (!currentprocess)    //当前没有程序正在运行，则选取最高优先级的进程出队并设为运行。
        {
            if (dispatcherqueues[0])
            {
                currentprocess = deqPcb(&dispatcherqueues[0]);
            }
            else if (dispatcherqueues[1])
            {
                currentprocess = deqPcb(&dispatcherqueues[1]);
            }
            else if (dispatcherqueues[2])
            {
                currentprocess = deqPcb(&dispatcherqueues[2]);
            }
            else if (dispatcherqueues[3])
            {
                currentprocess = deqPcb(&dispatcherqueues[3]);
            }
            if (currentprocess)
                startPcb(currentprocess);
        }
        //       v. sleep for quantum;

        // TODO
        sleep(QUANTUM);
        //      vi. Increment dispatcher timer;

        // TODO
        timer += QUANTUM;
        //     vii. Go back to 5.
    }
    //    6. Exit
    printf("TotalRunTime=%d\n", totalruntime);
    exit(0);
}
```

### 5.2 pcb.h 、 pcb.c 进程控制模块

​		该模块定义了进程控制块pcb，提供了相关的API，调用 sigtrap.c 编译成的 process ，用于控制进程。

#### 5.2.1 Pcb结构体

```c
struct pcb
{
    pid_t pid;				//进程pid
    char *args[MAXARGS];	//进程参数
    char *ans_file;			
    int arrivaltime;		//到达时间
    int priority;			//优先级
    int remainingcputime;	//运行所需时间
    int mbytes;				//所需内存
    MabPtr memoryblock;		//进程内存块
    Rsrc req;				//所需资源	
    int status;				//运行状态
    struct pcb *next;		
};
typedef struct pcb Pcb;
typedef Pcb *PcbPtr;		//Pcb pointer
```

#### 5.2.2 相关API函数

```c
PcbPtr startPcb(PcbPtr);			//（恢复）/启动进程
PcbPtr suspendPcb(PcbPtr);			//暂停进程
PcbPtr terminatePcb(PcbPtr);		//终止进程
PcbPtr printPcb(PcbPtr, FILE *);	//打印进程信息
void printPcbHdr(FILE *);			
PcbPtr createnullPcb();				//进程控制块初始化
PcbPtr enqPcb(PcbPtr, PcbPtr);		//Pcb队列入队
PcbPtr deqPcb(PcbPtr *);			//Pcb队列出队
```

### 5.3 mab.c  mab.h  内存模块

#### 5.3.1 mab结构体

```c
#define MEMORY_SIZE       1024
#define RT_MEMORY_SIZE    64
#define USER_MEMORY_SIZE  (MEMORY_SIZE - RT_MEMORY_SIZE)  //960
struct mab {
    int offset;
    int size;
    int allocated;
    struct mab * next;
    struct mab * prev;
};
typedef struct mab Mab;
typedef Mab * MabPtr; 
```

HOST 系统中的内存用双向链表进行表示, 链表中的每一个结点记录了一段内存信息（ offset 、内存大小、是否被占用）, 而双向链表便于内存的分割和合并。

#### 5.3.2 相关API函数

```c
MabPtr memChk(MabPtr, int);		//找到第一个足够大的空闲内存块
int    memChkMax(int);  		//返回最大的空闲内存块
MabPtr memAlloc(MabPtr, int);	//分配内存 MabAlgorithm: 		FIRST_FIT,NEXT_FIT,BEST_FIT,WORST_FIT
MabPtr memFree(MabPtr);			//释放内存块
MabPtr memMerge(MabPtr);   		//合并 m 和 m->next
MabPtr memSplit(MabPtr, int);	//切分内存块
void   memPrint(MabPtr);		//打印所有内存信息
```

### 5.4 rsrc.c  rsrc.h  IO资源模块
#### 5.4.1 rsrc结构体

```c
#define MAX_PRINTERS 2
#define MAX_SCANNERS 1
#define MAX_MODEMS 1
#define MAX_CDS 2

struct rsrc {
    int printers;
    int scanners;
    int modems;
    int cds;
};
typedef struct rsrc Rsrc;
typedef Rsrc * RsrcPtr;
```

#### 5.4.2 相关API函数

```c
int rsrcChk(RsrcPtr, Rsrc);		//检查资源是否足够
int rsrcChkMax(Rsrc);			//检查最大空闲资源
int rsrcAlloc(RsrcPtr, Rsrc);	//资源分配
void rsrcFree(RsrcPtr,Rsrc);	//资源释放
```



---

## 6 调度策略讨论

### 6.1 采用多级调度策略原因

- 为了确保实时进程的实时交互性，我们将其设为优先级最高，且不能被其他进程打断。从而让它快速执行完毕。
- 对于用户进程，我们采用多级反馈队列，此种方式既能使高优先级的作业得到响应，减少周转时间，又能使短作业（进程）迅速完成（在时间片随优先级下降逐渐增长的情况下），也能使重型作业有较为充足的时间完成。

### 6.2 真实的操作系统方案——Linux  scheduler

#### 6.2.1 Standard UNIX scheduling algorithm

1. 抢占的、基于优先级的调度，优先级从0~139，数值越小优先级越高。
2. 实时任务：0~100 普通任务：100~139
3. 两个独立的优先级队列： a.实时进程FCFS/RR， b. 普通进程队列
4. 高优先级获得大时间片

特性：

- O(1)调度程序性能好，但交互进程的响应时间欠佳。
- 到期任务 (expired 到期，时间片运行完),不能再执行，除非其他的任务也把各自的时间片用完。—>交互性用户程序响应较差。
- 可以看到，真实的调度方案，优先级划分更加细致

#### 6.2.2  Completely Fair Scheduler (CFS)：  完全公平的调度器

在v2.6以后，Linux 标准内核实现两个调度类：采用 CFS 调度算法的默认调度类和实时调度类。		

CFS基本原理：**设定一个调度周期**（sched_latency_ns ），目标是让每个进程在这个周期内至少有机会运行一次，即每个进程等待 CPU 的时间最长不超过这个调度周期；然后**根据进程的数量，平分这个调度周期内的 CPU使用权** ，由于进程的优先级即 nice 值不同，分割调度周期的时候要加权；每个进程的累计运行时间保存在自己的 vruntime（进程的虚拟运行时间）字段里， 哪个进程vruntime 最小就获得本轮运行的权利 。

特点：**并非以优化周转时间和响应时间为目标，保证任务调度的公平性，精确性**

### 6.3 本次多级调度策略缺点

1. 饥饿（starvation）问题。如果系统有“太多”交互型工作，就会不断占用CPU，导致长工作永远无法得到CPU。即使在这种情况下，我们希望这些长工作也能有所进展。
2. 愚弄调度程序问题：恶意进程在时间片用完之前，调用一个I/O操作（比如访问一个无关的文件），从而主动释放CPU。如此便可以保持在高优先级，占用更多的CPU时间。
3. 本次实验中不同优先级的时间片长度是一样的，导致低优先级的长工作需要频繁切换，影响运行效率。

### 6.4 改进建议

1. 经过一段时间*S*，就将系统中所有工作重新加入最高优先级队列。这样可以解决饥饿问题，同时，如果一个CPU密集型工作变成了交互型，当它优先级提升时，调度程序能够正确对待它。
2. 完善CPU计时方式。记录一个进程在某一优先级队列中消耗的总时间，而不是在调度时重新计时。只要进程用完了自己的配额，就将它降到低一优先级的队列中去。这样可以防止调度器被愚弄。
3. 内存分配方案上，当内存更大，进程变得更多，可以采取页式分配，采取请求分页技术，建立页表，更好更快地利用内存，减少碎片。