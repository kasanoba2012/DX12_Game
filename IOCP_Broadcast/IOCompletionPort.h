#pragma once
#pragma comment(lib, "ws2_32")

#include "ClientInfo.h"
#include "Define.h"
#include <thread>
#include <vector>
#include <string.h>

class IOCompletionPort
{
public:
	IOCompletionPort(void) {}

	~IOCompletionPort(void)
	{
		//윈속의 사용을 끝낸다.
		WSACleanup();
	}

	//소켓을 초기화하는 함수
	bool InitSocket()
	{
		WSADATA wsa_data;

		int nRet = WSAStartup(MAKEWORD(2, 2), &wsa_data);
		if (0 != nRet)
		{
			printf("[에러] WSAStartup()함수 실패 : %d\n", WSAGetLastError());
			return false;
		}

		//연결지향형 TCP , Overlapped I/O 소켓을 생성
		listen_socket_ = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);

		if (INVALID_SOCKET == listen_socket_)
		{
			printf("[에러] socket()함수 실패 : %d\n", WSAGetLastError());
			return false;
		}

		printf("소켓 초기화 성공\n");
		return true;
	}

	//------서버용 함수-------//
	//서버의 주소정보를 소켓과 연결시키고 접속 요청을 받기 위해 
	//소켓을 등록하는 함수
	bool BindandListen(int nBindPort)
	{
		SOCKADDR_IN		stServerAddr;
		stServerAddr.sin_family = AF_INET;
		stServerAddr.sin_port = htons(nBindPort); //서버 포트를 설정한다.				
		stServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

		//위에서 지정한 서버 주소 정보와 cIOCompletionPort 소켓을 연결한다.
		int nRet = bind(listen_socket_, (SOCKADDR*)&stServerAddr, sizeof(SOCKADDR_IN));
		if (0 != nRet)
		{
			printf("[에러] bind()함수 실패 : %d\n", WSAGetLastError());
			return false;
		}

		nRet = listen(listen_socket_, 5);
		if (0 != nRet)
		{
			printf("[에러] listen()함수 실패 : %d\n", WSAGetLastError());
			return false;
		}

		printf("서버 등록 성공..\n");
		return true;
	}

	bool StartServer(const UINT32 maxClientCount)
	{
		// 클라이언트 구조체 미리 쭉 만들어놓기
		CreateClient(maxClientCount);

		//CompletionPort객체 생성 요청을 한다.
		iocp_handle_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, MAX_WORKERTHREAD);
		if (NULL == iocp_handle_)
		{
			printf("[에러] CreateIoCompletionPort()함수 실패: %d\n", GetLastError());
			return false;
		}

		bool bRet = CreateWokerThread();
		if (false == bRet) {
			return false;
		}

		bRet = CreateAccepterThread();
		if (false == bRet) {
			return false;
		}

		CreateSendThread();

		printf("서버 시작\n");
		return true;
	}

	//생성되어있는 쓰레드를 파괴한다.
	void DestroyThread()
	{

		// SendThread 파괴

		is_sender_run_ = false;

		if (send_thread_.joinable())
		{
			send_thread_.join();
		}

		// Accepter 쓰레드를 종료한다.
		is_accepter_run_ = false;
		closesocket(listen_socket_);

		if (accepter_thread_.joinable())
		{
			accepter_thread_.join();
		}

		// WorkThread 종료
		is_worker_run_ = false;
		CloseHandle(iocp_handle_);

		for (auto& th : io_worker_threads_)
		{
			if (th.joinable())
			{
				th.join();
			}
		}

		
	}

	bool SendMsg(const UINT32 clientIndex, const UINT32 dataSize, char* pData)
	{
		auto pClient = GetClientInfo(clientIndex);
		return pClient->SendMsg(dataSize, pData);
	}

	char* mainSendMsg(char* pMsg)
	{
		//SendMsg(pClientInfo, pMsg, sizeof(pMsg));
		// TODO 모든 데이터 저장 첫번째 벡터에 소켓에 발송 이새끼를 브로드 캐스트 하면 전체발송
		//for (auto& broadastList : _ClientInfos)
		//{
		//	SendMsg(&broadastList, pMsg, sizeof(pMsg));
		//}
		//	
		//return pMsg;
		return nullptr;
	}

	// 네트워크 이벤트를 처리할 함수들
	virtual void OnConnect(const UINT32 clientIndex_) {}
	virtual void OnClose(const UINT32 clientIndex_) {}
	virtual void OnReceive(const UINT32 clientIndex_, const UINT32 size_, char* pData_) {}

private:
	void CreateClient(const UINT32 maxClientCount)
	{
		for (UINT32 i = 0; i < maxClientCount; ++i)
		{
			// 접속 허용 클라이언트 만큼 빈 stClientInfo 구조체 할당
			auto clinet = new stClientInfo();
			clinet->Init(i);
			
			client_Infos_.push_back(clinet);
		}
	}

	//WaitingThread Queue에서 대기할 쓰레드들을 생성
	bool CreateWokerThread()
	{
		unsigned int uiThreadId = 0;
		//WaingThread Queue에 대기 상태로 넣을 쓰레드들 생성 권장되는 개수 : (cpu개수 * 2) + 1 
		for (int i = 0; i < MAX_WORKERTHREAD; i++)
		{
			io_worker_threads_.emplace_back([this]() { WokerThread(); });
		}

		printf("WokerThread 시작..\n");
		return true;
	}

	//사용하지 않는 클라이언트 정보 구조체를 반환한다.
	stClientInfo* GetEmptyClientInfo()
	{
		for (auto& clinet : client_Infos_)
		{
			if (clinet->IsConnected() == false)
			{
				return clinet;
			}
		}
		return nullptr;
	}

	stClientInfo* GetClientInfo(const UINT32 clientIndex)
	{
		return client_Infos_[clientIndex];
	}

	//accept요청을 처리하는 쓰레드 생성
	bool CreateAccepterThread()
	{
		accepter_thread_ = std::thread([this]() { AccepterThread(); });

		printf("AccepterThread 시작..\n");
		return true;
	}

	void CreateSendThread()
	{
		is_sender_run_ = true;
		send_thread_ = std::thread([this]() {SendThread();  });
		printf("SendThread 시작..\n");
	}

	//Overlapped I/O작업에 대한 완료 통보를 받아 
	//그에 해당하는 처리를 하는 함수
	void WokerThread()
	{
		//CompletionKey를 받을 포인터 변수
		stClientInfo* P_client_info = nullptr;
		//함수 호출 성공 여부
		BOOL bSuccess = TRUE;
		//Overlapped I/O작업에서 전송된 데이터 크기
		DWORD dwIoSize = 0;
		//I/O 작업을 위해 요청한 Overlapped 구조체를 받을 포인터
		LPOVERLAPPED lpOverlapped = NULL;

		while (is_worker_run_)
		{
			bSuccess = GetQueuedCompletionStatus(iocp_handle_,
				&dwIoSize,					// 실제로 전송된 바이트
				(PULONG_PTR)&P_client_info,		// CompletionKey
				&lpOverlapped,				// Overlapped IO 객체
				INFINITE);					// 대기할 시간

			//사용자 쓰레드 종료 메세지 처리..
			if (TRUE == bSuccess && 0 == dwIoSize && NULL == lpOverlapped)
			{
				is_worker_run_ = false;
				continue;
			}

			if (NULL == lpOverlapped)
			{
				continue;
			}

			//client가 접속을 끊었을때..			
			if (FALSE == bSuccess || (0 == dwIoSize && TRUE == bSuccess))
			{
				//printf("socket(%d) 접속 끊김\n", (int)pClientInfo->_socketClient);
				CloseSocket(P_client_info);
				continue;
			}

			auto pOverlappedEx = (stOverlappedEx*)lpOverlapped;

			//Overlapped I/O Recv작업 결과 뒤 처리
			if (IOOperation::RECV == pOverlappedEx->E_operation_)
			{
				// 클라이언트에서 받은 메세지를 PacketData 형태로 변환
				OnReceive(P_client_info->GetIndex(), dwIoSize, P_client_info->RecvBuffer());

				// Recv 완료 했기 때문에 재차 Recv() 호출
				P_client_info->BindRecv();
				//클라이언트에 메세지를 에코한다.

				//if (wellcomeSw == false)
				//	//클라이언트에 메세지를 에코한다.
				//	SendMsg(pClientInfo, pClientInfo->mRecvBuf, dwIoSize);

				//while (wellcomeSw == true)
				//{
				//	//UPACKET returnPacket;
				//	//ZeroMemory(&returnPacket, sizeof(UPACKET));
				//	//// 패킷 길이 : 보낼 문자열 길이 + PACKET_HEADER_SIZE (4)
				//	//returnPacket.ph.len = PACKET_HEADER_SIZE + 3;
				//	//// 패킷 타입
				//	//returnPacket.ph.type = PACKET_NAME_ACK;
				//	//char* returnPacket2 = (char*)&returnPacket;
				//	//SendMsg(pClientInfo, returnPacket2, sizeof(returnPacket2));

				//	wellcomeSw = false;
				//}


				//SendMsg(pClientInfo, pClientInfo->mRecvBuf, dwIoSize);
			}
			//Overlapped I/O Send작업 결과 뒤 처리
			else if (IOOperation::SEND == pOverlappedEx->E_operation_)
			{
				P_client_info->SendCompleted(dwIoSize);

				//printf("[발신] bytes : %d , msg : %d\n", dwIoSize, pClientInfo->mSendBuf[0]);
				//printf("[발신] bytes : %d , msg : %s\n", dwIoSize, pClientInfo->mSendBuf);
			}
			//예외 상황
			else
			{
				printf("socket(%d)에서 예외상황\n", (int)P_client_info->GetIndex());
			}
		}
	}

	//사용자의 접속을 받는 쓰레드
	void AccepterThread()
	{
		SOCKADDR_IN		stClientAddr;
		int nAddrLen = sizeof(SOCKADDR_IN);

		while (is_accepter_run_)
		{
			//접속을 받을 구조체의 인덱스를 얻어온다.
			stClientInfo* pClientInfo = GetEmptyClientInfo();
			if (NULL == pClientInfo)
			{
				printf("[에러] Client Full\n");
				return;
			}

			//클라이언트 접속 요청이 들어올 때까지 기다린다.
			auto newSocket = accept(listen_socket_, (SOCKADDR*)&stClientAddr, &nAddrLen);
			if (INVALID_SOCKET == newSocket)
			{
				continue;
			}

			if (pClientInfo->OnConnect(iocp_handle_, newSocket) == false)
			{
				pClientInfo->Close(true);
				return;
			}

			//char clientIP[32] = { 0, };
			//inet_ntop(AF_INET, &(stClientAddr.sin_addr), clientIP, 32 - 1);
			//printf("클라이언트 접속 : IP(%s) SOCKET(%d)\n", clientIP, (int)pClientInfo->m_socketClient);

			OnConnect(pClientInfo->GetIndex());

			//if (wellcomeSw == true)
			//{
			//	UPACKET packetHeader;
			//	ZeroMemory(&packetHeader, sizeof(UPACKET));
			//	// 패킷 길이 : 보낼 문자열 길이 + PACKET_HEADER_SIZE (4)
			//	packetHeader.ph.len = PACKET_HEADER_SIZE;
			//	// 패킷 타입
			//	packetHeader.ph.type = PACKET_CHATNAME_REQ;
			//	char* wellcomePacket = (char*)&packetHeader;

			//	SendMsg(pClientInfo, wellcomePacket, sizeof(wellcomePacket));
			//}

			// TODO 여기서 player 생성해야되는데?

			//클라이언트 갯수 증가
			++client_cnt_;
		}
	}

	//소켓의 연결을 종료 시킨다.
	void CloseSocket(stClientInfo* pClientInfo, bool bIsForce = false)
	{
		auto clientIndex = pClientInfo->GetIndex();

		pClientInfo->Close(bIsForce);

		OnClose(clientIndex);
	}

	void SendThread()
	{
		while (is_sender_run_)
		{
			for (auto client : client_Infos_)
			{
				// 클라이언트 접속이 끊어지면
				if (client->IsConnected() == false)
				{
					continue;
				}
				// 접속이 되어 있는 클라이언트 SendIo 처리
				client->SendIO();
			}

			// 할짓 거리 없으면 쳐자고 있고
			std::this_thread::sleep_for(std::chrono::milliseconds(8));
		}
	}

	//클라이언트 정보 저장 구조체
	std::vector<stClientInfo*> client_Infos_;

	//클라이언트의 접속을 받기위한 리슨 소켓
	SOCKET		listen_socket_ = INVALID_SOCKET;

	//접속 되어있는 클라이언트 수
	int			client_cnt_ = 0;

	//작업 쓰레드 동작 플래그
	bool		is_worker_run_ = true;
	//IO Worker 스레드
	std::vector<std::thread> io_worker_threads_;

	//접속 쓰레드 동작 플래그
	bool		is_accepter_run_ = true;
	//Accept 스레드
	std::thread	accepter_thread_;
	
	// 접속 쓰레드 동작 플래그 (sender)
	bool is_sender_run_ = false;
	//Accept 스레드 (sender)
	std::thread send_thread_;

	//CompletionPort객체 핸들
	HANDLE		iocp_handle_ = INVALID_HANDLE_VALUE;

	// 웰컴스위치
	bool wellcome_sw = false;

	
};