#pragma comment(lib, "ws2_32")
#include <iostream>
#include <stdio.h>
#include <conio.h>
#include <string>
#include <vector>
#include <windows.h>	
#include <process.h> 

using namespace std;
#define MAXCOL 50
#define MAXROW 30
#define NAMEFLAG 0
#define CHATFLAG 1
#define MAX_NAME_LEN 30
#define MAX_LINE_LEN 30

unsigned WINAPI SendMsg(void* arg);
unsigned WINAPI RecvMsg(void* arg);
void ErrorHandling(const char* msg);
void initConsole();
void gotoxy(int, int);

string name;
string msg;
int flag = NAMEFLAG;
int line = 0;
vector<string> screen;


int main(int argc, char* argv[])
{
	WSADATA wsaData;
	SOCKET hSock;
	SOCKADDR_IN servAdr;
	HANDLE hSndThread, hRcvThread;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)//윈속 초기화
		ErrorHandling("WSAStartup() error!");

	gotoxy(MAXCOL / 2 - 10, MAXROW / 2);
	cout << "이름을 정해주십시오";
	char k;

	while ((k = _getch()) != 13) {
		if (k == 8 && name.size() != 0) {
			name.pop_back();
			gotoxy(MAXCOL / 2 - MAX_NAME_LEN / 2 + name.size(), MAXROW / 2 + 1);
			cout << " ";
			gotoxy(MAXCOL / 2 - MAX_NAME_LEN / 2 + name.size(), MAXROW / 2 + 1);
		}
		else if (k != 8 && name.size() == MAX_NAME_LEN) {
			gotoxy(MAXCOL / 2 - MAX_NAME_LEN / 2 + name.size()-1, MAXROW / 2 + 1);
			name.pop_back();
			name.push_back(k);
			cout << k;
		}
		else {
			gotoxy(MAXCOL / 2 - MAX_NAME_LEN / 2 + name.size(), MAXROW / 2 + 1);
			name.push_back(k);
			cout << k;
		}
	}
	system("cls");

	hSock = socket(PF_INET, SOCK_STREAM, 0);//소켓 연결

	memset(&servAdr, 0, sizeof(servAdr));//구조체 초기화
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servAdr.sin_port = htons(atoi("8080"));

	if (connect(hSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR)//연결
		ErrorHandling("connect() error");

	hSndThread =
		(HANDLE)_beginthreadex(NULL, 0, SendMsg, (void*)&hSock, 0, NULL);//보내기 메세지 스레드
	hRcvThread =
		(HANDLE)_beginthreadex(NULL, 0, RecvMsg, (void*)&hSock, 0, NULL);//받기 메세지 스레드

	WaitForSingleObject(hSndThread, INFINITE);
	WaitForSingleObject(hRcvThread, INFINITE);//스레드 종료(시그널 상태)까지 '무한정' 대기
	closesocket(hSock);
	WSACleanup();
	return 0;
}

unsigned WINAPI SendMsg(void* arg)   // send thread main
{
	SOCKET hSock = *((SOCKET*)arg);
	string nameMsg;
	while (1)
	{
		char k;
		while ((k = _getch()) != 13) {
			if (k == 8 && msg.size() != 0) {
				msg.pop_back();
				gotoxy(msg.size(), MAXROW - 1);
				cout << " ";
			}
			else if (k != 8 && msg.size() == MAX_LINE_LEN) {
				gotoxy(msg.size() - 1, MAXROW - 1);
				msg.pop_back();
				msg.push_back(k);
				cout << k;
			}
			else {
				gotoxy(msg.size(), MAXROW - 1);
				msg.push_back(k);
				cout << k;
			}
		}
		
		if (msg.compare("q") == 0 || msg.compare("Q") == 0) //qQ 입력시 종료
		{
			closesocket(hSock);
			exit(0);
		}
		nameMsg = "[" + name + "] : " + msg;
		gotoxy(0, MAXROW - 1);
		for (int i = 0; i < msg.size(); i++)
			cout << " ";
		msg.clear();
		send(hSock, nameMsg.c_str(), nameMsg.length(), 0);//서버로 보냄
	}
	return 0;
}

unsigned WINAPI RecvMsg(void* arg)   // read thread main
{
	int hSock = *((SOCKET*)arg);
	char buf[MAX_LINE_LEN + 1] = "";
	string nameMsg;
	while (recv(hSock, buf, sizeof(buf), 0) > 0) {
		gotoxy(0, line);
		nameMsg.append(buf,sizeof(buf));
		memset(buf, ' ', sizeof(buf));
		buf[MAX_LINE_LEN] = '\0';
		if (line < MAXROW - 1) {
			line++;
			cout << nameMsg << endl;
			screen.push_back(nameMsg);
		}
		else {
			screen.erase(screen.begin());
			screen.push_back(nameMsg);
			for (int i = 0; i < MAXROW - 1; i++)
			{
				gotoxy(0, i);
				cout << buf;
				gotoxy(0, i);
				cout << screen[i] << endl;
			}
		}
		nameMsg.clear();
	}

	return 0;
}

void ErrorHandling(const char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

void gotoxy(int x, int y) {
	COORD pos = { x,y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void initConsole() {
	system("Simple Katalk");
	system("mode con:cols=50 lines=30");

	CONSOLE_CURSOR_INFO c;
	HANDLE h;
	h = GetStdHandle(STD_OUTPUT_HANDLE);

	c.bVisible = 0;
	c.dwSize = 1;

	SetConsoleCursorInfo(h, &c);
	SetConsoleCursorPosition(h, { 0, 0 });
}