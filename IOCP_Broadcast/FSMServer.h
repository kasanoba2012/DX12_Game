#pragma once
#include "IOCompletionPort.h"
#include "Npc.h"
#include "Player.h"
#include "Packet.h"

#include <vector>
#include <deque>
#include <thread>
#include <mutex>

class FSMServer : public IOCompletionPort
{

public:
	// FSM 세팅
	FSM fsm;
	Npc npc;

	bool MovementSw = true;
public:
	FSMServer();
	virtual ~FSMServer() = default;

	void NpcMovement(Npc* npc);
	void ThreadTestfuntion();

	virtual void OnConnect(const UINT32 client_index_) override
	{
		// 접속 되면 플레이어 생성
		Player player;

		// TODO 접속하면 npc 따라가기 시작 근데 미리 좌표는 움직이고 있어야한다.
		while (0)
		{
			printf("Connect npc 좌표 : %d\n", (int)npc.m_NpcPos[0]);

			MovementSw = false;

			char npcPosMsg[256] = { 0, };
			*npcPosMsg = npc.m_NpcPos[0];
			npcPosMsg[1] = '\0';
			mainSendMsg(npcPosMsg);

			npc.Process(&player);
			Sleep(3000);
		}

		printf("[OnConnect] 클라이언트: Index(%d)\n", client_index_);
	}

	virtual void OnClose(const UINT32 client_index_) override
	{
		printf("[OnClose] 클라이언트: Index(%d)\n", client_index_);
	}

	virtual void OnReceive(const UINT32 client_index_, const UINT32 size_, char* P_data_) override
	{
		printf("[OnReceive] 클라이언트: Index(%d), dataSize(%d), data : %s\n", client_index_, size_, P_data_);

		// 클라이언트에서 받은 Recv 데이터를 큐에 넣어주기 위해서 패킷형태로 변환한다.
		// PacketData : 인덱스, 데이터 사이즈, 데이터 내용 시작 주소 받는 구조체
		PacketData packet;
		packet.Set(client_index_, size_, P_data_);

		// lock_guard : 이 scope 벗어나면 lock이 해제 됩니다.
		std::lock_guard<std::mutex> guard(lock_);
		packet_data_queue_.push_back(packet);
	}

	void Run(const UINT32 maxClient)
	{
		is_run_process_thread_ = TRUE;
		// ProcessPacket() : 패킷 처리 쓰레드
		// 클라이언트에서 Send가 날라오면 Send 내용을 PacketData 타입의 큐에 저장을 시키는 쓰레드 이며, Send에서 내용을 받아서 큐에 내용이 저장 될 경우
		// 클라이언트에게 다시 동일 내용을 Send 하게 하는 에코 서버 형태이다.
		process_thread_ = std::thread([this]() {ProcessPacket();  });

		StartServer(maxClient);
	}

	void End()
	{
		is_run_process_thread_ = false;

		if (process_thread_.joinable())
		{
			process_thread_.join();
		}

		DestroyThread();
	}

private:
	void ProcessPacket()
	{
		while (is_run_process_thread_)
		{
			// DequePacketData() 큐에서 데이터 읽어오기
			auto packetData = DequePacketData();
			if (packetData.DataSize != 0)
			{
				SendMsg(packetData.ClientIndex, packetData.DataSize, packetData.pPacketData);
			}
			else
			{
				// 큐에 데이터가 없으면 쓰레드 쉬어라
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}
	} // ProcessPacket()

	PacketData DequePacketData()
	{
		PacketData packetData;

		// lock_guard : 이 scope 벗어나면 lock이 해제 됩니다.
		std::lock_guard<std::mutex> guard(lock_);

		// 패킷 큐에 아무것도 없으면
		if (packet_data_queue_.empty())
		{
			return PacketData();
		}

		// 디큐 제일 앞에 내용을 packetData
		packetData.Set(packet_data_queue_.front());

		// 디큐 제일 앞 삭제 처리 delete pPacketData;
		packet_data_queue_.front().Release();
		// 디큐에서 제일 앞부분 삭제
		packet_data_queue_.pop_front();

		return packetData;

	}

	bool is_run_process_thread_ = false;

	std::thread process_thread_;

	std::mutex lock_;
	std::deque<PacketData> packet_data_queue_;
};

