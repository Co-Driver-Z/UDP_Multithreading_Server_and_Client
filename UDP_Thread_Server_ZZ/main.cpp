/*
 * 作者：Wow-zheng
 * 时间：2022年3月10日
 * 修订：2022年5月20日
 * 声明：转载请注明出处： https://github.com/Wow-zheng?tab=repositories
 */

/*
 * 程序功能：
 * 本进程作为一个总Server端，尽可能快地接受总Client端的数据并存储在容器中，同时处理容器中的数据(0.5s)发送至
 * 总Client端。该进程分为2个线程，记为线程1和线程2，以下是线程的功能：
 * 1、myThread1
 * 接受来自总Client端的数据, 设置recvfrom为阻塞1s，不断接受数据;一旦接收到数据则存储在Received_Data_Buf中；
 * 2、myThread2
 * 发送处理后的数据至总Client端，查看Received_Data_Buf是否有数据，有数据则处理后发送；否则不发送数据；
 *
 */

#include <iostream>
#include <sys/socket.h>   // socket()
#include <netinet/in.h>   // struct sockaddr_in ~
#include <arpa/inet.h>    // inet_addr()
#include <thread>
#include <mutex>
#include <list>

#define SERVER_IP "127.0.0.1"          // 本机地址
#define SERVER_PORT 8080               // 本机端口
#define UDP_TARGET_IP "127.0.0.1"      // 目标地址
#define UDP_TARGET_PORT 7070           // 目标端口
#define Received_BUFF_LEN 1024         // 接受数据数组的长度
#define Send_BUFF_LEN 1024             // 发送数据数组的长度
double Received_Data_Buf[Received_BUFF_LEN];  // 存储接受数据的数组
double Send_Data_Buf[Send_BUFF_LEN];          // 存储发送数据的数组

/*
 * [线程1]读取数据
 */
void recvThreadFunc(std::list<double>& recvBuff, std::mutex& myMutex1){
    // 1、变量初始化
    std::cout << "[接受线程开始]" << std::endl;
    struct sockaddr_in server_addr;             // 本机的地址信息
    struct sockaddr_in client_address;          // 用于存储服务端的地址信息
    int recvDataNum = 1000;                     // 尝试接受1000次数据，因为recvfrom是阻塞的，因此一定能够接受完1000个数据
    int recvNum = 0;                            // 用于计算总共接收到了多少数据：假设数据处理需要500ms = 0.5s
    // 2、建立套接字
    int server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(server_socket < 0)
    {
        printf("create socket fail!\n");
        return;
    }
    // 3、设置本地地址信息
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(SERVER_PORT);
    socklen_t clent_address_len = sizeof(client_address);
    // 4、将套接字绑定本地地址上
    int server_bind = bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(server_bind < 0)
    {
        printf("socket bind fail!\n");
        return;
    }
    // @ 设置recvfrom为阻塞和超时
    struct timeval timeout;
    timeout.tv_sec = 1;    // 秒
    timeout.tv_usec = 0;   // 微秒
    setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    while(recvDataNum > 0)
    {
        // 5、接受数据，先清空用于存储接受数据的数组
        memset(Received_Data_Buf, 0, Received_BUFF_LEN);
        int Received_Flag = recvfrom(server_socket, Received_Data_Buf, Received_BUFF_LEN, 0, (struct sockaddr*)&client_address, &clent_address_len);
        if(Received_Flag == -1)
        {
            continue;
        }
        // 5、接收到数据，尝试上锁
        std::unique_lock<std::mutex> myGuard(myMutex1, std::defer_lock);
        if (myGuard.try_lock()){
            // 6、打印数据
            recvBuff.push_back(Received_Data_Buf[0]);
            std::cout << "[Server]:接收到数据： " << Received_Data_Buf[0] << std::endl;
            myGuard.unlock();
            recvDataNum--;
        }
    }
}
/*
 * [线程2]处理完直接发送
 */
void ProcessSendData(std::mutex& myMutex1, std::mutex& myMutex2, std::list<double>& recvBuff, std::list<double>& sendBuff){
    std::cout << "[发送线程开始]" << std::endl;
    // 1、变量初始化
    struct sockaddr_in client_address_server;   // 用于存储地址信息
    int sendNumMax = 101;                       // 用于测试最多发送的次数
    // 2、建立套接字
    int client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(client_socket < 0)
    {
        printf("create socket fail!\n");
        return;
    }
    // 3、设置目标地址
    memset(&client_address_server, 0, sizeof(client_address_server));
    client_address_server.sin_family = AF_INET;
    client_address_server.sin_addr.s_addr = inet_addr(UDP_TARGET_IP);
    client_address_server.sin_port = htons(UDP_TARGET_PORT);
    while (sendNumMax >= 0){
        double temp_value = 0;
        std::unique_lock<std::mutex> myGuard1(myMutex1, std::defer_lock);
        if (!recvBuff.empty() && myGuard1.try_lock()){
            temp_value = recvBuff.back();
            recvBuff.clear();
            myGuard1.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 模拟处理数据0.5s
            Send_Data_Buf[0] = temp_value;
            sendto(client_socket, Send_Data_Buf, sizeof(Send_Data_Buf), 0, (struct sockaddr *)&client_address_server, sizeof(struct sockaddr_in));
            std::cout << "[Server]:----------发送数据: " << temp_value << std::endl;
            sendNumMax--;
        }
        else
            continue;
    }
}

/*
 * 主程序
 */
int main() {
    std::list<double> recvBuff;
    std::list<double> sendBuff;
    std::mutex myMutex1;          // 互斥锁1 用于保护数据 recvBuff
    std::mutex myMutex2;          // 互斥锁2 用于保护数据 sendBuff
    std::thread myThread1(recvThreadFunc, std::ref(recvBuff), std::ref(myMutex1));
    std::this_thread::sleep_for(std::chrono::seconds(2));     // 等待2s，使得myThread1确保启动
    std::thread myThread2(ProcessSendData, std::ref(myMutex1), std::ref(myMutex2), std::ref(recvBuff), std::ref(sendBuff));
    myThread1.join();
    myThread2.join();
    return 0;
}


