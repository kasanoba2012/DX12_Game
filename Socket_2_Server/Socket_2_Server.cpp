﻿// Socket_2_Server.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <list>
#include <winsock2.h>

using namespace std;
struct UserInfo
{
    SOCKET sock;
    SOCKADDR_IN address;
};

list<UserInfo> userList;

DWORD WINAPI ServerThread(LPVOID lpThreadParameter)
{
    printf("Server Open\n");
    while(1)
    {
        for (auto iterRecv = userList.begin();
            userList.end() != iterRecv; )
        {
            char szRecvMsg[256] = { 0, };
            int iRecvBytes = recv(iterRecv->sock, szRecvMsg, 256, 0);
            if (iRecvBytes == 0)
            {
                printf("클라이언트 접속 종료 : IP : %s, PORT : %d\n", inet_ntoa(iterRecv->address.sin_addr), ntohs(iterRecv->address.sin_port));
                // 소켓 닫기
                closesocket(iterRecv->sock);
                // 현재 연결 소켓 유저리스트에서 삭제처리
                iterRecv = userList.erase(iterRecv);
                continue;
            }
            if (iRecvBytes == SOCKET_ERROR)
            {
                if (WSAGetLastError() != WSAEWOULDBLOCK)
                {
                    //WSAEWOULDBLOCK 아니라면 오류!
                    closesocket(iterRecv->sock);
                    iterRecv = userList.erase(iterRecv);
                    continue;
                }
            }
            if (iRecvBytes > 0)
            {
                printf("%s\n", szRecvMsg);
                for (auto iterSend = userList.begin();
                    userList.end() != iterSend;)
                {
                    int iSendBytes = send(iterSend->sock, szRecvMsg, strlen(szRecvMsg), 0);
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
            }
            iterRecv++;
        }
    }
}

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
    int iRet = bind(listenSock, (sockaddr*)&sa, sizeof(sa));
    if (iRet == SOCKET_ERROR)
    {
        return 1;
    }
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


    }
    closesocket(listenSock);

    WSACleanup();
}