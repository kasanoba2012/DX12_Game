#pragma once

#define PACKET_HEADER_SIZE 4

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <mswsock.h>

const UINT32 MAX_SOCKBUF = 256;	//��Ŷ ũ��
const UINT32 MAX_SOCK_SENDBUF = 4096;	// ���� ������ ũ��
const UINT64 RE_USE_SESSION_WAIT_TIMESEC = 3;

enum class IOOperation
{
	ACCEPT,
	RECV,
	SEND
};

//WSAOVERLAPPED����ü�� Ȯ�� ���Ѽ� �ʿ��� ������ �� �־���.
struct stOverlappedEx
{
	WSAOVERLAPPED wsa_overlapped_;		//Overlapped I/O����ü
	WSABUF		wsa_buf_;				//Overlapped I/O�۾� ����
	IOOperation E_operation_;			//�۾� ���� ����
	UINT32 session_index_ = 0;
};

typedef struct
{
	short len;
	short type;
}PACKET_HEADER2;

typedef struct
{
	PACKET_HEADER2 ph;
	char msg[2048];
}UPACKET;

#define PACKET_CHAR_MSG   1000      // client ->
#define PACKET_CHATNAME_REQ   1001  // server -> client
#define PACKET_NAME_REQ   2000		// client -> server
#define PACKET_NAME_ACK   3000		// server -> client
#define PACKET_JOIN_USER  4000		// server -> client