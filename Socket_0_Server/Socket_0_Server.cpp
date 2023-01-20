// Socket_1.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <WinSock2.h>

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
    // sin_family : 구조체를 구분하기 위한 변수
    sa.sin_family = AF_INET;
    // sin_addr : 자신의 IP 할당 (IP주소가 2개이상 있지 않으면 INADDR_ANY)로 자동 할당
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    // sin_port : 사용할 포트 지정
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
    // listen (연결 소켓, 대기열의 크기 (Winsock2부터는 SOMAXCONN으로 자동할당))
    iRet = listen(sock, SOMAXCONN);
    if (iRet == SOCKET_ERROR)
    {
        return 1;
    }
    // 접속되면 반환된다.
    SOCKADDR_IN clientaddr;
    int length = sizeof(clientaddr);
    /*----------------------
    SOCKET 연결 수락
    -----------------------*/
    SOCKET clientSock = accept(sock, (sockaddr*)&clientaddr, &length);

    printf("클라이언트 접속 : IP:%s, PORT:%d\n",
        inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

    char szRecvMsg[256] = { 0, };

    /*----------------------
    SOCKET 연결 데이터 받기
    -----------------------*/
    int iRecvBytes = recv(clientSock, szRecvMsg, 256, 0);
    printf("%s\n", szRecvMsg);

    /*----------------------
    SOCKET 연결 데이터 보내기
    -----------------------*/
    int iSendBytes = send(clientSock, szRecvMsg, strlen(szRecvMsg), 0);

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
