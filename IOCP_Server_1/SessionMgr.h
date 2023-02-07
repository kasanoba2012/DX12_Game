#pragma once
#include "SessionUser.h"
#include "ObjectPool.h"
class SessionMgr
{
	ClassPool<SessionUser> m_Pool;
public:
	std::list<std::shared_ptr<SessionUser>> m_SessionList;
	SessionUser* Add(SOCKET sock, SOCKADDR_IN address);
	void   Delete(std::shared_ptr<SessionUser> user);
	std::list<UPACKET>  m_BroadcasttingPacketList;
	void   SendPrecess();
	// �ʱ� m_ChunkSize ������� 20���� ��������
	SessionMgr() : m_Pool(1)
	{
		OVERLAPPED2::Allocation();
	}
	~SessionMgr()
	{
		OVERLAPPED2::Release();
	}
};

