#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <winsock2.h>

// Winsockt2 서버 기본 개념 https://woo-dev.tistory.com/135

// 서버
int main()
{
    std::cout << "Server Open\n";
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

    /*----------------------
    SOCKET 정보 지정
    -----------------------*/
    SOCKADDR_IN sa;//목적지+포트
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    sa.sin_port = htons(10000);

    /*----------------------
    SOCKET 바인딩
    -----------------------*/
    int iRet = bind(sock, (sockaddr*)&sa, sizeof(sa));
    if (iRet == SOCKET_ERROR)
    {
        return 1;
    }

    /*----------------------
    SOCKET 연결 대기
    -----------------------*/
    iRet = listen(sock, SOMAXCONN);
    if (iRet == SOCKET_ERROR)
    {
        return 1;
    }

    SOCKADDR_IN clientaddr;
    int length = sizeof(clientaddr);
    
    /*----------------------
    클라이언트 연결 수락
    -----------------------*/
    SOCKET clientSock = accept(sock, (sockaddr*)&clientaddr, &length);
    if (clientSock == SOCKET_ERROR)
    {
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    // inet_ntoa() : IP주소 반환 실패시 -1 반환
    // ntohs() : 포트 번호 반환
    printf("클라이언트 접속 : IP:%s, PORT:%d\n",
        inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

    u_long iMode = TRUE;

    /*----------------------
    SOCKET 논블럭킹 설정  | ioctlsocket
    -----------------------*/
    ioctlsocket(clientSock, FIONBIO, &iMode);

    while (1)
    {
        // szRecvMsg : data 저장할 버퍼
        char szRecvMsg[256] = { 0, };
        // recv(연결된 Socket, data 저장 버퍼, 읽을 크기, 읽을 유형 (flags)
        int iRecvBytes = recv(clientSock, szRecvMsg, 256, 0);
        if (iRecvBytes == 0)
        {
            printf("클라이언트 접속 종료 : IP:%s, PORT:%d\n",
                inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
            closesocket(clientSock);
            break;
        }
        if (iRecvBytes == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
            {
                //WSAEWOULDBLOCK 아니라면 오류!
                closesocket(sock);
                return 1;
            }
        }
        else
        {
            printf("%s\n", szRecvMsg);
        }

        if (iRecvBytes > 0)
        {
            int iSendBytes = send(clientSock, szRecvMsg, strlen(szRecvMsg), 0);
            if (iSendBytes == SOCKET_ERROR)
            {
                if (WSAGetLastError() != WSAEWOULDBLOCK)
                {
                    //WSAEWOULDBLOCK 아니라면 오류!
                    closesocket(clientSock);
                    break;
                }
            }
        }
    }
    closesocket(clientSock);

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
