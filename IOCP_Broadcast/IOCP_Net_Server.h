#pragma once
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "mswsock.lib")

#include "ClientInfo.h"
#include "Define.h"
#include <thread>
#include <vector>

class IOCP_Net_Server
{
public:
	IOCP_Net_Server(void) {}

	~IOCP_Net_Server(void)
	{
		//������ ����� ������.
		WSACleanup();
	}

	//������ �ʱ�ȭ�ϴ� �Լ�
	bool Init(const UINT32 max_io_worker_thread_count)
	{
		WSADATA wsa_data;

		int nRet = WSAStartup(MAKEWORD(2, 2), &wsa_data);
		if (0 != nRet)
		{
			printf("[����] WSAStartup()�Լ� ���� : %d\n", WSAGetLastError());
			return false;
		}

		//���������� TCP , Overlapped I/O ������ ����
		listen_socket_ = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);

		if (INVALID_SOCKET == listen_socket_)
		{
			printf("[����] socket()�Լ� ���� : %d\n", WSAGetLastError());
			return false;
		}

		max_io_worker_thread_count_ = max_io_worker_thread_count;

		printf("���� �ʱ�ȭ ����\n");
		return true;
	}

	//------������ �Լ�-------//
	//������ �ּ������� ���ϰ� �����Ű�� ���� ��û�� �ޱ� ���� 
	//������ ����ϴ� �Լ�

	bool BindandListen(int nBindPort)
	{
		SOCKADDR_IN		stServerAddr;
		stServerAddr.sin_family = AF_INET;
		stServerAddr.sin_port = htons(nBindPort); //���� ��Ʈ�� �����Ѵ�.				
		stServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

		//������ ������ ���� �ּ� ������ cIOCompletionPort ������ �����Ѵ�.
		int nRet = bind(listen_socket_, (SOCKADDR*)&stServerAddr, sizeof(SOCKADDR_IN));
		if (0 != nRet)
		{
			printf("[����] bind()�Լ� ���� : %d\n", WSAGetLastError());
			return false;
		}

		// Ŭ���̾�Ʈ Connect() �Լ� ���� ��û�� listen_socket_ ���� ��û�� listen ��⿭ �߰�
		// accept ȣ�� �ÿ��� listen ��⿭���� ���� ������ ���� ������ �����ͼ� Ŭ���̾�Ʈ�� ���
		nRet = listen(listen_socket_, 5);
		if (0 != nRet)
		{
			printf("[����] listen()�Լ� ���� : %d\n", WSAGetLastError());
			return false;
		}

		//CompletionPort��ü ���� ��û�� �Ѵ�.
		iocp_handle_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, max_io_worker_thread_count_);
		if (NULL == iocp_handle_)
		{
			printf("[����] CreateIoCompletionPort()�Լ� ����: %d\n", GetLastError());
			return false;
		}

		// 1. listen_socket_ = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
		// 2. listen_socket�� ���� �� IOCP ��ü (iocp_handle_)
		auto H_iocp_handle = CreateIoCompletionPort((HANDLE)listen_socket_, iocp_handle_, (UINT32)0, 0);
		if (nullptr == H_iocp_handle)
		{
			printf("[����] listen socket IOCP bind ���� : %d\n", WSAGetLastError());
			return false;
		}

		printf("���� ��� ����..\n");
		return true;
	}

	bool StartServer(const UINT32 maxClientCount)
	{
		// Ŭ���̾�Ʈ ����ü �̸� �� ��������
		CreateClient(maxClientCount);

		bool bRet = CreateWokerThread();
		if (false == bRet) {
			return false;
		}

		bRet = CreateAccepterThread();
		if (false == bRet) {
			return false;
		}

		printf("���� ����\n");
		return true;
	}

	//�����Ǿ��ִ� �����带 �ı��Ѵ�.
	void DestroyThread()
	{
		// WorkThread ����
		is_worker_run_ = false;
		CloseHandle(iocp_handle_);

		for (auto& th : io_worker_threads_)
		{
			if (th.joinable())
			{
				th.join();
			}
		}

		// Accepter �����带 �����Ѵ�.
		is_accepter_run_ = false;
		closesocket(listen_socket_);

		if (accepter_thread_.joinable())
		{
			accepter_thread_.join();
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
		// TODO ��� ������ ���� ù��° ���Ϳ� ���Ͽ� �߼� �̻����� ��ε� ĳ��Ʈ �ϸ� ��ü�߼�
		//for (auto& broadastList : _ClientInfos)
		//{
		//	SendMsg(&broadastList, pMsg, sizeof(pMsg));
		//}
		//	
		//return pMsg;
		return nullptr;
	}

	// ��Ʈ��ũ �̺�Ʈ�� ó���� �Լ���
	virtual void OnConnect(const UINT32 clientIndex_) {}
	virtual void OnClose(const UINT32 clientIndex_) {}
	virtual void OnReceive(const UINT32 clientIndex_, const UINT32 size_, char* pData_) {}

private:
	void CreateClient(const UINT32 maxClientCount)
	{
		for (UINT32 i = 0; i < maxClientCount; ++i)
		{
			// ���� ��� Ŭ���̾�Ʈ ��ŭ �� stClientInfo Ŭ���� �Ҵ�
			auto clinet = new stClientInfo();
			clinet->Init(i, iocp_handle_);
			
			client_Infos_.push_back(clinet);
		}
	}

	//WaitingThread Queue���� ����� ��������� ����
	bool CreateWokerThread()
	{
		//WaingThread Queue�� ��� ���·� ���� ������� ���� ����Ǵ� ���� : (cpu���� * 2) + 1 
		for (UINT32 i = 0; i < max_io_worker_thread_count_; i++)
		{
			io_worker_threads_.emplace_back([this]() { WorkerThread(); });
		}

		printf("WokerThread ����..\n");
		return true;
	}

	//������� �ʴ� Ŭ���̾�Ʈ ���� ����ü�� ��ȯ�Ѵ�.
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

	//accept��û�� ó���ϴ� ������ ����
	bool CreateAccepterThread()
	{
		accepter_thread_ = std::thread([this]() { AccepterThread(); });

		printf("AccepterThread ����..\n");
		return true;
	}

	//Overlapped I/O�۾��� ���� �Ϸ� �뺸�� �޾� 
	//�׿� �ش��ϴ� ó���� �ϴ� �Լ�
	void WorkerThread()
	{
		//CompletionKey�� ���� ������ ����
		stClientInfo* P_client_info = nullptr;
		//�Լ� ȣ�� ���� ����
		BOOL bSuccess = TRUE;
		//Overlapped I/O�۾����� ���۵� ������ ũ��
		DWORD dwIoSize = 0;
		//I/O �۾��� ���� ��û�� Overlapped ����ü�� ���� ������
		LPOVERLAPPED lpOverlapped = NULL;

		while (is_worker_run_)
		{
			bSuccess = GetQueuedCompletionStatus(iocp_handle_,
				&dwIoSize,					// ������ ���۵� ����Ʈ
				(PULONG_PTR)&P_client_info,		// CompletionKey
				&lpOverlapped,				// Overlapped IO ��ü
				INFINITE);					// ����� �ð�

			//����� ������ ���� �޼��� ó��..
			if (TRUE == bSuccess && 0 == dwIoSize && NULL == lpOverlapped)
			{
				is_worker_run_ = false;
				continue;
			}

			if (NULL == lpOverlapped)
			{
				continue;
			}

			auto P_overlapped_ex = (stOverlappedEx*)lpOverlapped;

			//client�� ������ ��������..			
			if (FALSE == bSuccess || (0 == dwIoSize && IOOperation::ACCEPT != P_overlapped_ex->E_operation_))
			{
				//printf("socket(%d) ���� ����\n", (int)pClientInfo->_socketClient);
				CloseSocket(P_client_info);
				continue;
			}

			//Overlapped I/O Accept�۾� ��� �� ó��
			if (IOOperation::ACCEPT == P_overlapped_ex->E_operation_)
			{
				P_client_info = GetClientInfo(P_overlapped_ex->session_index_);
				if (P_client_info->AcceptCompletion())
				{
					// Ŭ���̾�Ʈ ���� ����
					++client_cnt_;

					OnConnect(P_client_info->GetIndex());
				}
				else {
					CloseSocket(P_client_info, true);
				}
			}
			
			//Overlapped I/O Recv�۾� ��� �� ó��
			else if (IOOperation::RECV == P_overlapped_ex->E_operation_)
			{
				// Ŭ���̾�Ʈ���� ���� �޼����� PacketData ���·� ��ȯ
				OnReceive(P_client_info->GetIndex(), dwIoSize, P_client_info->RecvBuffer());

				// Recv �Ϸ� �߱� ������ ���� Recv() ȣ��
				P_client_info->BindRecv();
				//Ŭ���̾�Ʈ�� �޼����� �����Ѵ�.

				//SendMsg(pClientInfo, pClientInfo->mRecvBuf, dwIoSize);
			}
			//Overlapped I/O Send�۾� ��� �� ó��
			else if (IOOperation::SEND == P_overlapped_ex->E_operation_)
			{
				P_client_info->SendCompleted(dwIoSize);

				//printf("[�߽�] bytes : %d , msg : %d\n", dwIoSize, pClientInfo->mSendBuf[0]);
				//printf("[�߽�] bytes : %d , msg : %s\n", dwIoSize, pClientInfo->mSendBuf);
			}
			//���� ��Ȳ
			else
			{
				printf("socket(%d)���� ���ܻ�Ȳ\n", (int)P_client_info->GetIndex());
			}
		}
	}

	//������� ������ �޴� ������
	void AccepterThread()
	{
		while (is_accepter_run_)
		{
			// �ð� ó��
			auto cur_time_sec = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
			//������ ���� ����ü�� �ε����� ���´�.

			// ������ ���� ���� ��ü�� ������ client->PostAccept(listen_socket_, cur_time_sec);���� 
			// Acceptex ������ �ɾ�д�.
			for (auto client : client_Infos_)
			{
				if (client->IsConnected())
				{
					continue;
				}

				// ������ ������ Ŭ���� ��� �ٷ� �����ϸ� ����� ���ϼ��� �ְ� Wsasend�� �߻��Ҽ��� �ֱ� ������
				// ���� �ð� ��ٸ��� client->PostAccept(listen_socket_, cur_time_sec); AcceptEx �� ����
				if ((UINT64)cur_time_sec < client->GetLatestClosedTimeSec())
				{
					continue;
				}

				// ����� Ŭ���̾�Ʈ�� �������µ� ������ ���� �ð��� �ʰ� ���� ������ �ϴ� ����
				auto diff = cur_time_sec - client->GetLatestClosedTimeSec();
				if (diff <= RE_USE_SESSION_WAIT_TIMESEC)
				{
					continue;
				}

				// ������ �Ǿ� ���� �ʴ� Ŭ���̾�Ʈ�� ������ �񵿱� accept ȣ��
				client->PostAccept(listen_socket_, cur_time_sec);
			}
				std::this_thread::sleep_for(std::chrono::milliseconds(32));
		}
	}


	//������ ������ ���� ��Ų��.
	void CloseSocket(stClientInfo* pClientInfo, bool bIsForce = false)
	{
		if (pClientInfo->IsConnected() == false)
		{
			return;
		}

		auto clientIndex = pClientInfo->GetIndex();

		pClientInfo->Close(bIsForce);

		OnClose(clientIndex);
	}

	// workerThread ����
	UINT32 max_io_worker_thread_count_ = 0;

	//Ŭ���̾�Ʈ ���� ���� ����ü
	std::vector<stClientInfo*> client_Infos_;

	//Ŭ���̾�Ʈ�� ������ �ޱ����� ���� ����
	SOCKET		listen_socket_ = INVALID_SOCKET;

	//���� �Ǿ��ִ� Ŭ���̾�Ʈ ��
	int			client_cnt_ = 0;

	//�۾� ������ ���� �÷���
	bool		is_worker_run_ = true;
	//IO Worker ������
	std::vector<std::thread> io_worker_threads_;

	//���� ������ ���� �÷���
	bool		is_accepter_run_ = true;
	//Accept ������
	std::thread	accepter_thread_;

	//CompletionPort��ü �ڵ�
	HANDLE		iocp_handle_ = INVALID_HANDLE_VALUE;	
};