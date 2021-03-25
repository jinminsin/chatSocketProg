#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <process.h> 

#define BUF_SIZE 100
#define MAX_CLNT 256

unsigned WINAPI HandleClnt(void * arg);
void SendMsg(char * msg, int len);
void ErrorHandling(char * msg);

int clntCnt=0;
SOCKET clntSocks[MAX_CLNT];
HANDLE hMutex;

int main(int argc, char *argv[])
{
	WSADATA wsaData;
	SOCKET hServSock, hClntSock;
	SOCKADDR_IN servAdr, clntAdr;
	int clntAdrSz;
	HANDLE  hThread;

	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}//명령어줄

	if(WSAStartup(MAKEWORD(2, 2), &wsaData)!=0)//윈속 초기화
		ErrorHandling("WSAStartup() error!"); 
  
	hMutex=CreateMutex(NULL, FALSE, NULL);//뮤텍스 오브젝트 생성

	hServSock=socket(PF_INET, SOCK_STREAM, 0);//소켓 연결

	memset(&servAdr, 0, sizeof(servAdr));//구조체 초기화
	servAdr.sin_family=AF_INET; 
	servAdr.sin_addr.s_addr=htonl(INADDR_ANY);
	servAdr.sin_port=htons(atoi(argv[1]));
	
	if(bind(hServSock, (SOCKADDR*) &servAdr, sizeof(servAdr))==SOCKET_ERROR)
		ErrorHandling("bind() error");
	if(listen(hServSock, 5)==SOCKET_ERROR)//연결 대기
		ErrorHandling("listen() error");
	
	while(1)
	{
		clntAdrSz=sizeof(clntAdr);//클라이언트 정보
		hClntSock=accept(hServSock, (SOCKADDR*)&clntAdr,&clntAdrSz);//연결 허가
		
		WaitForSingleObject(hMutex, INFINITE);//뮤텍스 대기(처음에 FALSE 초기화로 인해 첫번째 진입은 바로 허용)
		clntSocks[clntCnt++]=hClntSock;
		ReleaseMutex(hMutex);//대기 해제
	
		hThread=
			(HANDLE)_beginthreadex(NULL, 0, HandleClnt, (void*)&hClntSock, 0, NULL);//클라이언트로부터 데이터를 주고받는 스레드 생성
		printf("Connected client IP: %s \n", inet_ntoa(clntAdr.sin_addr));
	}
	closesocket(hServSock);
	WSACleanup();
	return 0;
}
	
unsigned WINAPI HandleClnt(void * arg)
{
	SOCKET hClntSock=*((SOCKET*)arg);
	int strLen=0, i;
	char msg[BUF_SIZE];
	
	while((strLen=recv(hClntSock, msg, sizeof(msg), 0))!=0)//클라이언트로부터 데이터 받기
		SendMsg(msg, strLen);//모든 클라이언트에 메세지 전달
	
	//종료시 0 반환으로 인해 반복문 탈출

	WaitForSingleObject(hMutex, INFINITE);
	for(i=0; i<clntCnt; i++)   // remove disconnected client
	{
		if(hClntSock==clntSocks[i])
		{
			while(i++<clntCnt-1)
				clntSocks[i]=clntSocks[i+1];
			break;
		}
	}//재정렬
	clntCnt--;
	ReleaseMutex(hMutex);
	closesocket(hClntSock);
	return 0;
}

void SendMsg(char * msg, int len)   // send to all
{
	int i;
	WaitForSingleObject(hMutex, INFINITE);
	for(i=0; i<clntCnt; i++)
		send(clntSocks[i], msg, len, 0);

	ReleaseMutex(hMutex);
}

void ErrorHandling(char * msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}