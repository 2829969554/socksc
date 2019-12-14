
#include <iostream>
#include "modsocket.h"
using namespace std;
modble_client client;
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