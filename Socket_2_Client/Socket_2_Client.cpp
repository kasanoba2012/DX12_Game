// Socket_0.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <winsock2.h>
#include "Protocol.h"

// Winsockt2 서버 기본 개념 https://woo-dev.tistory.com/135

/*----------------------
데이터 발신 로직
-----------------------*/
int SendMsg(SOCKET sock, char* msg, short type)
{
    UPACKET packet;
    ZeroMemory(&packet, sizeof(UPACKET));
    // 패킷 길이 : 보낼 문자열 길이 + PACKET_HEADER_SIZE (4)
    packet.ph.len = strlen(msg) + PACKET_HEADER_SIZE;
    // 패킷 타입
    packet.ph.type = type;
    // memcpy(복사받을 포인터, 복사할 포인터, 복사 데이터 길이)
    // SendMsg()에서 보낼 데이터를 Packet에 포장
    memcpy(packet.msg, msg, strlen(msg));

    // send() 함수를 사용하기 위해 char*로 형변환
    char* msgSend = (char*)&packet;

    int iSendBytes = send(sock, msgSend, packet.ph.len, 0);

    // Send 에러 체크
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

// 데이터 전송 쓰레드
DWORD WINAPI SendThread(LPVOID lpThreadParameter)
{
    SOCKET sock = (SOCKET)lpThreadParameter;
    while (1)
    {
        char szSendMsg[256] = { 0, };
        printf("%s ", "->");
        fgets(szSendMsg, 256, stdin);
        szSendMsg[strlen(szSendMsg) - 1] = 0;
        // strcmp : 문자열 비교 함수
        // exit 채팅이 들어오면 break 처리 (실제에서는 종료 처리)
        if (strcmp(szSendMsg, "exit") == 0)
        {
            break;
        }
        if (SendMsg(sock, szSendMsg, PACKET_CHAR_MSG) < 0)
        {
            break;
        }
    }
    closesocket(sock);
};
int main()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        return 0;
    }

    //SOCKET sock1 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //SOCKET sock2 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    // 블록형 소켓 <-> 넌블록형 소켓

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
    sa.sin_addr.s_addr = inet_addr("127.0.0.1");
    // sin_port : 사용할 포트 지정
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

    // win api
    DWORD dwThreadID;
    // (SendThread) 쓰레드 생성
    HANDLE hClient = CreateThread(0, 0, SendThread,
        (LPVOID)sock, CREATE_SUSPENDED, &dwThreadID);

    int iRecvPacketSize = PACKET_HEADER_SIZE;
    int iTotalRecvBytes = 0;
    while (1)
    {
        Sleep(1);

        char szRecvMsg[256] = { 0, };
        int iRecvBytes = recv(sock, szRecvMsg,
            PACKET_HEADER_SIZE - iTotalRecvBytes, 0);

        if (iRecvBytes == 0)
        {
            printf("서버 정상 종료\n");
            break;
        }

        // 전송 받은 데이터를 전체 전송 데이터에 저장
        iTotalRecvBytes += iRecvBytes;

        // iTotalRecvBytes 패킷 헤더와 사이즈가 같다면 헤더 데이터만 전송 받음
        // (iRecvPacketSize 값은 초기의 PACKET_HEADER_SIZE 4로 헤더 값과 같이 세팅되어 있다)
        if (iTotalRecvBytes == PACKET_HEADER_SIZE)
        {
            UPACKET packet;
            ZeroMemory(&packet, sizeof(UPACKET));
            // memcpy(복사받을 포인터, 복사할 포인터, 복사 데이터 길이)
            memcpy(&packet.ph, szRecvMsg, PACKET_HEADER_SIZE);

            char* msg = (char*)&packet;
            int iNumRecvByte = 0;

            do {
                // 전송 받은 데이터가 헤더 정보만 있다면!
                if (packet.ph.len == 4)
                {
                    break;
                }
                int iRecvBytes = recv(sock,
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
                        printf("에러 : %d", WSAGetLastError());
                        //WSAEWOULDBLOCK 아니라면 오류!
                        closesocket(sock);
                        printf("서버 비정상 종료\n");
                        return 1;
                    }
                    continue;
                }
                iNumRecvByte += iRecvBytes;

            } // do end
            
            while ((packet.ph.len - PACKET_HEADER_SIZE) > iNumRecvByte);

            // 헤더 타입 별 처리
            switch (packet.ph.type)
            {
            // 메세지 출력 헤더
            case PACKET_CHAR_MSG:
            {
                printf("%s\n", packet.msg);
            }break;

            // 초기 진입 헤더 (이름 입력 받기) Server에 접속하면 Server가 SendMsg(clientSock, nullptr, PACKET_CHATNAME_REQ);으로 호출한다
            case PACKET_CHATNAME_REQ:
            {
                printf("이름을 입력하시오 : ");
                char szName[256] = { 0, };
                fgets(szName, 256, stdin);
                szName[strlen(szName) - 1] = 0;
                // 이름 전송 후 PACKET_NAME_REQ 이름 입력 헤더와 함께 서버로 전송
                SendMsg(sock, szName, PACKET_NAME_REQ);
                
                /*----------------------
                Thread 호출
                -----------------------*/
                ResumeThread(hClient);
            }break;

            // 유저 입장 헤더
            case PACKET_JOIN_USER:
            {
                printf("%s %s\n", packet.msg, "님이 입장하였습니다.");
            }break;

            // 이름 허용 헤더
            case PACKET_NAME_ACK:
            {
                printf("대화명 사용 승인\n");
            }break;
            }

            iTotalRecvBytes = 0;
        }


    }

    CloseHandle(hClient);
    closesocket(sock);
    WSACleanup();

    Sleep(5000);
}