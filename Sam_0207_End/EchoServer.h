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
		printf("[OnConnect] 클라이언트: Index(%d)\n", clientIndex_);

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
		printf("[OnClose] 클라이언트: Index(%d)\n", clientIndex_);
	}

	virtual void OnReceive(const UINT32 clientIndex_, const UINT32 size_, char* pData_) override
	{
		printf("[OnReceive] 클라이언트: Index(%d), dataSize(%d)\n", clientIndex_, size_);
		maxCount++;

		// 클라이언트가 보낸 메세지를 PacketData 구조체에 담음
		PacketData packet;
		packet.Set(clientIndex_, size_, pData_);
		
		std::lock_guard<std::mutex> guard(mLock);

		// PacketData의 packet 내용을 deque<PacketData> mPacketDataQueue 넣어줌
		mPacketDataQueue.push_back(packet);
	}

	void Run(const UINT32 maxClient)
	{
		mIsRunProcessThread = true;
		// 프로세서 패킷 처리 쓰레드 생성
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
			// TODO 패킷를 보내는 것에 따라서 그것을 처리하기
			// 큐에 있는 데이터를 읽기
			auto packetData = DequePacketData();
			// 큐에 내용이 있으면 Send 없으면 잠자기
			if (packetData.DataSize != 0)
			{
				SendMsg(packetData.SessionIndex, packetData.DataSize, packetData.pPacketData);
			}
			else
			{
				// 할짓 없으면 쳐자고 있어
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

		// deque<PacketData> mPacketDataQueue 구조체를 PacketData packetData로 변환
		packetData.Set(mPacketDataQueue.front());

		// deque<PacketData> 첫번째 char* pPacketData 메모리 삭제
		mPacketDataQueue.front().Release();

		// deque<PacketData> 큐에서 메모리 해제한 것 날리기
		mPacketDataQueue.pop_front();

		return packetData;
	}

	UINT32 maxCount = 0;

	bool mIsRunProcessThread = false;

	std::thread mProcessThread;

	std::mutex mLock;
	std::deque<PacketData> mPacketDataQueue;
};
