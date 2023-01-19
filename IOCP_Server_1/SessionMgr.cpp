#include "SessionMgr.h"

SessionUser* SessionMgr::Add(SOCKET sock, SOCKADDR_IN address)
{
    //u_long iMode = TRUE;
    //ioctlsocket(sock, FIONBIO, &iMode);
    auto user = m_Pool.NewChunk();
    user->Set(sock, address);
    m_SessionList.push_back(user);
    return user.get();
}
void   SessionMgr::Delete(std::shared_ptr<SessionUser> user)
{
    m_Pool.DeleteChunk(user);
}
void   SessionMgr::SendPrecess()
{
    for (auto& packet : m_BroadcasttingPacketList)
    {
        for (auto iterSend = m_SessionList.begin();
            m_SessionList.end() != iterSend; iterSend++)
        {
            SessionUser* pUser = (*iterSend).get();
            pUser->SendMsg(packet);
        }
    }
    m_BroadcasttingPacketList.clear();
}