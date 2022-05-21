/*
 * 作者：Wow-zheng  
 * 时间：2022年3月10日
 * 修订：2022年5月20日
 * 声明：转载请注明出处： https://github.com/Wow-zheng?tab=repositories
 */

/*
 * 程序功能：
 * 本进程作为一个总Client端， 以0.1s的时间间隔不断将数据发送至Server端，同时尽可能
 * 接受Server端返回的数据; 该进程分为两个线程，记为线程1和线程2，以下是线程的功能：
 * 1、myThread1
 * 接受来自总Server端的数据,设置recvfrom为阻塞1s，不断接受数据;一旦接受数据则打印数据
 * 2、myThread2
 * 向总Server端发送数据
 */

#include <iostream>
#include <sys/socket.h> // socker()
#include <netinet/in.h> // struct sockaddr_in ~
#include <arpa/inet.h>  // inet_addr()
#include <thread>

#define UDP_TARGET_PORT 8080           // 目标地址(本地建立服务端时使用)
#define UDP_TARGET_IP "127.0.0.1"      // 目标端口(本地建立服务端时使用)

#define SERVER_IP "127.0.0.1"          // 本机地址(和目标地址建立连接时使用)
#define SERVER_PORT 7070               // 本机端口(和目标地址建立连接时使用)
#define Received_BUFF_LEN 1024         // 接受数据数组的长度
#define Send_BUFF_LEN 1024             // 发送数据数组的长度
double Received_Data_Buf[Received_BUFF_LEN];  // 存储接受数据的数组
double Send_Data_Buf[Send_BUFF_LEN];          // 存储发送数据的数组

/*
 * [发送线程]用于持续不断发送100/1000组数据，时间间隔为100ms = 0.1s
 */
void sendThreadFunc(int& send_flag, int& recv_flag){
    std::cout << "[发送线程开始]" << std::endl;
    // 1、变量初始化
    struct sockaddr_in server_address;   // 服务端的地址信息
    double DataBuff[1024];               // 发送数据的数组
    DataBuff[0] = 1;                     // 只发送第一个数据
    int sendDataNum = 100;               // 尝试直接发送1000次数据，直接发送，不管对方是否收到，假设发送间隔为100ms = 0.1s

    // 2、建立套接字
    int client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(client_socket < 0)
    {
        printf("create socket fail!\n");
        return;
    }

    // 3、设置服务端地址信息
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(UDP_TARGET_IP);
    server_address.sin_port = htons(UDP_TARGET_PORT);

    // @ 等待接受线程准备完毕
    if (!send_flag)
        std::this_thread::sleep_for(std::chrono::seconds(3));
    // 4、尝试发送数据
    while(sendDataNum >= 0){
        sendto(client_socket, DataBuff, sizeof(DataBuff), 0, (struct sockaddr *)&server_address, sizeof(struct sockaddr_in));
        std::cout << "[Client]: 发送数据： " << DataBuff[0] << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        DataBuff[0]++;
        sendDataNum--;
    }
    recv_flag = 0;   // 关闭接受线程接受数据
    std::cout << "[发送线程结束]" << std::endl;
}

/*
 * [接受线程]用于持续不断接受数据的线程，接受无时间损耗
 */
void recvThreadFunc(int& send_flag, int& recv_flag){
    std::cout << "[接受线程开始]" << std::endl;
    // 1、变量初始化
    struct sockaddr_in server_addr;    // 本机的地址信息
    struct sockaddr_in client_address; // 用于存储服务端的地址信息
    int recvDataNum = 1000;            // 尝试接受1000次数据，因为recvfrom是阻塞的，因此一定能够接受完1000个数据
    int recvNum = 0;                   // 用于计算总共接收到了多少数据：假设数据处理需要500ms = 0.5s

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
    while(recvDataNum > 0 && recv_flag)
    {
        // 5、接受数据，先清空用于存储接受数据的数组
        memset(Received_Data_Buf, 0, Received_BUFF_LEN);
        socklen_t clent_address_len = sizeof(client_address);
        send_flag = 1;        // 启动发送线程发送数据
        int Received_Flag = recvfrom(server_socket, Received_Data_Buf, Received_BUFF_LEN, 0, (struct sockaddr*)&client_address, &clent_address_len);
        if(Received_Flag == -1)
        {
//            printf("recieve data fail!\n");
            continue;
        }
        else if(Received_Flag > 0){
            // 6、计数，即接收到返回的数据次数
            std::cout << "[Client]:------------[接受数据]： " << Received_Data_Buf[0] << std::endl;
            recvNum++;
        }
        else{
            std::cout << "************" << Received_Flag << std::endl;
        }

    }
    std::cout << "[接受线程结束]: 接受数据次数： " << recvNum << std::endl;
}

/*
 * 主程序
 */
int main() {
    int send_flag = 0;
    int recv_flag = 1;
    std::thread myThread1(recvThreadFunc, std::ref(send_flag), std::ref(recv_flag));
    std::thread myThread2(sendThreadFunc, std::ref(send_flag), std::ref(recv_flag));
    myThread1.join();
    myThread2.join();
    std::cout << "[进程结束]" << std::endl;
    return 0;
}
