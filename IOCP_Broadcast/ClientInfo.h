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
		shutdown(sock_, SD_BOTH);

		//���� �ɼ��� �����Ѵ�.
		setsockopt(sock_, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));

		//���� ������ ���� ��Ų��. 
		closesocket(sock_);
		sock_ = INVALID_SOCKET;
	}

	void Clear()
	{
	}

	bool BindIOCompletionPort(HANDLE iocpHandle_)
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

		recv_overlapped_ex_.wsa_buf_.len = MAX_SOCKBUF;
		recv_overlapped_ex_.wsa_buf_.buf = recv_buf_;
		recv_overlapped_ex_.E_operation_ = IOOperation::RECV;

		int nRet = WSARecv(sock_, 
			&(recv_overlapped_ex_.wsa_buf_),
			1, &dwRecvNumBytes, &dwFlag, 
			(LPWSAOVERLAPPED) & (recv_overlapped_ex_),
			NULL);

		// socket_error�̸� client socket�� �������ɷ� ó���Ѵ�.
		if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
		{
			printf("[����] WSARecv()�Լ� ���� : %d\n", WSAGetLastError());
			return false;
		}

		return true;
	}

	bool SendMsg(const UINT32 data_size_, char* P_msg_)
	{
		// send_overlapped ����ü ����
		auto send_overlapped_ex = new stOverlappedEx;
		ZeroMemory(send_overlapped_ex, sizeof(stOverlappedEx));
		send_overlapped_ex->wsa_buf_.len = data_size_;
		// Send ���� ������ �����ŭ buf ���� �Ҵ�
		send_overlapped_ex->wsa_buf_.buf = new char[data_size_];
		CopyMemory(send_overlapped_ex->wsa_buf_.buf, P_msg_, data_size_);
		// send_overlapped ���� �˷��ֱ� 
		send_overlapped_ex->E_operation_ = IOOperation::SEND;

		std::lock_guard<std::mutex> guard(send_lock_);

		// send queue�� ������ �о� �ֱ�
		send_data_queue_.push(send_overlapped_ex);

		// ���� ť�� �Ѱ� �ۿ� ������ �����ص� �ǰ� 2�� �̻��̸� ���� �������� WSASend�� �ִ�
		if (send_data_queue_.size() == 1)
		{
			SendIO();
		}

		return true;
	}


	void SendCompleted(const UINT32 dataSize_)
	{
		printf("[�۽� �Ϸ�] bytes : %d\n", dataSize_);

		std::lock_guard<std::mutex> guard(send_lock_);

		// ť�� ���� �� ���� �迭 ����
		delete[] send_data_queue_.front()->wsa_buf_.buf;
		// ť ���� ����
		delete send_data_queue_.front();

		// ť ���� �� ��������
		send_data_queue_.pop();

		// ť�� ��� �ִ� ���°� �ƴϸ� �� �������� �ٽ� wsasend
		if (send_data_queue_.empty() == false)
		{
			SendIO();
		}
	}

private:
	bool SendIO()
	{
		// ť�� ���� �պκ� stOverlappedEx�� ��ü�� ���
		auto send_overlapped_ex = send_data_queue_.front();

		DWORD dwRecvNumBytes = 0;
		int nRet = WSASend(sock_,
			&(send_overlapped_ex->wsa_buf_),
			1,
			&dwRecvNumBytes,
			0,
			(LPWSAOVERLAPPED)send_overlapped_ex,
			NULL);

		//socket_error�̸� client socket�� �������ɷ� ó���Ѵ�.
		if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
		{
			printf("[����] WSASend()�Լ� ���� : %d\n", WSAGetLastError());
			return false;
		}
	}
	INT32 index_ = 0;
	SOCKET sock_; //Cliet��
	stOverlappedEx recv_overlapped_ex_; //RECV Overlapped I/O�۾��� ���� ����

	char recv_buf_[MAX_SOCKBUF];  //������ ����

	// 1 send (queue ����)
	std::mutex send_lock_;
	std::queue<stOverlappedEx*> send_data_queue_;
};