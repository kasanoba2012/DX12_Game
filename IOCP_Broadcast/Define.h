#pragma once

#define PACKET_HEADER_SIZE 4

#include <winsock2.h>
#include <Ws2tcpip.h>

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
	WSAOVERLAPPED wsa_overlapped_;		//Overlapped I/O구조체
	SOCKET		socket_client_;			//클라이언트 소켓
	WSABUF		wsa_buf_;				//Overlapped I/O작업 버퍼
	IOOperation E_operation_;			//작업 동작 종류
};

#define PACKET_CHAR_MSG   1000      // client ->
#define PACKET_CHATNAME_REQ   1001  // server -> client
#define PACKET_NAME_REQ   2000		// client -> server
#define PACKET_NAME_ACK   3000		// server -> client
#define PACKET_JOIN_USER  4000		// server -> client