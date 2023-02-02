//출처: 강정중님의 저서 '온라인 게임서버'에서
#pragma once
#pragma comment(lib, "ws2_32")

#include "ClientInfo.h"
#include "Define.h"
#include <thread>
#include <vector>

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
		WSADATA wsaData;

		// WSAStartup : 성공시 0을 리턴 즉 0 아니면 윈속 초기화 실패
		int nRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (0 != nRet)
		{
			printf("[에러] WSAStartup()함수 실패 : %d\n", WSAGetLastError());
			return false;
		}

		//연결지향형 TCP , Overlapped I/O 소켓을 생성
		mListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);

		if (INVALID_SOCKET == mListenSocket)
		{
			printf("[에러] socket()함수 실패 : %d\n", WSAGetLastError());
			return false;
		}

		printf("소켓 초기화 성공\n");
		return true;
	} // End InitSocket()


	//------서버용 함수-------//
	//서버의 주소정보를 소켓과 연결시키고 접속 요청을 받기 위해 
	//소켓을 등록하는 함수
	bool BindandListen(int nBindPort)
	{
		SOCKADDR_IN		stServerAddr;
		stServerAddr.sin_family = AF_INET;
		stServerAddr.sin_port = htons(nBindPort); //서버 포트를 설정한다.		
		//어떤 주소에서 들어오는 접속이라도 받아들이겠다.
		//보통 서버라면 이렇게 설정한다. 만약 한 아이피에서만 접속을 받고 싶다면
		//그 주소를 inet_addr함수를 이용해 넣으면 된다.
		stServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

		//위에서 지정한 서버 주소 정보와 cIOCompletionPort 소켓을 연결한다.
		int nRet = bind(mListenSocket, (SOCKADDR*)&stServerAddr, sizeof(SOCKADDR_IN));
		if (0 != nRet)
		{
			printf("[에러] bind()함수 실패 : %d\n", WSAGetLastError());
			return false;
		}

		//접속 요청을 받아들이기 위해 cIOCompletionPort소켓을 등록하고 
		//접속대기큐를 5개로 설정 한다.
		nRet = listen(mListenSocket, 0);
		if (0 != nRet)
		{
			printf("[에러] listen()함수 실패 : %d\n", WSAGetLastError());
			return false;
		}

		printf("서버 등록 성공..\n");
		return true;
	}

	//접속 요청을 수락하고 메세지를 받아서 처리하는 함수
	bool StartServer(const UINT32 maxClientCount)
	{
		CreateClient(maxClientCount);

		// CreateIoCompletionPort : IOCP 생성
		// https://www.slideshare.net/namhyeonuk90/iocp
		mIOCPHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, MAX_WORKERTHREAD);
		if (NULL == mIOCPHandle)
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

		printf("서버 시작\n");
		return true;
	}

	//생성되어있는 쓰레드를 파괴한다.
	void DestroyThread()
	{
		mIsWorkerRun = false;
		CloseHandle(mIOCPHandle);

		for (auto& th : mIOWorkerThreads)
		{
			// joinable() : 쓰레드 실행중 확인 여부
			if (th.joinable())
			{
				th.join();
			}
		}

		//Accepter 쓰레드를 종요한다.
		mIsAccepterRun = false;
		closesocket(mListenSocket);

		if (mAccepterThread.joinable())
		{
			mAccepterThread.join();
		}
	}

	// 1대1 채팅 (귓속말)
	bool SendMsg(const UINT32 sessionIndex_, const UINT32 dataSize_, char* pData)
	{
		// Send를 발신할 Index를 가져와서 해당 IndexSession에만 메세지 전송
		auto pClient = GetClientInfo(sessionIndex_);

		return pClient->SendMsg(dataSize_, pData);
	}

	// 네트워크 이벤트를 처리할 함수들
	virtual void OnConnect(const UINT32 clientIndex_) {}
	virtual void OnClose(const UINT32 clientIndex_) {}
	virtual void OnReceive(const UINT32 clientIndex_, const UINT32 size_, char* pData_) {}

private:
	// CreateClient : 허용 클라이언트 숫자만큼 빈 구조체 생성
	void CreateClient(const UINT32 maxClientCount)
	{
		for (UINT32 i = 0; i < maxClientCount; ++i)
		{
			mClientInfos.emplace_back();

			// 클라이언트 정보 저장 구조체에 별로 인덱스 값 세팅
			mClientInfos[i].Init(i);
		}
	} // end CreateClient()

	//WaitingThread Queue에서 대기할 쓰레드들을 생성
	bool CreateWokerThread()
	{
		unsigned int uiThreadId = 0;
		//WaingThread Queue에 대기 상태로 넣을 쓰레드들 생성 권장되는 개수 : (cpu개수 * 2) + 1 
		for (int i = 0; i < MAX_WORKERTHREAD; i++)
		{
			// mIOWorkerThreads에 WokerThread()담아서 실행 / WokerThread : (네트워크 IO 처리전용 쓰레드)
			mIOWorkerThreads.emplace_back([this]() { WokerThread(); });
		}

		printf("WokerThread 시작..\n");
		return true;
	} // end CreateWokerThread()

	//사용하지 않는 클라이언트 정보 구조체를 반환한다.
	stClientInfo* GetEmptyClientInfo()
	{
		for (auto& client : mClientInfos)
		{
			if (client.IsConnectd() == false)
			{
				return &client;
			}
		}

		return nullptr;
	} // end GetEmptyClientInfo()

	stClientInfo* GetClientInfo(const UINT32 sessionIndex)
	{
		return &mClientInfos[sessionIndex];
	}

	//accept요청을 처리하는 쓰레드 생성
	bool CreateAccepterThread()
	{
		mAccepterThread = std::thread([this]() { AccepterThread(); });

		printf("AccepterThread 시작..\n");
		return true;
	} // end CreateAccepterThread()

	//Overlapped I/O작업에 대한 완료 통보를 받아 
	//그에 해당하는 처리를 하는 함수
	void WokerThread()
	{
		//CompletionKey를 받을 포인터 변수
		stClientInfo* pClientInfo = NULL;
		//함수 호출 성공 여부
		BOOL bSuccess = TRUE;
		//Overlapped I/O작업에서 전송된 데이터 크기
		DWORD dwIoSize = 0;
		//I/O 작업을 위해 요청한 Overlapped 구조체를 받을 포인터
		LPOVERLAPPED lpOverlapped = NULL;

		while (mIsWorkerRun)
		{
			//////////////////////////////////////////////////////
			//이 함수로 인해 쓰레드들은 WaitingThread Queue에
			//대기 상태로 들어가게 된다.
			//완료된 Overlapped I/O작업이 발생하면 IOCP Queue에서
			//완료된 작업을 가져와 뒤 처리를 한다.
			//그리고 PostQueuedCompletionStatus()함수에의해 사용자
			//메세지가 도착되면 쓰레드를 종료한다.
			//////////////////////////////////////////////////////
			bSuccess = GetQueuedCompletionStatus(mIOCPHandle,
				&dwIoSize,					// 실제로 전송된 바이트
				(PULONG_PTR)&pClientInfo,		// CompletionKey
				&lpOverlapped,				// Overlapped IO 객체
				INFINITE);					// 대기할 시간

			//사용자 쓰레드 종료 메세지 처리..
			if (TRUE == bSuccess && 0 == dwIoSize && NULL == lpOverlapped)
			{
				mIsWorkerRun = false;
				continue;
			}

			if (NULL == lpOverlapped)
			{
				continue;
			}

			//client가 접속을 끊었을때..			
			if (FALSE == bSuccess || (0 == dwIoSize && TRUE == bSuccess))
			{
				//printf("socket(%d) 접속 끊김\n", (int)pClientInfo->m_socketClient);
				CloseSocket(pClientInfo);
				continue;
			}

			stOverlappedEx* pOverlappedEx = (stOverlappedEx*)lpOverlapped;

			//Overlapped I/O Recv작업 결과 뒤 처리
			if (IOOperation::RECV == pOverlappedEx->m_eOperation)
			{
				// Recv 받은 내용 처리
				auto RecvData = pClientInfo->RecvBuffer();

				OnReceive(pClientInfo->GetIndex(), dwIoSize, pClientInfo->RecvBuffer());

				// Recv 받은 내용 처리 이후 다시 Recv 예약 걸기
				pClientInfo->BindRecv();

				//// RECV Iocp 호출 시 OnReceive 함수 호출
				//OnReceive(pClientInfo->mIndex, dwIoSize, pClientInfo->mRecvBuf);

				//pClientInfo->mRecvBuf[dwIoSize] = '\0';
				//printf("[수신] bytes : %d , msg : %s\n", dwIoSize, pClientInfo->mRecvBuf[4]);

				////클라이언트에 메세지를 에코한다.
				//SendMsg(pClientInfo, pClientInfo->mRecvBuf, dwIoSize);

				//BindRecv(pClientInfo);
			}
			//Overlapped I/O Send작업 결과 뒤 처리
			else if (IOOperation::SEND == pOverlappedEx->m_eOperation)
			{
				delete[] pOverlappedEx->m_wsaBuf.buf;
				delete pOverlappedEx;
				pClientInfo->SendCompleted(dwIoSize);
				//printf("[송신] bytes : %d , msg : %s\n", dwIoSize, &pClientInfo->mSendBuf[4]);
			}
			//예외 상황
			else
			{
				printf("Client Index(%d)에서 예외상황\n", pClientInfo->GetIndex());
				//printf("socket(%d)에서 예외상황\n", (int)pClientInfo->m_socketClient);
			}
		}
	}

	//사용자의 접속을 받는 쓰레드
	void AccepterThread()
	{
		SOCKADDR_IN		stClientAddr;
		int nAddrLen = sizeof(SOCKADDR_IN);

		while (mIsAccepterRun)
		{
			//접속을 받을 구조체의 인덱스를 얻어온다.
			stClientInfo* pClientInfo = GetEmptyClientInfo();
			if (NULL == pClientInfo)
			{
				printf("[에러] Client Full\n");
				return;
			}

			//클라이언트 접속 요청이 들어올 때까지 기다린다.
			auto newSocket = accept(mListenSocket, (SOCKADDR*)&stClientAddr, &nAddrLen);
			if (INVALID_SOCKET == newSocket)
			{
				continue;
			}

			//I/O Completion Port객체와 소켓을 연결시킨다.
			/*bool bRet = BindIOCompletionPort(pClientInfo);
			if (false == bRet)
			{
				return;
			}*/

			if (pClientInfo->OnConnect(mIOCPHandle, newSocket) == false)
			{
				pClientInfo->Close(true);
				return;
			}

			//char clientIP[32] = { 0, };
			//inet_ntop(AF_INET, &(stClientAddr.sin_addr), clientIP, 32 - 1);
			//printf("클라이언트 접속 : IP(%s) SOCKET(%d)\n", clientIP, (int)pClientInfo->m_socketClient);

			// 클라이언트 Accepter 발생시 OnConnect 
			OnConnect(pClientInfo->GetIndex());

			//클라이언트 갯수 증가
			++mClientCnt;
		}
	}

	//소켓의 연결을 종료 시킨다.
	void CloseSocket(stClientInfo* pClientInfo, bool bIsForce = false)
	{
		auto clientIndex = pClientInfo->GetIndex();

		pClientInfo->Close(bIsForce);

		OnClose(clientIndex);
	}

	//클라이언트 정보 저장 구조체
	std::vector<stClientInfo> mClientInfos;

	//클라이언트의 접속을 받기위한 리슨 소켓
	SOCKET		mListenSocket = INVALID_SOCKET;

	//접속 되어있는 클라이언트 수
	int			mClientCnt = 0;

	//IO Worker 스레드
	std::vector<std::thread> mIOWorkerThreads;

	//Accept 스레드
	std::thread	mAccepterThread;

	//CompletionPort객체 핸들
	HANDLE		mIOCPHandle = INVALID_HANDLE_VALUE;

	//작업 쓰레드 동작 플래그
	bool		mIsWorkerRun = true;

	//접속 쓰레드 동작 플래그
	bool		mIsAccepterRun = true;
};