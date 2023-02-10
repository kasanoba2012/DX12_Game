#pragma once

#include "Define.h"
#include <stdio.h>
#include <mutex>
#include <queue>

class stClientInfo
{
public:
	stClientInfo()
	{
		ZeroMemory(&recv_overlapped_ex_, sizeof(stOverlappedEx));
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

	bool SendMsg(const UINT32 data_size_, char* P_msg_)
	{
		// send_overlapped 구조체 생성
		auto send_overlapped_ex = new stOverlappedEx;
		ZeroMemory(send_overlapped_ex, sizeof(stOverlappedEx));
		send_overlapped_ex->wsa_buf_.len = data_size_;
		// Send 보낼 데이터 사이즈만큼 buf 동적 할당
		send_overlapped_ex->wsa_buf_.buf = new char[data_size_];
		CopyMemory(send_overlapped_ex->wsa_buf_.buf, P_msg_, data_size_);
		// send_overlapped 형태 알려주기 
		send_overlapped_ex->E_operation_ = IOOperation::SEND;

		std::lock_guard<std::mutex> guard(send_lock_);

		// send queue에 데이터 밀어 넣기
		send_data_queue_.push(send_overlapped_ex);

		// 현재 큐에 한개 밖에 없으면 실행해도 되고 2개 이상이면 현재 진행중인 WSASend가 있다
		if (send_data_queue_.size() == 1)
		{
			SendIO();
		}

		return true;
	}


	void SendCompleted(const UINT32 dataSize_)
	{
		printf("[송신 완료] bytes : %d\n", dataSize_);

		std::lock_guard<std::mutex> guard(send_lock_);

		// 큐의 제일 앞 버퍼 배열 삭제
		delete[] send_data_queue_.front()->wsa_buf_.buf;
		// 큐 제일 삭제
		delete send_data_queue_.front();

		// 큐 제일 앞 빼버리기
		send_data_queue_.pop();

		// 큐가 비어 있는 상태가 아니면 그 다음부터 다시 wsasend
		if (send_data_queue_.empty() == false)
		{
			SendIO();
		}
	}

private:
	bool SendIO()
	{
		// 큐의 제일 앞부분 stOverlappedEx로 객체에 담기
		auto send_overlapped_ex = send_data_queue_.front();

		DWORD dwRecvNumBytes = 0;
		int nRet = WSASend(sock_,
			&(send_overlapped_ex->wsa_buf_),
			1,
			&dwRecvNumBytes,
			0,
			(LPWSAOVERLAPPED)send_overlapped_ex,
			NULL);

		//socket_error이면 client socket이 끊어진걸로 처리한다.
		if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
		{
			printf("[에러] WSASend()함수 실패 : %d\n", WSAGetLastError());
			return false;
		}
	}
	INT32 index_ = 0;
	SOCKET sock_; //Cliet와
	stOverlappedEx recv_overlapped_ex_; //RECV Overlapped I/O작업을 위한 변수

	char recv_buf_[MAX_SOCKBUF];  //데이터 버퍼

	// 1 send (queue 형태)
	std::mutex send_lock_;
	std::queue<stOverlappedEx*> send_data_queue_;
};