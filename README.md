# UDP_Multithreading_Server_and_Client
<---------------------------------UDP Multithreading Server------------------------>  
Author: Wow-zheng    
Illustrate：  
                                     [Chinese]
    本进程作为一个总Server端，尽可能快地接受总Client端的数据并存储在容器中，同时处理容器  
    中的数据(0.5s)发送至总Client端。该进程分为2个线程，记为线程1和线程2，以下是线程的功能：  
    1、myThread1  
    接受来自总Client端的数据, 设置recvfrom为阻塞1s，不断接受数据;一旦接收到数据则存储在  
    Received_Data_Buf中  
    2、myThread2  
    发送处理后的数据至总Client端,查看Received_Data_Buf是否有数据，有数据则处理后发送;  
    否则不发送数据；(假设处理时间为0.5s)  
                                     [English]  
    As a total server side, this process accepts the data of the total client side  
    as soon as possible and stores it in the container, and at the same time  
    processes the data in the container (0.5s) and sends result to the total client side.  
    The process is divided into 2 threads, denoted as myThread1 and myThread2, the  
    following is the function of the thread:  
    1、myThread1  
    Accept data from the total client, set recvfrom to block for 1s, and continuously  
    accept data; once the data is received, it is stored in Received_Data_Buf;  
    2、myThread2  
    Send the processed data to the total client, check whether there is data in  
    Received_Data_Buf, if there is data, send it after processing; otherwise, no  
    data will be sent; (assuming the processing time is 0.5s)  
<---------------------------------------------------------------------------------->  
  
<---------------------------------UDP Multithreading Client------------------------>  
Author: Wow-zheng  
Illustrate:  
                                     [Chinese]  
    本进程作为一个总Client端， 以0.1s的时间间隔不断将数据发送至Server端，同时尽可能  
    接受Server端返回的数据; 该进程分为两个线程，记为线程1和线程2，以下是线程的功能：  
    1、myThread1  
    接受来自总Server端的数据,设置recvfrom为阻塞1s，不断接受数据;一旦接受数据则打印数据  
    2、myThread2  
    向总Server端发送数据  
                                     [English]  
    As a total client side, this process continuously sends data to the server  
    side at 0.1s intervals,and at the same time accepts the data returned by the  
    server side as much as possible; the process is divided into two threads,  
    denoted as myThread1 and myThread2, the following are The function of the thread:  
    1、myThread1  
    Accept data from the total server side, set recvfrom to block for 1s, and  
    continuously accept data; once the data is accepted, print the data  
    2、myThread2  
    Send data to the total server  
<---------------------------------------------------------------------------------->  
    Please feel free to contact me if you have any questions or find bugs, it is greatly appreciated  
<---------------------------------------------------------------------------------->  