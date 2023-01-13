#include "TNetwork.h"

TUser* TNetwork::UserFind(SOCKET sock)
{
    // 유저리스트에 있는 소켓 모두를 뒤져라
    for (auto iterRecv = userlist.begin(); userlist.end() != iterRecv; iterRecv++)
    {
        // 유저리스트 소캣에 있는 소켓과 UserFind에서 찾고자 하는 소켓과 일치하는 경우 &(*iterRecv) 반환
        if (sock == iterRecv->sock)
        {
            return &(*iterRecv);
        }
    }
    // 일치하는 게 없는 경우 nullptr 리턴
    return nullptr;
}
void TNetwork::PacketProcess()
{
    // WSAWaitForMultipleEvents : 지정된 이벤트 객체의 하나 혹은 모두가 신호상태를 기다린다.
    DWORD dwIndex = WSAWaitForMultipleEvents(userlist.size() + 1, m_EventArray, FALSE, 0, FALSE);
    if (dwIndex == WSA_WAIT_FAILED) return;
    if (dwIndex == WSA_WAIT_TIMEOUT) return;
    dwIndex -= WSA_WAIT_EVENT_0;
    if (dwIndex == 0)
    {
        WSANETWORKEVENTS netEvent;
        int iRet = WSAEnumNetworkEvents(m_Sock, m_EventArray[dwIndex], &netEvent);
        if (iRet != SOCKET_ERROR)
        {
            if (netEvent.lNetworkEvents & FD_ACCEPT)
            {
                // 접속되면 반환된다.
                SOCKADDR_IN clientaddr;
                int length = sizeof(clientaddr);
                SOCKET clientSock = accept(m_Sock, (sockaddr*)&clientaddr, &length);
                // 소켓 에러인 경우
                if (clientSock == SOCKET_ERROR)
                {
                    closesocket(m_Sock);
                    WSACleanup();
                    return;
                }
                m_Print({ L"클라이언트 접속->IP:" + to_mw(inet_ntoa(clientaddr.sin_addr)),
                              L" PORT:" + std::to_wstring(ntohs(clientaddr.sin_port)) });

                u_long iMode = TRUE;
                ioctlsocket(clientSock, FIONBIO, &iMode);

                TUser user;
                user.sock = clientSock;
                user.address = clientaddr;
                userlist.push_back(user);

                HANDLE hListenEvent = WSACreateEvent(); // 수동리셋이벤트
                m_EventArray[userlist.size()] = hListenEvent;
                m_SocketArray[userlist.size()] = clientSock;
                WSAEventSelect(clientSock, hListenEvent, FD_READ | FD_WRITE | FD_CLOSE);

                SendMsg(clientSock, nullptr, 0, PACKET_CHATNAME_REQ);
            }
        }
        return;
    }

    for (int iEvent = dwIndex; iEvent < userlist.size() + 1; iEvent++)
    {
        TUser* user = UserFind(m_SocketArray[iEvent]);
        if (user->m_bExit) continue;
        DWORD dwSignal = WSAWaitForMultipleEvents(1, &m_EventArray[iEvent], TRUE, 0, FALSE);
        if (dwSignal == WSA_WAIT_FAILED) continue;
        if (dwSignal == WSA_WAIT_TIMEOUT) continue;

        WSANETWORKEVENTS netEvent;
        int iRet = WSAEnumNetworkEvents(user->sock, m_EventArray[dwIndex], &netEvent);
        if (iRet != SOCKET_ERROR)
        {
            UPACKET packet;
            ZeroMemory(&packet, sizeof(UPACKET));
            if (netEvent.lNetworkEvents & FD_READ)
            {
                int iRecvPacketSize = PACKET_HEADER_SIZE;

                int iRecvBytes = recv(user->sock, user->szRecvMsg, PACKET_HEADER_SIZE - user->iTotalRecvBytes, 0);
                if (iRecvBytes == 0)
                {
                    user->m_bExit = true;
                    continue;
                }
                DWORD dwError = WSAGetLastError();
                if (iRecvBytes == SOCKET_ERROR)
                {
                    if (dwError != WSAEWOULDBLOCK)
                    {
                        user->m_bExit = true;
                    }
                    continue;
                }
                user->iTotalRecvBytes += iRecvBytes;
                if (user->iTotalRecvBytes == PACKET_HEADER_SIZE)
                {
                    memcpy(&packet.ph, user->szRecvMsg, PACKET_HEADER_SIZE);

                    char* msg = (char*)&packet;
                    int iNumRecvByte = 0;
                    do
                    {
                        int iRecvBytes = recv(user->sock, &packet.msg[iNumRecvByte], packet.ph.len - PACKET_HEADER_SIZE - iNumRecvByte, 0);

                        if (iRecvBytes == 0)
                        {
                            user->m_bExit = true;
                            break;
                        }
                        if (iRecvBytes == SOCKET_ERROR)
                        {
                            if (WSAGetLastError() != WSAEWOULDBLOCK)
                            {
                                user->m_bExit = true;
                            }
                            continue;
                        }
                        iNumRecvByte += iRecvBytes;
                    } while ((packet.ph.len - PACKET_HEADER_SIZE) > iNumRecvByte);
                }
                if (iRecvBytes > 0)
                {
                    if (packet.ph.type == PACKET_NAME_REQ)
                    {
                        SendMsg(user->sock, nullptr, 0, PACKET_NAME_ACK);
                    }
                    else
                    {
                        m_RecvPacketList.push_back(packet);
                    }
                }
                user->iTotalRecvBytes = 0;
            }
            if (netEvent.lNetworkEvents & FD_WRITE)
            {

            }
            if (netEvent.lNetworkEvents & FD_CLOSE)
            {
                user->m_bExit = true;
            }
        }
    }

    for (auto& packet : m_RecvPacketList)
    {
        TNetwork::FunIter iter = m_fnExecutePacket.find(packet.ph.type);
        if (iter != m_fnExecutePacket.end()) 
        {
            TNetwork::CallFuction call = iter->second;
            call(packet);
        }
    }
    m_RecvPacketList.clear();

    for (auto userIter = userlist.begin(); userlist.end() != userIter;)
    {
        if (userIter->m_bExit)
        {
            // 접속 종료시 userlist 삭제 처리
            closesocket(userIter->sock);
            m_Print({ L"클라이언트 종료->IP:" + to_mw(inet_ntoa(userIter->address.sin_addr)),
                        L" PORT:" + std::to_wstring(ntohs(userIter->address.sin_port)) });

            userIter = userlist.erase(userIter);
        }
        else
        {
            userIter++;
        }
    }
}
void TNetwork::MakePacket(UPACKET& packet, const char* msg, int iSize, short type)
{
    // 패킷 만들기
    ZeroMemory(&packet, sizeof(UPACKET));
    packet.ph.len = iSize + PACKET_HEADER_SIZE;
    packet.ph.type = type;
    memcpy(packet.msg, msg, iSize);
}
bool   TNetwork::NetStart(std::string ip, int iPort)
{
    m_Sock = socket(AF_INET, SOCK_STREAM, 0);
    SOCKADDR_IN sa;//목적지+포트
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    sa.sin_port = htons(iPort);
    int iRet = bind(m_Sock, (sockaddr*)&sa, sizeof(sa));
    if (iRet == SOCKET_ERROR)
    {
        return 1;
    }
    iRet = listen(m_Sock, SOMAXCONN);
    if (iRet == SOCKET_ERROR)
    {
        return 1;
    }

    HANDLE hListenEvent = WSACreateEvent(); // 수동리셋이벤트
    m_EventArray[0] = hListenEvent;
    WSAEventSelect(m_Sock, m_EventArray[0], FD_ACCEPT);

    return true;
}

void TNetwork::AddSend(SOCKET sock, const char* data, int iSize, short type)
{
    UPACKET packet;
    MakePacket(packet, data, iSize, type);
    m_BroadCasttingPacketList.push_back(packet);
}

int TNetwork::SendMsg(SOCKET sock, const char* data, int iSize, short type)
{
    UPACKET packet;
    MakePacket(packet, data, iSize, type);

    char* msgSend = (char*)&packet;
    int iSendBytes = send(sock, msgSend, packet.ph.len, 0);

    if (iSendBytes == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSAEWOULDBLOCK)
        {
            //WSAEWOULDBLOCK 아니라면 오류!
            closesocket(sock);
            return -1;
        }
    }
    return 1;
}
void TNetwork::SendPrecess()
{
    for (auto& packet : m_BroadCasttingPacketList)
    {
        for (auto iterSend = userlist.begin(); userlist.end() != iterSend; )
        {
            int iSendBytes = send(iterSend->sock, (char*)&packet,
                packet.ph.len, 0);

            if (iSendBytes == SOCKET_ERROR)
            {
                if (WSAGetLastError() != WSAEWOULDBLOCK)
                {
                    //WSAEWOULDBLOCK 아니라면 오류!
                    printf("클라이언트 접속 비정상 종료 : IP:%s, PORT:%d\n",
                        inet_ntoa(iterSend->address.sin_addr), ntohs(iterSend->address.sin_port));
                    closesocket(iterSend->sock);
                    iterSend = userlist.erase(iterSend);
                    continue;
                }
            }
            iterSend++;
        }
    }
    m_BroadCasttingPacketList.clear();
}
void TNetwork::RecvPrecess()
{
    int iRecvPacketSize = PACKET_HEADER_SIZE;
    int iTotalRecvBytes = 0;
    while (1)
    {
        char szRecvMsg[256] = { 0, };
        int iRecvBytes = recv(m_Sock, szRecvMsg,
            PACKET_HEADER_SIZE - iTotalRecvBytes, 0);
        if (iRecvBytes == 0)
        {
            printf("서버 정상 종료\n");
            return;
        }
        if (iRecvBytes == SOCKET_ERROR)
        {
            DWORD dwError = WSAGetLastError();
            if (dwError != WSAEWOULDBLOCK)
            {
                closesocket(m_Sock);
            }
            return;
        }
        iTotalRecvBytes += iRecvBytes;
        if (iTotalRecvBytes == PACKET_HEADER_SIZE)
        {
            UPACKET packet;
            ZeroMemory(&packet, sizeof(UPACKET));
            memcpy(&packet.ph, szRecvMsg, PACKET_HEADER_SIZE);

            char* msg = (char*)&packet;
            int iNumRecvByte = 0;
            do {
                if (packet.ph.len == 4)
                {
                    break;
                }
                int iRecvBytes = recv(m_Sock,
                    &packet.msg[iNumRecvByte],
                    packet.ph.len - PACKET_HEADER_SIZE - iNumRecvByte, 0);

                if (iRecvBytes == 0)
                {
                    printf("서버 정상 종료\n");
                    break;
                }
                if (iRecvBytes == SOCKET_ERROR)
                {
                    if (WSAGetLastError() != WSAEWOULDBLOCK)
                    {
                        //WSAEWOULDBLOCK 아니라면 오류!
                        closesocket(m_Sock);
                        printf("서버 비정상 종료\n");
                        return;
                    }
                    continue;
                }
                iNumRecvByte += iRecvBytes;


            } while ((packet.ph.len - PACKET_HEADER_SIZE) > iNumRecvByte);

            m_RecvPacketList.push_back(packet);
            iTotalRecvBytes = 0;
            return;
        }
    }
};
bool TNetwork::Frame()
{    
    return true;
}
bool TNetwork::Render()
{
    return true;
}
bool TNetwork::Release()
{
    closesocket(m_Sock);
    return true;
}
TNetwork::TNetwork()
{
	WSADATA ws;
	WSAStartup(MAKEWORD(2, 2), &ws);
}

TNetwork::~TNetwork()
{
	WSACleanup();
}