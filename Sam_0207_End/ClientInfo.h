#pragma once
#include "Define.h"
#include <stdio.h>

//Ŭ���̾�Ʈ ������ ������� ����ü
class stClientInfo
{
public:
	stClientInfo()
	{
		ZeroMemory(&mRecvOverlappedEx, sizeof(stOverlappedEx));
		mSock = INVALID_SOCKET;
	}

	void Init(const UINT32 index)
	{
		mIndex = index;
	}

	UINT32 GetIndex() { return mIndex; }

	bool IsConnectd() { return mSock != INVALID_SOCKET; }

	SOCKET GetSock() { return mSock; }

	char* RecvBuffer() { return mRecvBuf; }

	bool OnConnect(HANDLE iocpHandle_, SOCKET socket_)
	{
		mSock = socket_;

		// ���� �̱���
		Clear();

		// I/O Completion Port ��ü�� ������ �����Ų��.
		if (SetBindIocpSocket(iocpHandle_) == false)
		{
			return false;
		}

		// Recv ���� ���� �ɱ�
		return BindRecv();
	}

	void Close(bool bIsForce = false)
	{
		// linger : https://learn.microsoft.com/en-us/windows/win32/api/winsock/ns-winsock-linger
		struct linger stLinger = { 0,0 }; // SO_DONTLINGER�� ����
		
		if (true == bIsForce)
		{
			stLinger.l_onoff = 1;
		}

		//socketClose������ ������ �ۼ����� ��� �ߴ� ��Ų��.
		shutdown(mSock, SD_BOTH);

		//���� �ɼ��� �����Ѵ�.
		setsockopt(mSock, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));

		//���� ������ ���� ��Ų��. 
		closesocket(mSock);
		mSock = INVALID_SOCKET;
	}

	void Clear()
	{
	}

	//CompletionPort��ü�� ���ϰ� CompletionKey�� �����Ű�� ������ �Ѵ�.
	bool SetBindIocpSocket(HANDLE iocpHandle_)
	{
		//socket�� pClientInfo�� CompletionPort��ü�� �����Ų��.
		auto hIOCP = CreateIoCompletionPort((HANDLE)GetSock()
			, iocpHandle_
			, (ULONG_PTR)(this), 0);

		if (hIOCP == INVALID_HANDLE_VALUE)
		{
			printf("[����] CreateIoCompletionPort()�Լ� ����: %d\n", GetLastError());
			return false;
		}

		return true;
	}

	bool BindRecv()
	{
		DWORD dwFlag = 0;
		DWORD dwRecvNumBytes = 0;

		//Overlapped I/O�� ���� �� ������ ������ �ش�.
		mRecvOverlappedEx.m_wsaBuf.len = MAX_SOCKBUF;
		mRecvOverlappedEx.m_wsaBuf.buf = mRecvBuf;
		mRecvOverlappedEx.m_eOperation = IOOperation::RECV;

		// RECV ���� ����
		int nRet = WSARecv(mSock,
			&(mRecvOverlappedEx.m_wsaBuf),
			1,
			&dwRecvNumBytes,
			&dwFlag,
			(LPWSAOVERLAPPED) & (mRecvOverlappedEx),
			NULL);

		//socket_error�̸� client socket�� �������ɷ� ó���Ѵ�.
		if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
		{
			printf("[����] WSARecv()�Լ� ���� : %d\n", WSAGetLastError());
			return false;
		}

		return true;
	}

	// 1���� �����忡���� ȣ���ؾ� �Ѵ�!
	bool SendMsg(const UINT32 dataSize_, char* pMsg_)
	{
		auto sendOverlappedEx = new stOverlappedEx;
		ZeroMemory(sendOverlappedEx, sizeof(stOverlappedEx));
		sendOverlappedEx->m_wsaBuf.len = dataSize_;
		sendOverlappedEx->m_wsaBuf.buf = new char[dataSize_];
		CopyMemory(sendOverlappedEx->m_wsaBuf.buf, pMsg_, dataSize_);
		sendOverlappedEx->m_eOperation = IOOperation::SEND;

		DWORD dwRecvNumBytes = 0;
		int nRet = WSASend(mSock,
			&(sendOverlappedEx->m_wsaBuf),
			1,
			&dwRecvNumBytes,
			0,
			(LPWSAOVERLAPPED)sendOverlappedEx,
			NULL);

		//socket_error�̸� client socket�� �������ɷ� ó���Ѵ�.
		if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
		{
			printf("[����] WSASend()�Լ� ���� : %d\n", WSAGetLastError());
			return false;
		}

		return true;
	}
	
	void SendCompleted(const UINT32 dataSize_)
	{
		printf("[�۽� �Ϸ�] bytes : %d\n", dataSize_);
	}

	bool WellcomeSend(short type)
	{
		UPACKET packet;
		ZeroMemory(&packet, sizeof(UPACKET));

		packet.ph.len = 4;
		packet.ph.type = type;
		// ���� ����
		auto sendOverlappedEx = new stOverlappedEx;
		ZeroMemory(sendOverlappedEx, sizeof(stOverlappedEx));
		//sendOverlappedEx->ph.type = 1001;
		sendOverlappedEx->m_wsaBuf.len = 4;
		sendOverlappedEx->m_wsaBuf.buf = (char*)&packet;
		//CopyMemory(sendOverlappedEx->m_wsaBuf.buf, pMsg_, 4);
		sendOverlappedEx->m_eOperation = IOOperation::SEND;

		DWORD dwRecvNumBytes = 0;
		int nRet = WSASend(mSock,
			&(sendOverlappedEx->m_wsaBuf),
			1,
			&dwRecvNumBytes,
			0,
			(LPWSAOVERLAPPED)sendOverlappedEx,
			NULL);

		//socket_error�̸� client socket�� �������ɷ� ó���Ѵ�.
		if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
		{
			printf("[����] WSASend()�Լ� ���� : %d\n", WSAGetLastError());
			return false;
		}

		return true;
	}

public:
	INT32 mIndex = 0;
	SOCKET mSock; // Client�� ����Ǵ� ����
	stOverlappedEx mRecvOverlappedEx; // RECV Overlapped I/O �۾��� ���� ����
	stOverlappedEx	m_stSendOverlappedEx;	//SEND Overlapped I/O�۾��� ���� ����

	char mRecvBuf[MAX_SOCKBUF]; //������ ����
};

#define PACKET_CHAR_MSG   1000      // client ->
#define PACKET_CHATNAME_REQ   1001  // server -> client
#define PACKET_NAME_REQ   2000		// client -> server
#define PACKET_NAME_ACK   3000		// server -> client
#define PACKET_JOIN_USER  4000		// server -> client