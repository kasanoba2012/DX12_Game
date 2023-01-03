// Socket_1_Client.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <winsock2.h>

int main()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        return 0;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

    short sData = 10000;
    short tData = 0x2710;
    short fData = 0x1027;

    SOCKADDR_IN sa; // 목적지+포트
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = inet_addr("192.168.0.157");
    sa.sin_port = htons(10000);
    int iRet = connect(sock, (sockaddr*)&sa, sizeof(sa));
    if (iRet == SOCKET_ERROR)
    {
        int iError = WSAGetLastError();
        printf("%d ", iError);
        return 1;
    }

    u_long iMode = TRUE;
    ioctlsocket(sock, FIONBIO, &iMode);

    while (1)
    {
        char szSendMsg[256] = { 0, };
        printf("%s", "send---->");
        fgets(szSendMsg, 256, stdin);

        int iSendBytes = send(sock, szSendMsg, strlen(szSendMsg), 0);
        if (iSendBytes == SOCKET_ERROR)
        {
            // WSAEWOULDBLOCK 데이터는 받았는데 빈데이터인 경우
            if (WSAGetLastError() != WSAEWOULDBLOCK)
            {
                //WSAEWOULDBLOCK 아니라면 빈데이터가 아니라 진짜 오류발생!
                closesocket(sock);
                return 1;
            }
            continue;
        }

        char szRecvMsg[256] = { 0, };
        int iRecvBytes = recv(sock, szRecvMsg, 256, 0);
        if (iRecvBytes == SOCKET_ERROR)
        {
            // WSAEWOULDBLOCK 데이터는 받았는데 빈데이터인 경우
            if (WSAGetLastError() != WSAEWOULDBLOCK)
            {
                //WSAEWOULDBLOCK 아니라면 빈데이터가 아니라 진짜 오류발생!
                closesocket(sock);
                return 1;
            }
            continue;
        }
        printf("Recv---->%s\n", szRecvMsg);
    }
    closesocket(sock);
    WSACleanup();
}