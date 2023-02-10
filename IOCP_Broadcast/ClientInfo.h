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
		// ���� ������ 0���� �ʱ�ȭ
		send_pos_ = 0;
		is_sending_ = false;
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

	bool SendMsg(const UINT32 dataSize_, char* P_msg_)
	{
		std::lock_guard<std::mutex> guard(send_lock_);

		// ���۰� ������ ���� ������� ũ�� �Ǹ� 0���� �ؼ� �ٽ� ó������ ����
		if ((send_pos_ + dataSize_) > MAX_SOCKBUF)
		{
			send_pos_ = 0;
		}

		auto P_send_buf_ = &send_buf_[send_pos_];

		//���۵� �޼����� ����
		CopyMemory(P_send_buf_, P_msg_, dataSize_);
		send_pos_ += dataSize_;

		return true;
	}

	bool SendIO()
	{
		// send_pos_ <= 0 : ���� �����Ͱ� �ִ°�?
		// is_sending_ : Send �Ϸᰡ �ȵƴµ� �� ȣ�� ���� �� SendIO ó�� �ȵǰ� ����
		if (send_pos_ <= 0 || is_sending_)
		{
			return true;
		}

		std::lock_guard<std::mutex> guard(send_lock_);

		is_sending_ = true;

		CopyMemory(sending_buf_, &send_buf_[0], send_pos_);

		//Overlapped I/O�� ���� �� ������ ������ �ش�.
		send_overlapped_ex_.wsa_buf_.len = send_pos_;
		send_overlapped_ex_.wsa_buf_.buf = &sending_buf_[0];
		send_overlapped_ex_.E_operation_ = IOOperation::SEND;

		DWORD dwRecvNumBytes = 0;
		// WSASend() 2��° &(mSendOverlappedEx.m_wsaBuf ������ �۾� ���� ��ȿ�� ���¸� �����ؾ��Ѵ�.
		// ���ۿ� ���� �ɰ� WSASend�� �ϰ������� �ع�����.
		int nRet = WSASend(sock_,
			&(send_overlapped_ex_.wsa_buf_),
			1,
			&dwRecvNumBytes,
			0,
			(LPWSAOVERLAPPED) & (send_overlapped_ex_),
			NULL);

		//socket_error�̸� client socket�� �������ɷ� ó���Ѵ�.
		if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
		{
			printf("[����] WSASend()�Լ� ���� : %d\n", WSAGetLastError());
			return false;
		}

		// Send �ѹ��� �� �ϰ� send_pos 0���� �о �ٽ� ÷���� ����
		send_pos_ = 0;
		return true;
	}

	void SendCompleted(const UINT32 dataSize_)
	{
		// �ٸ� �����忡�� SendIO ó���� �ϰ� ���� ��� ����� ���� ���� sw
		is_sending_ = false;
		printf("[�۽� �Ϸ�] bytes : %d\n", dataSize_);
	}
private:
	INT32 index_ = 0;
	SOCKET sock_; //Cliet��
	stOverlappedEx recv_overlapped_ex_; //RECV Overlapped I/O�۾��� ���� ����
	stOverlappedEx send_overlapped_ex_; //SEND Overlapped I/O�۾��� ���� ����

	char recv_buf_[MAX_SOCKBUF];  //������ ����

	// 1 ���� �߰� ����
	std::mutex send_lock_;
	bool is_sending_ = false;
	UINT64 send_pos_ = 0;
	char send_buf_[MAX_SOCK_SENDBUF];
	char sending_buf_[MAX_SOCK_SENDBUF];

};