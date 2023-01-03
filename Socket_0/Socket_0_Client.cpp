// Socket_0.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include<WinSock2.h>

// 클라이언트
int main()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        return 0;
    }

    // TCP 소켓 사용시
    //SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    // UDP 소켓 사용시
    //SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

    short sData = 10000;
    short tData = 0x2710;
    short fData = 0x1027;

    SOCKADDR_IN sa; // 목적지+포트
    sa.sin_family = AF_INET;

    sa.sin_addr.s_addr = inet_addr("192.168.0.157"); // 접속 IP
    sa.sin_port = htons(10000); // 접속 포트
    int iRet = connect(sock, (sockaddr*)&sa, sizeof(sa));
    if (iRet == SOCKET_ERROR) { return 1; }
    char szSendMsg[256] = { 0, };
    fgets(szSendMsg, 256, stdin);
    // 메세지 발신
    int iSendBytes = send(sock, szSendMsg, strlen(szSendMsg), 0);

    // 받는 메시지 배열 256크기로 생성
    char szRecvMsg[256] = { 0, };
    // 메시지 수신 (보내는 메세지가 256을 초과하는 경우 256 뒷자리는 못받음)
    int iRecvBytes = recv(sock, szRecvMsg, 256, 0);
    printf("%s\n", szRecvMsg);
    closesocket(sock);
    WSACleanup();

    std::cout << "Hello World!\n";
}