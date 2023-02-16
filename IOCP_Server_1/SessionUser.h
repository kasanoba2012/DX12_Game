#pragma once
#include "ServerStd.h"
#include "ObjectPool.h"

#define g_iMaxRecvBufferSize 256
#define g_iMaxDataBufferSize (PACKET_MAX_DATA_SIZE+PACKET_HEADER_SIZE*2)

struct OVERLAPPED2 : ObjectPool<OVERLAPPED2>
{
	enum { MODE_RECV = 1, MODE_SEND = 2, MODE_EXIT = 3 };
	OVERLAPPED ov;
	int  iType;
	OVERLAPPED2()
	{
		memset(&ov, 0, sizeof(OVERLAPPED));
		iType = 0;
	}
	OVERLAPPED2(int type)
	{
		// memset : 특정 크기만큼 mem 세팅
		// memset(첫번째 인자 void* ptr은 세팅하고자 하는 메모리의 시작 주소)
		// memset(두번째 인자 value는 메모리에 세팅하고자 하는 값을 집어 넣으면 됩니다)
		// memset(세번째 인자 size_t num은 길이를 뜻합니다)
		// &ov는 m_mPool의 시작 위치 주소 이다 | 즉 m_mPool 시작위치부터 OVERLAPPED 사이즈만큼 0으로 밀어버린다.
		memset(&ov, 0, sizeof(OVERLAPPED));
		iType = type;
	}
};

class SessionUser
{
public:
	bool m_bDisConnect;
	SOCKET m_Sock;
	SOCKADDR_IN m_Address;
	char m_szName[9] = { 0, };
public:
	// 배열 사용의 한계->리스트+배열(무한대)
	//char m_szRecvBuffer[g_iMaxDataBufferSize] = { 0, };
	char m_szDataBuffer[g_iMaxDataBufferSize] = { 0, };
	int m_iPacketPos = 0; // 페킷의 시작 주소
	int m_iWritePos = 0; // 페킷 데이터 저장 주소
	int m_iReadPos = 0; // 페킷 데이터 저장 주소

	WSABUF m_wsaRecvBuffer;
	WSABUF m_wsaSendBuffer;
	OVERLAPPED2 m_RecvOV;
	OVERLAPPED2 m_SendOV;

	std::list<UPACKET> m_RecvPacketList;
	std::list<UPACKET> m_SendPacketList;
public:
	bool    Set(SOCKET sock, SOCKADDR_IN address);
	int     SendMsg(short type, char* msg = nullptr);
	int     SendMsg(UPACKET& packet);
	int     RecvMsg();
	void    DispatchRead(DWORD dwTrans, OVERLAPPED2* pOV2);
	void    DispatchSend(DWORD dwTrans, OVERLAPPED2* pOV2);
public:
	SessionUser()
	{
		ZeroMemory(m_szName, sizeof(char) * 1);
	}
};

