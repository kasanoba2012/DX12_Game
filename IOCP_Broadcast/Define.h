#pragma once

#define PACKET_HEADER_SIZE 4

#include <winsock2.h>
#include <Ws2tcpip.h>

const UINT32 MAX_SOCKBUF = 256;	//��Ŷ ũ��
const UINT32 MAX_SOCK_SENDBUF = 4096;	// ���� ������ ũ��
const UINT32 MAX_WORKERTHREAD = 4;  //������ Ǯ�� ���� ������ ��

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

//WSAOVERLAPPED����ü�� Ȯ�� ���Ѽ� �ʿ��� ������ �� �־���.
struct stOverlappedEx
{
	WSAOVERLAPPED wsa_overlapped_;		//Overlapped I/O����ü
	SOCKET		socket_client_;			//Ŭ���̾�Ʈ ����
	WSABUF		wsa_buf_;				//Overlapped I/O�۾� ����
	IOOperation E_operation_;			//�۾� ���� ����
};

#define PACKET_CHAR_MSG   1000      // client ->
#define PACKET_CHATNAME_REQ   1001  // server -> client
#define PACKET_NAME_REQ   2000		// client -> server
#define PACKET_NAME_ACK   3000		// server -> client
#define PACKET_JOIN_USER  4000		// server -> client