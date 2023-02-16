// Recv_Socket_0.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
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

    /*----------------------
    SOCKET 생성
    -----------------------*/
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

    short sData = 10000;
    short tData = 0x2710;
    short fData = 0x1027;

    SOCKADDR_IN sa; // 목적지+포트
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = inet_addr("127.0.0.1");
    sa.sin_port = htons(10000);

    /*----------------------
    SOCKET 연결
    -----------------------*/
    int iRet = connect(sock, (sockaddr*)&sa, sizeof(sa));
    if (iRet == SOCKET_ERROR)
    {
        int iError = WSAGetLastError();
        printf("%d ", iError);
        return 1;
    }

    u_long iMode = TRUE;
    /*----------------------
    SOCKET 논블럭킹 설정  | ioctlsocket
    -----------------------*/
    ioctlsocket(sock, FIONBIO, &iMode);

    while (1)
    {
        /*----------------------
         데이터 발신 로직
         -----------------------*/
        //char szSendMsg[256] = { 0, };
        //printf("%s", "send---->");
        //fgets(szSendMsg, 256, stdin);

        //int iSendBytes = send(sock, szSendMsg, strlen(szSendMsg), 0);
        //if (iSendBytes == SOCKET_ERROR)
        //{
        //    // WSAEWOULDBLOCK 데이터는 받았는데 빈데이터인 경우
        //    if (WSAGetLastError() != WSAEWOULDBLOCK)
        //    {
        //        //WSAEWOULDBLOCK 아니라면 빈데이터가 아니라 진짜 오류발생!
        //        closesocket(sock);
        //        return 1;
        //    }
        //    continue;
        //}

        /*----------------------
         데이터 수신 로직
         -----------------------*/
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
        printf("Recv----> 몬스터의 현재 X 좌표 : %d\n", szRecvMsg[0]);
    }
    closesocket(sock);
    WSACleanup();
}

// 프로그램 실행: <Ctrl+F5> 또는 [디버그] > [디버깅하지 않고 시작] 메뉴
// 프로그램 디버그: <F5> 키 또는 [디버그] > [디버깅 시작] 메뉴

// 시작을 위한 팁: 
//   1. [솔루션 탐색기] 창을 사용하여 파일을 추가/관리합니다.
//   2. [팀 탐색기] 창을 사용하여 소스 제어에 연결합니다.
//   3. [출력] 창을 사용하여 빌드 출력 및 기타 메시지를 확인합니다.
//   4. [오류 목록] 창을 사용하여 오류를 봅니다.
//   5. [프로젝트] > [새 항목 추가]로 이동하여 새 코드 파일을 만들거나, [프로젝트] > [기존 항목 추가]로 이동하여 기존 코드 파일을 프로젝트에 추가합니다.
//   6. 나중에 이 프로젝트를 다시 열려면 [파일] > [열기] > [프로젝트]로 이동하고 .sln 파일을 선택합니다.
