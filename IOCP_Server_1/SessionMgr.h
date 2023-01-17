#pragma once
#include "SessionUser.h"
class SessionMgr
{
public:
	std::list<SessionUser*> m_SessionList;
	SessionUser* Add(SOCKET sock, SOCKADDR_IN address);
};

