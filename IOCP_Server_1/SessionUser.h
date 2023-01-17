#pragma once
#include "ServerStd.h"

#define g_iMaxDataBufferSize 255

struct OVERLAPPED2 : OVERLAPPED
{
	enum { MODE_RECV = 1, MODE_SEND = 2, MODE_EXIT =3};
	int iType;
	OVERLAPPED2()
	{
		iType = 0;
	}
	OVERLAPPED2(int type)
	{
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
	char m_szRecvBuffer[g_iMaxDataBufferSize] = { 0, };
	char m_szDataBuffer[g_iMaxDataBufferSize] = { 0, };
	int m_iPacketPos = 0; // 其哦狼 矫累 林家
	int m_iWritePos = 0; // 其哦 单捞磐 历厘 林家
	int m_iReadPos = 0; // 其哦 单捞磐 历厘 林家

	WSABUF m_wsaRecvBuffer;
	WSABUF m_wsaSendBuffer;
	OVERLAPPED2 m_RecvOV;
	OVERLAPPED2 m_SendOV;

	std::list<UPACKET> m_RecvPacketList;
	std::list<UPACKET> m_SendPacketList;
public:
	bool Set(SOCKET sock, SOCKADDR_IN address);
	int SendMsg(short type, char* msg = nullptr);
	int SendMsg(UPACKET& packet);
	int RecvMsg();
	void DispatchRead(DWORD dwTrans);
	void DispatchSend(DWORD dwTrans);
};

