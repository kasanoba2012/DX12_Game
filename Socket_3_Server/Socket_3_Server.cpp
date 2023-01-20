// Winsockt2 서버 기본 개념 https://woo-dev.tistory.com/135

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <list>
#include <winsock2.h>
#include "Protocol.h"

using namespace std;

struct UserInfo
{
    SOCKET sock;
    SOCKADDR_IN address;
    char _szName[9] = { 0, };
    char szRecvMsg[255] = { 0, };
    int iTotalRecvBytes = 0;
};

// 유저 정보를 담은 리스트 생성
list<UserInfo> userList;

/*----------------------
데이터 발신 로직
-----------------------*/
int SendMsg(SOCKET sock, char* msg, short type)
{
    UPACKET packet;
    ZeroMemory(&packet, sizeof(UPACKET));
    // 메세지가 있다면
    if (msg != nullptr)
    {
        // 패킷 헤더 길이를 4로 설정
        packet.ph.len = PACKET_HEADER_SIZE;
        memcpy(packet.msg, msg, strlen(msg));
    }
    // 메세지가 없다면
    else {
        // 패킷 헤더 길이를 4로 설정
        packet.ph.len = PACKET_HEADER_SIZE;
    }
    packet.ph.type = type;

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
DWORD WINAPI ServerThread(LPVOID lpThreadParameter)
{
    printf("Server Open\n");
    while (1)
    {
        // 유저 리스트 처음부터 끝까지 무한으로 돌려
        for (auto iterRecv = userList.begin();
            userList.end() != iterRecv;)
        {
            // Recv패킷 사이즈를 4로 설정
            int iRecvPacketSize = PACKET_HEADER_SIZE;

            int iRecvBytes = recv(iterRecv->sock, iterRecv->szRecvMsg,
                PACKET_HEADER_SIZE - iterRecv->iTotalRecvBytes, 0);

            if (iRecvBytes == 0)
            {
                printf("클라이언트 접속 종료 : IP : %s, PORT : %d\n",
                    inet_ntoa(iterRecv->address.sin_addr), ntohs(iterRecv->address.sin_port));
                closesocket(iterRecv->sock);
                iterRecv = userList.erase(iterRecv);
                continue;
            }
            DWORD dwError = WSAGetLastError();
            if (iRecvBytes == SOCKET_ERROR)
            {
                // 창 닫음으로 인한 종료
                if (dwError != WSAEWOULDBLOCK)
                {
                    //WSAEWOULDBLOCK 아니라면 오류!
                    printf("클라이언트 접속 종료 : IP : %s, PORT : %d\n",
                        inet_ntoa(iterRecv->address.sin_addr), ntohs(iterRecv->address.sin_port));
                    closesocket(iterRecv->sock);
                    iterRecv = userList.erase(iterRecv);
                }
                else {
                    iterRecv++;
                }
                continue;
            }

            UPACKET packet;
            ZeroMemory(&packet, sizeof(UPACKET));

            iterRecv->iTotalRecvBytes += iRecvBytes;
            if (iterRecv->iTotalRecvBytes == PACKET_HEADER_SIZE)
            {
                memcpy(&packet.ph, iterRecv->szRecvMsg, PACKET_HEADER_SIZE);

                char* msg = (char*)&packet;
                int iNumRecvByte = 0;
                do {
                    int iRecvBytes = recv(iterRecv->sock,
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
                            closesocket(iterRecv->sock);
                            printf("서버 비정상 종료\n");
                            return 1;
                        }
                        continue;
                    }
                    iNumRecvByte += iRecvBytes;
                } while ((packet.ph.len - PACKET_HEADER_SIZE) > iNumRecvByte);
            }
            if (iRecvBytes > 0)
            {
                switch (packet.ph.type)
                {
                case PACKET_CHAR_MSG:
                {
                    printf("[%s] %s\n", iterRecv->_szName, packet.msg);
                    packet.ph.len += strlen(iterRecv->_szName) + 2;
                    string pMsg = "[";
                    pMsg += packet.msg;
                    pMsg += "]";
                    pMsg += packet.msg;
                    ZeroMemory(packet.msg, 2048);
                    memcpy(packet.msg, pMsg.c_str(), pMsg.size());
                }break;

                case PACKET_NAME_REQ:
                {
                    memcpy(iterRecv->_szName, packet.msg, strlen(packet.msg));
                    packet.ph.type = PACKET_JOIN_USER;
                    // 대화명 승인 헤더 전송
                    SendMsg(iterRecv->sock, nullptr, PACKET_NAME_ACK);
                }break;
                }

                for (auto iterSend = userList.begin();
                    userList.end() != iterSend;)
                {
                    if (iterSend == iterRecv)
                    {
                        iterSend++;
                        continue;
                    }
                    int iSendBytes = send(iterSend->sock, (char*)&packet, packet.ph.len, 0);

                    if (iSendBytes == SOCKET_ERROR)
                    {
                        if (WSAGetLastError() != WSAEWOULDBLOCK)
                        {
                            //WSAEWOULDBLOCK 아니라면 오류!
                            printf("클라이언트 접속 비정상 종료 : IP:%s, PORT:%d\n",
                                inet_ntoa(iterSend->address.sin_addr), ntohs(iterSend->address.sin_port));

                            closesocket(iterSend->sock);
                            iterSend = userList.erase(iterSend);
                            continue;
                        }
                    }

                    iterSend++;
                }
                ZeroMemory(&packet, sizeof(UPACKET));
                iterRecv->iTotalRecvBytes = 0;
            }
            iterRecv++;
        }
    }
}; // ServerThread End

int main()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        return 0;
    }

    SOCKET listenSock = socket(AF_INET, SOCK_STREAM, 0);
    short sData = 10000;
    short tData = 0x2710;
    short fData = 0x1027;

    // client
    SOCKADDR_IN sa;//목적지+포트
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    sa.sin_port = htons(10000);

    /*----------------------
    SOCKET 바인딩
    -----------------------*/
    int iRet = bind(listenSock, (sockaddr*)&sa, sizeof(sa));
    if (iRet == SOCKET_ERROR)
    {
        return 1;
    }

    /*----------------------
    SOCKET 연결 대기
    -----------------------*/
    iRet = listen(listenSock, SOMAXCONN);
    if (iRet == SOCKET_ERROR)
    {
        return 1;
    }

    DWORD dwThreadID;
    HANDLE hClient = CreateThread(0, 0, ServerThread,
        0, 0, &dwThreadID);

    while (1)
    {
        // 접속되면 반환된다.
        SOCKADDR_IN clientaddr;
        int length = sizeof(clientaddr);

        /*----------------------
        클라이언트 연결 수락
        -----------------------*/
        SOCKET clientSock = accept(listenSock, (sockaddr*)&clientaddr, &length);
        if (clientSock == SOCKET_ERROR)
        {
            closesocket(listenSock);
            WSACleanup();
            return 1;
        }
        printf("클라이언트 접속 : IP:%s, PORT:%d\n",
            inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

        u_long iMode = TRUE;
        ioctlsocket(clientSock, FIONBIO, &iMode);

        UserInfo user;
        user.sock = clientSock;
        user.address = clientaddr;
        userList.push_back(user);

        // 클라이언트 이름 호출 헤더 전송
        SendMsg(clientSock, nullptr, PACKET_CHATNAME_REQ);
    }
    closesocket(listenSock);

    WSACleanup();
}