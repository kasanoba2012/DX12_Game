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
		WSADATA wsaData;

		int nRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (0 != nRet)
		{
			printf("[����] WSAStartup()�Լ� ���� : %d\n", WSAGetLastError());
			return false;
		}

		//���������� TCP , Overlapped I/O ������ ����
		_ListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);

		if (INVALID_SOCKET == _ListenSocket)
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
		int nRet = bind(_ListenSocket, (SOCKADDR*)&stServerAddr, sizeof(SOCKADDR_IN));
		if (0 != nRet)
		{
			printf("[����] bind()�Լ� ���� : %d\n", WSAGetLastError());
			return false;
		}

		nRet = listen(_ListenSocket, 5);
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
		CreateClient(maxClientCount);

		//CompletionPort��ü ���� ��û�� �Ѵ�.
		_IOCPHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, MAX_WORKERTHREAD);
		if (NULL == _IOCPHandle)
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

		printf("���� ����\n");
		return true;
	}

	//�����Ǿ��ִ� �����带 �ı��Ѵ�.
	void DestroyThread()
	{
		_IsWorkerRun = false;
		CloseHandle(_IOCPHandle);

		for (auto& th : _IOWorkerThreads)
		{
			if (th.joinable())
			{
				th.join();
			}
		}

		//Accepter �����带 �����Ѵ�.
		_IsAccepterRun = false;
		closesocket(_ListenSocket);

		if (_AccepterThread.joinable())
		{
			_AccepterThread.join();
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
			_ClientInfos.emplace_back();

			_ClientInfos[i].Init(i);
		}
	}

	//WaitingThread Queue���� ����� ��������� ����
	bool CreateWokerThread()
	{
		unsigned int uiThreadId = 0;
		//WaingThread Queue�� ��� ���·� ���� ������� ���� ����Ǵ� ���� : (cpu���� * 2) + 1 
		for (int i = 0; i < MAX_WORKERTHREAD; i++)
		{
			_IOWorkerThreads.emplace_back([this]() { WokerThread(); });
		}

		printf("WokerThread ����..\n");
		return true;
	}

	//������� �ʴ� Ŭ���̾�Ʈ ���� ����ü�� ��ȯ�Ѵ�.
	stClientInfo* GetEmptyClientInfo()
	{
		for (auto& clinet : _ClientInfos)
		{
			if (clinet.IsConnected() == false)
			{
				return &clinet;
			}
		}
		return nullptr;
	}

	stClientInfo* GetClientInfo(const UINT32 clientIndex)
	{
		return &_ClientInfos[clientIndex];
	}

	//accept��û�� ó���ϴ� ������ ����
	bool CreateAccepterThread()
	{
		_AccepterThread = std::thread([this]() { AccepterThread(); });

		printf("AccepterThread ����..\n");
		return true;
	}

	//Overlapped I/O�۾��� ���� �Ϸ� �뺸�� �޾� 
	//�׿� �ش��ϴ� ó���� �ϴ� �Լ�
	void WokerThread()
	{
		stClientInfo* pClientInfo = nullptr;
		BOOL bSuccess = TRUE;
		DWORD dwIoSize = 0;
		LPOVERLAPPED lpOverlapped = NULL;

		while (_IsWorkerRun)
		{
			bSuccess = GetQueuedCompletionStatus(_IOCPHandle,
				&dwIoSize,					// ������ ���۵� ����Ʈ
				(PULONG_PTR)&pClientInfo,		// CompletionKey
				&lpOverlapped,				// Overlapped IO ��ü
				INFINITE);					// ����� �ð�

			//����� ������ ���� �޼��� ó��..
			if (TRUE == bSuccess && 0 == dwIoSize && NULL == lpOverlapped)
			{
				_IsWorkerRun = false;
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
				CloseSocket(pClientInfo);
				continue;
			}

			auto pOverlappedEx = (stOverlappedEx*)lpOverlapped;

			//Overlapped I/O Recv�۾� ��� �� ó��
			if (IOOperation::RECV == pOverlappedEx->_eOperation)
			{
				// Ŭ���̾�Ʈ���� ���� �޼����� PacketData ���·� ��ȯ
				OnReceive(pClientInfo->GetIndex(), dwIoSize, pClientInfo->recvBuffer());

				// Recv �Ϸ� �߱� ������ ���� Recv() ȣ��
				pClientInfo->BindRecv();
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
			else if (IOOperation::SEND == pOverlappedEx->_eOperation)
			{
				// Send �� ���� �Ҵ� ������ �޸� ����
				delete[] pOverlappedEx->_wsaBuf.buf;
				delete pOverlappedEx;
				pClientInfo->SendCompleted(dwIoSize);

				//printf("[�߽�] bytes : %d , msg : %d\n", dwIoSize, pClientInfo->mSendBuf[0]);
				//printf("[�߽�] bytes : %d , msg : %s\n", dwIoSize, pClientInfo->mSendBuf);
			}
			//���� ��Ȳ
			else
			{
				printf("socket(%d)���� ���ܻ�Ȳ\n", (int)pClientInfo->GetIndex());
			}
		}
	}

	//������� ������ �޴� ������
	void AccepterThread()
	{
		SOCKADDR_IN		stClientAddr;
		int nAddrLen = sizeof(SOCKADDR_IN);

		while (_IsAccepterRun)
		{
			//������ ���� ����ü�� �ε����� ���´�.
			stClientInfo* pClientInfo = GetEmptyClientInfo();
			if (NULL == pClientInfo)
			{
				printf("[����] Client Full\n");
				return;
			}

			//Ŭ���̾�Ʈ ���� ��û�� ���� ������ ��ٸ���.
			auto newSocket = accept(_ListenSocket, (SOCKADDR*)&stClientAddr, &nAddrLen);
			if (INVALID_SOCKET == newSocket)
			{
				continue;
			}

			if (pClientInfo->OnConnect(_IOCPHandle, newSocket) == false)
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
			++_ClientCnt;
		}
	}

	//������ ������ ���� ��Ų��.
	void CloseSocket(stClientInfo* pClientInfo, bool bIsForce = false)
	{
		auto clientIndex = pClientInfo->GetIndex();

		pClientInfo->Close(bIsForce);

		OnClose(clientIndex);
	}

	//Ŭ���̾�Ʈ ���� ���� ����ü
	std::vector<stClientInfo> _ClientInfos;

	//Ŭ���̾�Ʈ�� ������ �ޱ����� ���� ����
	SOCKET		_ListenSocket = INVALID_SOCKET;

	//���� �Ǿ��ִ� Ŭ���̾�Ʈ ��
	int			_ClientCnt = 0;

	//IO Worker ������
	std::vector<std::thread> _IOWorkerThreads;

	//Accept ������
	std::thread	_AccepterThread;

	//CompletionPort��ü �ڵ�
	HANDLE		_IOCPHandle = INVALID_HANDLE_VALUE;

	//�۾� ������ ���� �÷���
	bool		_IsWorkerRun = true;

	//���� ������ ���� �÷���
	bool		_IsAccepterRun = true;
	//���� ����
	char		_SocketBuf[1024] = { 0, };

	// ���Ľ���ġ
	bool wellcomeSw = false;
};