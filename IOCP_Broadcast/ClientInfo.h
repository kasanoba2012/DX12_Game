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

		//I/O Completion Port객체와 소켓을 연결시킨다.
		if (BindIOCompletionPort(iocpHandle_) == false)
		{
			return false;
		}

		return BindRecv();
	}

	void Close(bool bIsForce = false)
	{
		struct linger stLinger = { 0,0 }; // SO_DONTLINGER로 설정

		if (true == bIsForce)
		{
			stLinger.l_onoff = 1;
		}

		//socketClose소켓의 데이터 송수신을 모두 중단 시킨다.
		shutdown(_Sock, SD_BOTH);

		//소켓 옵션을 설정한다.
		setsockopt(_Sock, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));

		//소켓 연결을 종료 시킨다. 
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
			printf("[에러] CreateIoCompletionPort()함수 실패: %d\n", GetLastError());
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

		// socket_error이면 client socket이 끊어진걸로 처리한다.
		if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
		{
			printf("[에러] WSARecv()함수 실패 : %d\n", WSAGetLastError());
			return false;
		}

		return true;
	}

	bool SendMsg(const UINT32 dataSize_, char* pMsg_)
	{
		// Send를 할때 stOverlappedEx 새로 생성
		auto sendOverlappedEx = new stOverlappedEx;
		ZeroMemory(sendOverlappedEx, sizeof(stOverlappedEx));

		// 오버랩트 구조체에 샌드할 내용 구조체에 동적할당으로 담는 과정
		sendOverlappedEx->_wsaBuf.len = dataSize_;

		// dataSize_ 길이만큼 sendOverlappedEx->_wsaBuf.buf 버퍼 사이즈 동적 할당
		sendOverlappedEx->_wsaBuf.buf = new char[dataSize_];
		// SendMsg()로 보낼 데이터 구조체에 저장하기
		CopyMemory(sendOverlappedEx->_wsaBuf.buf, pMsg_, dataSize_);
		// 샌드 완료 알려주기
		sendOverlappedEx->_eOperation = IOOperation::SEND;

		DWORD dwRecvNumbytes = 0;
		int nRet = WSASend(_Sock, &(sendOverlappedEx->_wsaBuf), 1, &dwRecvNumbytes, 0, (LPWSAOVERLAPPED)sendOverlappedEx, NULL);

		//socket_error이면 client socket이 끊어진걸로 처리한다.
		if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
		{
			printf("[에러] WSASend()함수 실패 : %d\n", WSAGetLastError());
			return false;
		}

		return true;
	}

	void SendCompleted(const UINT32 dataSize_)
	{
		printf("[송신 완료] bytes : %d\n", dataSize_);
	}
private:
	INT32 _Index = 0;
	SOCKET _Sock; //Cliet와
	stOverlappedEx _RecvOverlappedEx; //RECV Overlapped I/O작업을 위한 변수

	char _RecvBuf[MAX_SOCKBUF];  //데이터 버퍼
};