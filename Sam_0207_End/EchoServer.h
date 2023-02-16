#pragma once
#include "IOCompletionPort.h"
#include "Packet.h"

#include <vector>
#include <deque>
#include <thread>
#include <mutex>

class EchoServer : public IOCP_Net_Server
{
public:
	EchoServer() = default;
	virtual ~EchoServer() = default;

	virtual void OnConnect(const UINT32 clientIndex_) override
	{
		printf("[OnConnect] Ŭ���̾�Ʈ: Index(%d)\n", clientIndex_);

		//PacketData mPacket;
		//mPacket.SessionIndex = 0;
		//mPacket.DataSize = 4;
		//mPacket.pPacketData = (char*) 1001;

		//if (mPacket.DataSize != 0)
		//{
		//	SendMsg(mPacket.SessionIndex, mPacket.DataSize, mPacket.pPacketData);
		//}
	} 

	virtual void OnClose(const UINT32 clientIndex_) override
	{
		printf("[OnClose] Ŭ���̾�Ʈ: Index(%d)\n", clientIndex_);
	}

	virtual void OnReceive(const UINT32 clientIndex_, const UINT32 size_, char* pData_) override
	{
		printf("[OnReceive] Ŭ���̾�Ʈ: Index(%d), dataSize(%d)\n", clientIndex_, size_);
		maxCount++;

		// Ŭ���̾�Ʈ�� ���� �޼����� PacketData ����ü�� ����
		PacketData packet;
		packet.Set(clientIndex_, size_, pData_);
		
		std::lock_guard<std::mutex> guard(mLock);

		// PacketData�� packet ������ deque<PacketData> mPacketDataQueue �־���
		mPacketDataQueue.push_back(packet);
	}

	void Run(const UINT32 maxClient)
	{
		mIsRunProcessThread = true;
		// ���μ��� ��Ŷ ó�� ������ ����
		mProcessThread = std::thread([this]() { ProcessPacket(); });

		StartServer(maxClient);
	}

	void End()
	{
		mIsRunProcessThread = false;

		if (mProcessThread.joinable())
		{
			mProcessThread.join();
		}

		DestroyThread();
	}

private:
	void ProcessPacket()
	{
		while (mIsRunProcessThread)
		{
			// TODO ��Ŷ�� ������ �Ϳ� ���� �װ��� ó���ϱ�
			// ť�� �ִ� �����͸� �б�
			auto packetData = DequePacketData();
			// ť�� ������ ������ Send ������ ���ڱ�
			if (packetData.DataSize != 0)
			{
				SendMsg(packetData.SessionIndex, packetData.DataSize, packetData.pPacketData);
			}
			else
			{
				// ���� ������ ���ڰ� �־�
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}
	}

	PacketData DequePacketData()
	{
		PacketData packetData;

		std::lock_guard<std::mutex> guard(mLock);
		if (mPacketDataQueue.empty())
		{
			return PacketData();
		}

		// deque<PacketData> mPacketDataQueue ����ü�� PacketData packetData�� ��ȯ
		packetData.Set(mPacketDataQueue.front());

		// deque<PacketData> ù��° char* pPacketData �޸� ����
		mPacketDataQueue.front().Release();

		// deque<PacketData> ť���� �޸� ������ �� ������
		mPacketDataQueue.pop_front();

		return packetData;
	}

	UINT32 maxCount = 0;

	bool mIsRunProcessThread = false;

	std::thread mProcessThread;

	std::mutex mLock;
	std::deque<PacketData> mPacketDataQueue;
};
