#include "SessionMgr.h"

SessionUser* SessionMgr::Add(SOCKET sock, SOCKADDR_IN address)
{
	SessionUser* user = new SessionUser;
	user->Set(sock, address);
	m_SessionList.push_back(user);

	return user;
}
