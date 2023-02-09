#pragma once

#include "Define.h"
#include <stdio.h>

class stClientInfo
{
public:
	stClientInfo()
	{
		ZeroMemory(&_RecvOverlappedEx, sizeof(stOverlappedEx));
		_Sock = INVALID_SOCKET;
	}

	void Init(const UINT32 index)
	{
		_Index = index;
	}

	UINT32 GetIndex() { return _Index; }

	bool IsConnected() { return _Sock != INVALID_SOCKET; }

	SOCKET GetSock() { return _Sock; }

	char* recvBuffer() { return _RecvBuf; }

	bool OnConnect(HANDLE iocpHandle_, SOCKET socket_)
	{
		_Sock = socket_;

		Clear();

		//I/O Completion Port��ü�� ������ �����Ų��.
		if (BindIOCompletionPort(iocpHandle_) == false)
		{
			return false;
		}

		return BindRecv();
	}

	void Close(bool bIsForce = false)
	{
		struct linger stLinger = { 0,0 }; // SO_DONTLINGER�� ����

		if (true == bIsForce)
		{
			stLinger.l_onoff = 1;
		}

		//socketClose������ ������ �ۼ����� ��� �ߴ� ��Ų��.
		shutdown(_Sock, SD_BOTH);

		//���� �ɼ��� �����Ѵ�.
		setsockopt(_Sock, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));

		//���� ������ ���� ��Ų��. 
		closesocket(_Sock);
		_Sock = INVALID_SOCKET;
	}

	void Clear()
	{
	}

	bool BindIOCompletionPort(HANDLE iocpHandle_)
	{
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

		_RecvOverlappedEx._wsaBuf.len = MAX_SOCKBUF;
		_RecvOverlappedEx._wsaBuf.buf = _RecvBuf;
		_RecvOverlappedEx._eOperation = IOOperation::RECV;

		int nRet = WSARecv(_Sock, &(_RecvOverlappedEx._wsaBuf), 1, &dwRecvNumBytes, &dwFlag, (LPWSAOVERLAPPED) & (_RecvOverlappedEx), NULL);

		// socket_error�̸� client socket�� �������ɷ� ó���Ѵ�.
		if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
		{
			printf("[����] WSARecv()�Լ� ���� : %d\n", WSAGetLastError());
			return false;
		}

		return true;
	}

	bool SendMsg(const UINT32 dataSize_, char* pMsg_)
	{
		// Send�� �Ҷ� stOverlappedEx ���� ����
		auto sendOverlappedEx = new stOverlappedEx;
		ZeroMemory(sendOverlappedEx, sizeof(stOverlappedEx));

		// ������Ʈ ����ü�� ������ ���� ����ü�� �����Ҵ����� ��� ����
		sendOverlappedEx->_wsaBuf.len = dataSize_;

		// dataSize_ ���̸�ŭ sendOverlappedEx->_wsaBuf.buf ���� ������ ���� �Ҵ�
		sendOverlappedEx->_wsaBuf.buf = new char[dataSize_];
		// SendMsg()�� ���� ������ ����ü�� �����ϱ�
		CopyMemory(sendOverlappedEx->_wsaBuf.buf, pMsg_, dataSize_);
		// ���� �Ϸ� �˷��ֱ�
		sendOverlappedEx->_eOperation = IOOperation::SEND;

		DWORD dwRecvNumbytes = 0;
		int nRet = WSASend(_Sock, &(sendOverlappedEx->_wsaBuf), 1, &dwRecvNumbytes, 0, (LPWSAOVERLAPPED)sendOverlappedEx, NULL);

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
private:
	INT32 _Index = 0;
	SOCKET _Sock; //Cliet��
	stOverlappedEx _RecvOverlappedEx; //RECV Overlapped I/O�۾��� ���� ����

	char _RecvBuf[MAX_SOCKBUF];  //������ ����
};