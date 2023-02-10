#pragma once

#include "Define.h"
#include <stdio.h>
#include <mutex>

class stClientInfo
{
public:
	stClientInfo()
	{
		ZeroMemory(&recv_overlapped_ex_, sizeof(stOverlappedEx));
		ZeroMemory(&send_overlapped_ex_, sizeof(stOverlappedEx));
		sock_ = INVALID_SOCKET;
	}

	~stClientInfo() = default;

	void Init(const UINT32 index)
	{
		index_ = index;
	}

	UINT32 GetIndex() { return index_; }

	bool IsConnected() { return sock_ != INVALID_SOCKET; }

	SOCKET GetSock() { return sock_; }

	char* RecvBuffer() { return recv_buf_; }

	bool OnConnect(HANDLE iocpHandle_, SOCKET socket_)
	{
		sock_ = socket_;

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
		shutdown(sock_, SD_BOTH);

		//소켓 옵션을 설정한다.
		setsockopt(sock_, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));

		//소켓 연결을 종료 시킨다. 
		closesocket(sock_);
		sock_ = INVALID_SOCKET;
	}

	void Clear()
	{
		// 버퍼 시작점 0으로 초기화
		send_pos_ = 0;
		is_sending_ = false;
	}

	bool BindIOCompletionPort(HANDLE iocpHandle_)
	{
		//socket과 pClientInfo를 CompletionPort객체와 연결시킨다.
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

		recv_overlapped_ex_.wsa_buf_.len = MAX_SOCKBUF;
		recv_overlapped_ex_.wsa_buf_.buf = recv_buf_;
		recv_overlapped_ex_.E_operation_ = IOOperation::RECV;

		int nRet = WSARecv(sock_, 
			&(recv_overlapped_ex_.wsa_buf_),
			1, &dwRecvNumBytes, &dwFlag, 
			(LPWSAOVERLAPPED) & (recv_overlapped_ex_),
			NULL);

		// socket_error이면 client socket이 끊어진걸로 처리한다.
		if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
		{
			printf("[에러] WSARecv()함수 실패 : %d\n", WSAGetLastError());
			return false;
		}

		return true;
	}

	bool SendMsg(const UINT32 dataSize_, char* P_msg_)
	{
		std::lock_guard<std::mutex> guard(send_lock_);

		// 버퍼가 정해진 버퍼 사이즈보다 크게 되면 0으로 해서 다시 처음부터 쓴다
		if ((send_pos_ + dataSize_) > MAX_SOCKBUF)
		{
			send_pos_ = 0;
		}

		auto P_send_buf_ = &send_buf_[send_pos_];

		//전송될 메세지를 복사
		CopyMemory(P_send_buf_, P_msg_, dataSize_);
		send_pos_ += dataSize_;

		return true;
	}

	bool SendIO()
	{
		// send_pos_ <= 0 : 보낼 데이터가 있는가?
		// is_sending_ : Send 완료가 안됐는데 또 호출 했을 때 SendIO 처리 안되게 막음
		if (send_pos_ <= 0 || is_sending_)
		{
			return true;
		}

		std::lock_guard<std::mutex> guard(send_lock_);

		is_sending_ = true;

		CopyMemory(sending_buf_, &send_buf_[0], send_pos_);

		//Overlapped I/O을 위해 각 정보를 셋팅해 준다.
		send_overlapped_ex_.wsa_buf_.len = send_pos_;
		send_overlapped_ex_.wsa_buf_.buf = &sending_buf_[0];
		send_overlapped_ex_.E_operation_ = IOOperation::SEND;

		DWORD dwRecvNumBytes = 0;
		// WSASend() 2번째 &(mSendOverlappedEx.m_wsaBuf 보내기 작업 동안 유효한 상태를 유지해야한다.
		// 버퍼에 락을 걸고 WSASend를 일괄적으로 해버린다.
		int nRet = WSASend(sock_,
			&(send_overlapped_ex_.wsa_buf_),
			1,
			&dwRecvNumBytes,
			0,
			(LPWSAOVERLAPPED) & (send_overlapped_ex_),
			NULL);

		//socket_error이면 client socket이 끊어진걸로 처리한다.
		if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
		{
			printf("[에러] WSASend()함수 실패 : %d\n", WSAGetLastError());
			return false;
		}

		// Send 한번에 싹 하고 send_pos 0으로 밀어서 다시 첨부터 쓰기
		send_pos_ = 0;
		return true;
	}

	void SendCompleted(const UINT32 dataSize_)
	{
		// 다른 쓰레드에서 SendIO 처리를 하고 있을 경우 못들어 가게 막는 sw
		is_sending_ = false;
		printf("[송신 완료] bytes : %d\n", dataSize_);
	}
private:
	INT32 index_ = 0;
	SOCKET sock_; //Cliet와
	stOverlappedEx recv_overlapped_ex_; //RECV Overlapped I/O작업을 위한 변수
	stOverlappedEx send_overlapped_ex_; //SEND Overlapped I/O작업을 위한 변수

	char recv_buf_[MAX_SOCKBUF];  //데이터 버퍼

	// 1 버퍼 추가 변수
	std::mutex send_lock_;
	bool is_sending_ = false;
	UINT64 send_pos_ = 0;
	char send_buf_[MAX_SOCK_SENDBUF];
	char sending_buf_[MAX_SOCK_SENDBUF];

};