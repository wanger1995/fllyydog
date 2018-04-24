// serverTest.cpp : 定义控制台应用程序的入口点。
//
#include "stdafx.h"
#include <winsock2.h>
#include <Windows.h>  
#include <stdlib.h>

#include <iostream>

using namespace std;

#pragma comment(lib,"WS2_32.lib")
#define BUF_SIZE 64
int retVal;
char buf[BUF_SIZE];
SOCKET clientSocket;
sockaddr_in addrClient;

DWORD WINAPI AnswerThread(LPVOID lparam)
{
	clientSocket = (SOCKET)(LPVOID)lparam;
	
	while (true)
	{
		ZeroMemory(buf, BUF_SIZE);//清空缓冲区
		retVal = recv(clientSocket, buf, BUFSIZ, 0);//接受数据
		if (SOCKET_ERROR == retVal)
		{
			int err = WSAGetLastError();
			if (err = WSAEWOULDBLOCK)
			{
				Sleep(100);
				continue;
			}
			else if (err = WSAETIMEDOUT || err == WSAENETDOWN)
			{
				printf("listen error!\n");
				closesocket(clientSocket);
				WSACleanup();
				return -1;

			}
			
		}

		printf("From client[%s:%d]:%s\n", inet_ntoa(addrClient.sin_addr), addrClient.sin_port, buf);
		if (strcmp(buf, "quit") == 0)
		{
			retVal = send(clientSocket, "quit", strlen("quit"), 0);
			break;
		}
		else
		{
			char msg[BUF_SIZE] = { NULL };
			sprintf_s(msg, "Message received - %s\n", buf);
			retVal = send(clientSocket, msg, strlen(msg), 0);
			if (SOCKET_ERROR == retVal)
			{
				int err = WSAGetLastError();
				if (err = WSAEWOULDBLOCK)
				{
					Sleep(500);
					continue;
				}
				else
				{
					printf("listen error!\n");
					closesocket(clientSocket);
					WSACleanup();
					return -1;
				}
				
			}
		}
	}
	
}

int main()
{
	
	
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup 无法初始化！");
		return 0;
	}

	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == INVALID_SOCKET)
	{
		printf("socket error!");
		WSACleanup();
		return -1;
	}
	//设置服务器socker地址
	SOCKADDR_IN addrServer;
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons(9900);//监听端口9990；
	addrServer.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//任意本地地址

														//绑定server socket到本地地址
	retVal = bind(serverSocket, (sockaddr*)&addrServer, sizeof(SOCKADDR_IN));
	if (SOCKET_ERROR == retVal)
	{
		printf("bind error!\n");
		closesocket(serverSocket);
		WSACleanup();
		return -1;
	}
	int iMode = 1;
	retVal = ioctlsocket(serverSocket, FIONBIO, (u_long FAR*) &iMode);
	//监听
	retVal = listen(serverSocket, 3);
	if (SOCKET_ERROR == retVal)
	{
		printf("listen error!\n");
		closesocket(serverSocket);
		WSACleanup();
		return -1;
	}

	//处理请求
	printf("Server start...\n");
	
	int addrClientlen = sizeof(addrClient);

	//循环等待
	while (true)
	{
		clientSocket = accept(serverSocket, (sockaddr FAR*)&addrClient, &addrClientlen);
		if (INVALID_SOCKET == clientSocket)
		{
			int err = WSAGetLastError();
			if (err = WSAEWOULDBLOCK)
			{
				Sleep(100);
				continue;
			}

		}
		DWORD dwThreadTd;
		CreateThread(NULL, NULL, AnswerThread, (LPVOID)clientSocket, 0, &dwThreadTd);
	}

}


