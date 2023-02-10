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
		//������ ����� ������.
		WSACleanup();
	}

	//������ �ʱ�ȭ�ϴ� �Լ�
	bool InitSocket()
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

		nRet = listen(listen_socket_, 5);
		if (0 != nRet)
		{
			printf("[����] listen()�Լ� ���� : %d\n", WSAGetLastError());
			return false;
		}

		printf("���� ��� ����..\n");
		return true;
	}

	bool StartServer(const UINT32 maxClientCount)
	{
		// Ŭ���̾�Ʈ ����ü �̸� �� ��������
		CreateClient(maxClientCount);

		//CompletionPort��ü ���� ��û�� �Ѵ�.
		iocp_handle_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, MAX_WORKERTHREAD);
		if (NULL == iocp_handle_)
		{
			printf("[����] CreateIoCompletionPort()�Լ� ����: %d\n", GetLastError());
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

		printf("���� ����\n");
		return true;
	}

	//�����Ǿ��ִ� �����带 �ı��Ѵ�.
	void DestroyThread()
	{

		// SendThread �ı�

		is_sender_run_ = false;

		if (send_thread_.joinable())
		{
			send_thread_.join();
		}

		// Accepter �����带 �����Ѵ�.
		is_accepter_run_ = false;
		closesocket(listen_socket_);

		if (accepter_thread_.joinable())
		{
			accepter_thread_.join();
		}

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
			// ���� ��� Ŭ���̾�Ʈ ��ŭ �� stClientInfo ����ü �Ҵ�
			auto clinet = new stClientInfo();
			clinet->Init(i);
			
			client_Infos_.push_back(clinet);
		}
	}

	//WaitingThread Queue���� ����� ��������� ����
	bool CreateWokerThread()
	{
		unsigned int uiThreadId = 0;
		//WaingThread Queue�� ��� ���·� ���� ������� ���� ����Ǵ� ���� : (cpu���� * 2) + 1 
		for (int i = 0; i < MAX_WORKERTHREAD; i++)
		{
			io_worker_threads_.emplace_back([this]() { WokerThread(); });
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

	void CreateSendThread()
	{
		is_sender_run_ = true;
		send_thread_ = std::thread([this]() {SendThread();  });
		printf("SendThread ����..\n");
	}

	//Overlapped I/O�۾��� ���� �Ϸ� �뺸�� �޾� 
	//�׿� �ش��ϴ� ó���� �ϴ� �Լ�
	void WokerThread()
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

			//client�� ������ ��������..			
			if (FALSE == bSuccess || (0 == dwIoSize && TRUE == bSuccess))
			{
				//printf("socket(%d) ���� ����\n", (int)pClientInfo->_socketClient);
				CloseSocket(P_client_info);
				continue;
			}

			auto pOverlappedEx = (stOverlappedEx*)lpOverlapped;

			//Overlapped I/O Recv�۾� ��� �� ó��
			if (IOOperation::RECV == pOverlappedEx->E_operation_)
			{
				// Ŭ���̾�Ʈ���� ���� �޼����� PacketData ���·� ��ȯ
				OnReceive(P_client_info->GetIndex(), dwIoSize, P_client_info->RecvBuffer());

				// Recv �Ϸ� �߱� ������ ���� Recv() ȣ��
				P_client_info->BindRecv();
				//Ŭ���̾�Ʈ�� �޼����� �����Ѵ�.

				//if (wellcomeSw == false)
				//	//Ŭ���̾�Ʈ�� �޼����� �����Ѵ�.
				//	SendMsg(pClientInfo, pClientInfo->mRecvBuf, dwIoSize);

				//while (wellcomeSw == true)
				//{
				//	//UPACKET returnPacket;
				//	//ZeroMemory(&returnPacket, sizeof(UPACKET));
				//	//// ��Ŷ ���� : ���� ���ڿ� ���� + PACKET_HEADER_SIZE (4)
				//	//returnPacket.ph.len = PACKET_HEADER_SIZE + 3;
				//	//// ��Ŷ Ÿ��
				//	//returnPacket.ph.type = PACKET_NAME_ACK;
				//	//char* returnPacket2 = (char*)&returnPacket;
				//	//SendMsg(pClientInfo, returnPacket2, sizeof(returnPacket2));

				//	wellcomeSw = false;
				//}


				//SendMsg(pClientInfo, pClientInfo->mRecvBuf, dwIoSize);
			}
			//Overlapped I/O Send�۾� ��� �� ó��
			else if (IOOperation::SEND == pOverlappedEx->E_operation_)
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
		SOCKADDR_IN		stClientAddr;
		int nAddrLen = sizeof(SOCKADDR_IN);

		while (is_accepter_run_)
		{
			//������ ���� ����ü�� �ε����� ���´�.
			stClientInfo* pClientInfo = GetEmptyClientInfo();
			if (NULL == pClientInfo)
			{
				printf("[����] Client Full\n");
				return;
			}

			//Ŭ���̾�Ʈ ���� ��û�� ���� ������ ��ٸ���.
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
			//printf("Ŭ���̾�Ʈ ���� : IP(%s) SOCKET(%d)\n", clientIP, (int)pClientInfo->m_socketClient);

			OnConnect(pClientInfo->GetIndex());

			//if (wellcomeSw == true)
			//{
			//	UPACKET packetHeader;
			//	ZeroMemory(&packetHeader, sizeof(UPACKET));
			//	// ��Ŷ ���� : ���� ���ڿ� ���� + PACKET_HEADER_SIZE (4)
			//	packetHeader.ph.len = PACKET_HEADER_SIZE;
			//	// ��Ŷ Ÿ��
			//	packetHeader.ph.type = PACKET_CHATNAME_REQ;
			//	char* wellcomePacket = (char*)&packetHeader;

			//	SendMsg(pClientInfo, wellcomePacket, sizeof(wellcomePacket));
			//}

			// TODO ���⼭ player �����ؾߵǴµ�?

			//Ŭ���̾�Ʈ ���� ����
			++client_cnt_;
		}
	}

	//������ ������ ���� ��Ų��.
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
				// Ŭ���̾�Ʈ ������ ��������
				if (client->IsConnected() == false)
				{
					continue;
				}
				// ������ �Ǿ� �ִ� Ŭ���̾�Ʈ SendIo ó��
				client->SendIO();
			}

			// ���� �Ÿ� ������ ���ڰ� �ְ�
			std::this_thread::sleep_for(std::chrono::milliseconds(8));
		}
	}

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
	
	// ���� ������ ���� �÷��� (sender)
	bool is_sender_run_ = false;
	//Accept ������ (sender)
	std::thread send_thread_;

	//CompletionPort��ü �ڵ�
	HANDLE		iocp_handle_ = INVALID_HANDLE_VALUE;

	// ���Ľ���ġ
	bool wellcome_sw = false;

	
};