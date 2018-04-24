// server_tcp.cpp : 定义控制台应用程序的入口点。
//
#define WIN32_LEAN_AND_MEAN
#include <Ws2tcpip.h>
#include <windows.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <string.h>
using namespace std;
#pragma comment(lib,"WS2_32.lib")
#define BUF_SIZE 1024
#define SIZE 1024
char s[SIZE];

int retVal;
char buf[BUF_SIZE];
SOCKET sHost;
SOCKADDR_IN servAddr;

int __cdecl main(int argc, char **argv)
{
	//指向包含关于主机的响应信息的一个或多个addrinfo结构的链接列表的指针 。
	struct addrinfo *result = NULL;
	//初始化
	
	//指向addrinfo结构的指针， 它提供有关调用方支持的套接字类型的提示。
	struct addrinfo hints;


	//pHints参数指向的addrinfo结构的ai_addrlen，ai_canonname，ai_addr和ai_next成员必须为零或NULL。
	ZeroMemory(&hints, sizeof(hints));//初始化
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;//tcp套接字
	hints.ai_protocol = IPPROTO_TCP;

	SOCKET ConnectSocket = INVALID_SOCKET;//创建连接套接字，并初始化

	//检查控制台参数
	if (argc != 3) {
		printf("usage: %s <IPv4 address> <port>\n", argv[0]);
		printf("      %s 192.168.16.34 52000\n", argv[0]);
		return 1;
	}
	
	//初始化wsatata
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("WSAStartup error!\n");
		return 0;
	}

	
	//getaddrinfo 处理要连接的远程地址和端口，原型如下：
	/*int WSAAPI getaddrinfo（
		_In_opt_ PCSTR pNodeName,	//IP地址
		_In_opt_ PCSTR pServiceName，//端口
		_In_opt_  const ADDRINFOA * pHints，//套接字类型
		_Out_ PADDRINFOA * ppResult		//主机的响应信息
		）;*/
	
	//通过getaddrinfo解决远程地址和端口
	int dwRetval = getaddrinfo(argv[1], argv[2], &hints, &result);
	if (dwRetval != 0) 
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		printf("getaddrinfo failed with error: %d\n", dwRetval);
		//system("pause");
		WSACleanup();
		return 1;
	}
	ConnectSocket = socket(result->ai_family, result->ai_socktype,
		result->ai_protocol);

	//连接远程地址
	retVal = connect(ConnectSocket, result->ai_addr, (int)result->ai_addrlen);
	if (SOCKET_ERROR == retVal)
	{
		wprintf(L"connect function failed with error: %ld\n", WSAGetLastError());
		closesocket(sHost);
		WSACleanup();
		return -2;
	}

	while (true)
	{
		printf("Input:");
		gets_s(s);
		ZeroMemory(buf, BUF_SIZE);//初始化
		strcpy(buf, s);//获取到输入的内容，到buf中

		retVal = send(ConnectSocket, buf, strlen(buf), 0);//发送

		if (SOCKET_ERROR == retVal)
		{
			printf_s("send error!\n");
			closesocket(sHost);
			WSACleanup();
			return -3;
		}

		retVal = recv(sHost, buf, sizeof(buf) + 1, 0);
		printf_s("Recv from server:%s\n", buf);
		if (strcmp(buf, "quit") == 0)
		{
			printf_s("quit!\n");
			break;
		}


	}
	closesocket(sHost);
	WSACleanup();
	return 0;
}

