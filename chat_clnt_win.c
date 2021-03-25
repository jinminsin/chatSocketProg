#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <process.h> 
	
#define BUF_SIZE 100
#define NAME_SIZE 20
	
unsigned WINAPI SendMsg(void * arg);
unsigned WINAPI RecvMsg(void * arg);
void ErrorHandling(char * msg);
	
char name[NAME_SIZE]="[DEFAULT]";
char msg[BUF_SIZE];
	
int main(int argc, char *argv[])
{
	WSADATA wsaData;
	SOCKET hSock;
	SOCKADDR_IN servAdr;
	HANDLE hSndThread, hRcvThread;

	if(argc!=4) {
		printf("Usage : %s <IP> <port> <name>\n", argv[0]);
		exit(1);
	}//명령어줄 확인

	if(WSAStartup(MAKEWORD(2, 2), &wsaData)!=0)//윈속 초기화
		ErrorHandling("WSAStartup() error!"); 

	sprintf(name, "[%s]", argv[3]);
	hSock=socket(PF_INET, SOCK_STREAM, 0);//소켓 연결
	
	memset(&servAdr, 0, sizeof(servAdr));//구조체 초기화
	servAdr.sin_family=AF_INET;
	servAdr.sin_addr.s_addr=inet_addr(argv[1]);
	servAdr.sin_port=htons(atoi(argv[2]));
	  
	if(connect(hSock, (SOCKADDR*)&servAdr, sizeof(servAdr))==SOCKET_ERROR)//연결
		ErrorHandling("connect() error");
	
	hSndThread=
		(HANDLE)_beginthreadex(NULL, 0, SendMsg, (void*)&hSock, 0, NULL);//보내기 메세지 스레드
	hRcvThread=
		(HANDLE)_beginthreadex(NULL, 0, RecvMsg, (void*)&hSock, 0, NULL);//받기 메세지 스레드

	WaitForSingleObject(hSndThread, INFINITE);
	WaitForSingleObject(hRcvThread, INFINITE);//스레드 종료(시그널 상태)까지 '무한정' 대기
	closesocket(hSock);
	WSACleanup();
	return 0;
}
	
unsigned WINAPI SendMsg(void * arg)   // send thread main
{
	SOCKET hSock=*((SOCKET*)arg);
	char nameMsg[NAME_SIZE+BUF_SIZE];
	while(1) 
	{
		fgets(msg, BUF_SIZE, stdin);//입력
		if(!strcmp(msg,"q\n")||!strcmp(msg,"Q\n")) //qQ 입력시 종료
		{
			closesocket(hSock);
			exit(0);
		}
		sprintf(nameMsg,"%s %s", name, msg); // 메세지
		send(hSock, nameMsg, strlen(nameMsg), 0);//서버로 보냄
	}
	return 0;
}
	
unsigned WINAPI RecvMsg(void * arg)   // read thread main
{
	int hSock=*((SOCKET*)arg);
	char nameMsg[NAME_SIZE+BUF_SIZE];
	int strLen;
	while(1)
	{
		strLen=recv(hSock, nameMsg, NAME_SIZE+BUF_SIZE-1, 0);//받기
		if(strLen==-1) 
			return -1;//오류시 종료
		nameMsg[strLen]=0;//메세지 끝
		fputs(nameMsg, stdout);//출력
	}
	return 0;
}
	
void ErrorHandling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
