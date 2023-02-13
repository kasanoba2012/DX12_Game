#pragma once

#include "Define.h"
#include <stdio.h>
#include <mutex>
#include <queue>

//Ŭ���̾�Ʈ ������ ������� ����ü
class stClientInfo
{
public:
	stClientInfo()
	{
		ZeroMemory(&recv_overlapped_ex_, sizeof(stOverlappedEx));
		socket_ = INVALID_SOCKET;
	}

	void Init(const UINT32 index, HANDLE iocp_handle)
	{
		index_ = index;
		iocp_handle_ = iocp_handle;
	}

	UINT32 GetIndex() { return index_; }

	bool IsConnected() { return is_connect_ == 1; }

	SOCKET GetSock() { return socket_; }

	UINT64 GetLatestClosedTimeSec() { return latest_closed_time_sec_; }

	char* RecvBuffer() { return recv_buf_; }

	bool OnConnect(HANDLE iocpHandle, SOCKET socket)
	{
		socket_ = socket;
		is_connect_ = 1;

		Clear();

		//I/O Completion Port��ü�� ������ �����Ų��.
		if (BindIOCompletionPort(iocpHandle) == false)
		{
			return false;
		}

		return BindRecv();
	}

	void Close(bool bIsForce = false)
	{
		struct linger stLinger = { 0,0 }; // SO_DONTLINGER�� ����

		// bIsForce�� true�̸� SO_LINGER, timeout = 0���� �����Ͽ� ���� ���� ��Ų��. ���� : ������ �ս��� ������ ���� 
		if (true == bIsForce)
		{
			stLinger.l_onoff = 1;
		}

		//socketClose������ ������ �ۼ����� ��� �ߴ� ��Ų��.
		shutdown(socket_, SD_BOTH);

		//���� �ɼ��� �����Ѵ�.
		setsockopt(socket_, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));

		is_connect_ = 0;
		latest_closed_time_sec_ = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

		//���� ������ ���� ��Ų��. 
		closesocket(socket_);
		socket_ = INVALID_SOCKET;
	}

	void Clear()
	{
	}

	// �񵿱� accept ���� ����
	bool PostAccept(SOCKET listen_socket, const UINT64 cur_time_sec)
	{
		printf_s("PostAccept. client Index: %d\n", GetIndex());

		latest_closed_time_sec_ = UINT32_MAX;

		// WSASocket : �񵿱� ����
		socket_ = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED);
		if (INVALID_SOCKET == socket_)
		{
			printf_s("client Socket WSASocket Error : %d\n", GetLastError());
			return false;
		}

		ZeroMemory(&accept_context_, sizeof(stOverlappedEx));

		DWORD bytes = 0;
		DWORD flags = 0;
		accept_context_.wsa_buf_.len = 0;
		accept_context_.wsa_buf_.buf = nullptr;
		accept_context_.E_operation_ = IOOperation::ACCEPT;
		accept_context_.session_index_ = index_;

		// AcceptEx : �񵿱� accept
		if (FALSE == AcceptEx(listen_socket, socket_, accept_buf_, 0, sizeof(SOCKADDR_IN) + 16,
			sizeof(SOCKADDR_IN) + 16, &bytes, (LPWSAOVERLAPPED) & (accept_context_)))
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				printf_s("AcceptEx Error : %d\n", GetLastError());
				return false;
			}
		}

		return true;
	}


	// accept �Ϸ� �Լ�
	bool AcceptCompletion()
	{
		printf_s("AcceptCompletion : SessionIndex(%d)\n", index_);

		if (OnConnect(iocp_handle_, socket_) == false)
		{
			return false;
		}

		SOCKADDR_IN st_client_addr;
		int addr_len = sizeof(SOCKADDR_IN);
		char client_ip[32] = { 0, };
		inet_ntop(AF_INET, &(st_client_addr.sin_addr), client_ip, 32 - 1);
		printf("Ŭ���̾�Ʈ ���� : IP(%s) SOCKET(%d)\n", client_ip, (int)socket_);

		return true;
	}

	bool BindIOCompletionPort(HANDLE iocpHandle)
	{
		//socket�� pClientInfo�� CompletionPort��ü�� �����Ų��.
		auto H_iocp = CreateIoCompletionPort((HANDLE)GetSock(),
			iocpHandle,
			(ULONG_PTR)(this),
			0);

		if (H_iocp == INVALID_HANDLE_VALUE)
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

		int nRet = WSARecv(socket_, 
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
		int nRet = WSASend(socket_,
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

	bool SetSocketOption()
	{
		int opt = 1;
		if (SOCKET_ERROR == setsockopt(socket_, IPPROTO_TCP, TCP_NODELAY, (const char*)&opt,
			sizeof(int)))
		{
			printf_s("[DEBUG] TCP_NODELAY error: %d\n", GetLastError());
			return false;
		}

		opt = 0;
		if (SOCKET_ERROR == setsockopt(socket_, SOL_SOCKET, SO_RCVBUF, (const char*)&opt, sizeof(int)))
		{
			printf_s("[DEBUG] SO_RCVBUF change error: %d\n", GetLastError());
			return false;
		}

		return true;
	}

	INT32 index_ = 0;
	HANDLE iocp_handle_ = INVALID_HANDLE_VALUE;

	INT64 is_connect_ = 0;
	UINT64 latest_closed_time_sec_ = 0;

	SOCKET socket_; //Cliet��

	// Accept ������Ʈ ����ü wsaaccpet ���� ��� �Ǵ� ����
	stOverlappedEx accept_context_;
	char accept_buf_[64];

	stOverlappedEx recv_overlapped_ex_; //RECV Overlapped I/O�۾��� ���� ����
	char recv_buf_[MAX_SOCKBUF];  //������ ����

	// 1 send (queue ����)
	std::mutex send_lock_;
	std::queue<stOverlappedEx*> send_data_queue_;
};