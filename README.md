# ModSocket


ModSocket是一套通用的轻量级TCP 通信框架，包含服务端组件、客户端组件，适用于各种不同应用场景的TCP通信系统，提供C/C++、E（易语言）、Python等编程语言接口。ModSocket对通信层完全封装；ModSocket提供基于事件通知模型的API接口，能非常简单的整合到新旧应用程序中。



ModSocket适用于windows系统



ModSocket.h


#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS 0


#include<WinSock2.h>


#include <thread>
#include <iostream>
using namespace std;

#pragma comment (lib,"ws2_32")
#pragma comment (lib,"winmm")

#pragma comment(lib, "Ws2_32.lib")
//客户端
class modble_client {
private:
	WSADATA wsaData;
	SOCKET sHost;
	SOCKADDR_IN servAddr;

	//内部消息处理
	void cl(void Received(char* data), void discontinue(const char* err)) {
		char recv_buf[MAXBYTE] = { 0 };
		int recv_len;
		while (1) {
			recv_len = recv(sHost, recv_buf, MAXBYTE, 0);
			if (recv_len < 0) {
				discontinue("服务器主动断开连接");
				break;
			}
			else
			{
				Received(recv_buf);
			}

		}
	}
public:
	//服务端IP,服务端port,是否阻塞,数据到达事件,断开连接事件
	void init(const char* ip, int port, bool isok, void Received(char* data), void discontinue(const char* err)) {
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
			discontinue("Winsock load faild!");
			return;
		}

		//  服务器套接字
		sHost = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sHost == INVALID_SOCKET) {
			discontinue("socket faild!");
			return;
		}

		servAddr.sin_family = AF_INET;
		//  注意   当把客户端程序发到别人的电脑时 此处IP需改为服务器所在电脑的IP
		servAddr.sin_addr.S_un.S_addr = inet_addr(ip);
		servAddr.sin_port = htons(port);
		//  连接服务器
		if (connect(sHost, (LPSOCKADDR)& servAddr, sizeof(servAddr)) == SOCKET_ERROR) {
			discontinue("connect faild!");
			return;
		}
		if (isok) {
			cl(Received, discontinue);
		}
		else
		{
			thread t123(&modble_client::cl, this, Received, discontinue);
			t123.detach();
		}

		return;

	}
	void ssend(const char* data) {
		send(sHost, data, MAXBYTE, 0);
	}
	void exit() {
		closesocket(sHost);
		WSACleanup();
	}
};

/*******************************************************************************************/

//*****************************以下为modsocket_server*******************************************
//服务端
class modsocket_server {

private:
	WSADATA wsaData;
	sockaddr_in addr;
	SOCKET s;
	//理论上可以同时在线5000人
	SOCKET clientSock[5000];
	SOCKADDR clientAddr[5000];
	sockaddr_in clientsa[5000];
	int uid = 0;

	/*
	内部消息处理
	参数一:数据到达事件
	参数二:用户断开事件
	参数三:用户socket结构体
	参数四:用户info结构体
	*/
	void cl(void Received(SOCKET, sockaddr_in, char*), void leave(SOCKET, sockaddr_in), SOCKET xxyh, sockaddr_in yhinfo) {
		//不断的接收客户端发送过来的信息
		while (TRUE)
		{
			char buff[MAXBYTE] = { 0 };
			//接收在线客户端发来的数据
			int i ;
			i = recv(xxyh, buff, MAXBYTE, 0);
			if (i >= 0 ) {
				//事件通知::接收数据
				Received(xxyh, yhinfo, buff);
			}
			else
			{

				//事件通知::客户断开
				closesocket(xxyh);
				leave(xxyh, yhinfo);
				//uid--;
				return;
			}
		}
	}

	/*
	内部事件处理
	参数一:用户进入事件
	参数二:数据到达事件
	参数三:用户断开事件
	参数四:端口
	*/
	void forecho(void  into(SOCKET, sockaddr_in), void Received(SOCKET, sockaddr_in, char*), void leave(SOCKET, sockaddr_in),void error(const char*), int port) {

		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {

			error("Winsock load faild!");
			return;
		}
		s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (s == INVALID_SOCKET) {
			error("socket faild!");
			return;
		}

		addr.sin_addr.S_un.S_addr = inet_addr("0.0.0.0");
		addr.sin_port = htons(port);
		addr.sin_family = PF_INET;

		bind(s, (SOCKADDR*)& addr, sizeof(SOCKADDR));

		if (listen(s, 1)) {
			error("监听失败,可能端口已经存在,请尝试更换...");
			return;
		}

		int nSize = sizeof(SOCKADDR);


		while (TRUE) {
			clientSock[uid] = accept(s, (SOCKADDR*)& clientAddr, &nSize);

			//事件通知::用户进入
			int len = sizeof(clientsa[uid]);
			getpeername(clientSock[uid], (struct sockaddr*) & clientsa[uid], &len);
			into(clientSock[uid], clientsa[uid]);
			//消息处理

			//cl( Received, leave);
			thread t123(&modsocket_server::cl, this, Received, leave, clientSock[uid], clientsa[uid]);
			t123.detach();
			uid++;
		}

	}
public:

	/*
	初始化事件
	参数一:用户进入事件
	参数二:数据到达事件
	参数三:用户断开事件
	参数四:错误处理
	参数无:端口
	参数六:是否阻塞
	*/
	void init(void  into(SOCKET, sockaddr_in), void Received(SOCKET, sockaddr_in, char*), void leave(SOCKET, sockaddr_in),void error(const char*), int port, bool isok) {
		if (isok) {
			//阻塞模式
			forecho(into, Received, leave,error, port);
		}
		else
		{
			//非阻塞模式
			thread t123a(&modsocket_server::forecho, this, into, Received, leave, error, port);
			t123a.detach();
		}


	}
	//获取用户远程IP
	char* get_user_ip(SOCKET client) {
		sockaddr_in sa;
		int len = sizeof(sa);
		getpeername(client, (struct sockaddr*) & sa, &len);
		return inet_ntoa(sa.sin_addr);
	}

	//获取用户远程端口
	int get_user_port(SOCKET client) {
		sockaddr_in sa;
		int len = sizeof(sa);
		getpeername(client, (struct sockaddr*) & sa, &len);
		return ntohs(sa.sin_port);
	}
	void Collective_Notice(const char* text,SOCKET nosend) {
		for (int i = 0; i < sizeof(clientSock); i++ ) {
			if (clientSock[i] != nosend) {
				send(clientSock[i], text, MAXBYTE, 0);
			}
			
		}
	}
	void Collective_Notice(const char* text) {
		for (int i = 0; i < sizeof(clientSock); i++) {
			send(clientSock[i], text, MAXBYTE, 0);
		}
	}
	void ssend(SOCKET client, const char* text) {
		send(client, text, MAXBYTE, 0);
	}
	void exit() {
		//关闭服务
		closesocket(s);
		WSACleanup();
	}
};

//***************************************以上为modsocket_server******************************************** 



C++实例
//Server代码
// server.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#define CRTSECURENOWARNINGS
#include <iostream>
#include "modsocket.h"
using namespace std;
//定义服务端组件
modsocketserver mysocket;
void error(const char* err){
    cout << "error:" << err << endl;
}
//用户进入
void into(SOCKET client, sockaddrin info) {
   char* text = nullptr;
   text=strcat(inetntoa(info.sinaddr),"进入了聊天室>>");
 mysocket.CollectiveNotice(text, client);
   cout << "into" << "IP:" << inetntoa(info.sinaddr)<< "PORT:" << ntohs(info.sinport) << endl;
  //发送数据
  send(client, "来了老弟?", sizeof("来了老弟?"), 0);
}

//数据到达
void Received(SOCKET client, sockaddrin info, char* data) {
   
    cout << "recv:"   << "IP:" << inetntoa(info.sinaddr) <<  "PORT:" << ntohs(info.sinport) << endl << data<<endl;
   //send(client, data, sizeof(data)+1, 0);
    char* text = nullptr;
   text = strcat(inetntoa(info.sinaddr), ":");
   text = strcat(text, data);
  mysocket.CollectiveNotice(text, client);
}

//用户离开
void leave(SOCKET client, sockaddrin info) {
   char* text = nullptr;
   text = strcat(inetntoa(info.sinaddr), "离开了聊天室>>");
  mysocket.CollectiveNotice(text, client);
   cout << "exit"  << "IP:" << inetntoa(info.sinaddr) <<  "PORT:" << ntohs(info.sinport) << endl;
}
int main()
{

   mysocket.init(&into,&Received,&leave,&error,19449,true);
    //停止服务
  mysocket.exit();
}

//Client代码
#include <iostream>
#include "modsocket.h"
using namespace std;
modbleclient client;
//数据到达
void Received(char* data) {
    cout << "recv:" << data << endl;
  
    cout << "请输入:";
   char text[MAXBYTE] = { 0 };
 cin >> text;
  client.ssend(text);
}
//断开连接
void discontinue(const char* err) {
   cout << "exit:"<< err << endl;
}
int main()
{
    client.init("127.0.0.1", 19449,true,&Received,&discontinue);
    client.exit();
  return 0;
}
