// server.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include "modsocket.h"
using namespace std;
//定义服务端
modsocket_server mysocket;

void error(const char* err){
	cout << "error:" << err << endl;
}

//用户进入
void into(SOCKET client, sockaddr_in info) {

	char* text = nullptr;
	text=strcat(inet_ntoa(info.sin_addr),"进入了聊天室>>");

	mysocket.Collective_Notice(text, client);
	cout << "into" << "IP:" << inet_ntoa(info.sin_addr)<< "PORT:" << ntohs(info.sin_port) << endl;
	//发送数据
	send(client, "来了老弟?", sizeof("来了老弟?"), 0);
}


//数据到达
void Received(SOCKET client, sockaddr_in info, char* data) {
	
	cout << "recv:"   << "IP:" << inet_ntoa(info.sin_addr) <<  "PORT:" << ntohs(info.sin_port) << endl << data<<endl;
	//send(client, data, sizeof(data)+1, 0);
	char* text = nullptr;
	text = strcat(inet_ntoa(info.sin_addr), ":");
	text = strcat(text, data);

	mysocket.Collective_Notice(text, client);
}


//用户离开
void leave(SOCKET client, sockaddr_in info) {
	char* text = nullptr;
	text = strcat(inet_ntoa(info.sin_addr), "离开了聊天室>>");

	mysocket.Collective_Notice(text, client);
	cout << "exit"  << "IP:" << inet_ntoa(info.sin_addr) <<  "PORT:" << ntohs(info.sin_port) << endl;
}

int main()
{


	mysocket.init(&into,&Received,&leave,&error,19449,true);
	//停止服务
	mysocket.exit();

}


