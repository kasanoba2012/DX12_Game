#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>

const UINT32 MAX_SOCKBUF = 256;	//패킷 크기
const UINT32 MAX_SOCK_SENDBUF = 4096;	// 소켓 버퍼의 크기
const UINT32 MAX_WORKERTHREAD = 4;  //쓰레드 풀에 넣을 쓰레드 수

typedef struct
{
	short len;
	short type;
}PACKET_HEADER;

typedef struct
{
	PACKET_HEADER ph;
	char msg[2048];
}UPACKET;

enum class IOOperation
{
	RECV,
	SEND
};

//WSAOVERLAPPED구조체를 확장 시켜서 필요한 정보를 더 넣었다.
struct stOverlappedEx
{
	PACKET_HEADER ph; 
	WSAOVERLAPPED m_wsaOverlapped;		//Overlapped I/O구조체
	SOCKET		m_socketClient;			//클라이언트 소켓
	WSABUF		m_wsaBuf;				//Overlapped I/O작업 버퍼
	char		m_szBuf[MAX_SOCKBUF]; //데이터 버퍼
	IOOperation m_eOperation;			//작업 동작 종류
};