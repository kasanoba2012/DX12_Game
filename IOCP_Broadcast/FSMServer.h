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
	// FSM ����
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
		// ���� �Ǹ� �÷��̾� ����
		Player player;

		// TODO �����ϸ� npc ���󰡱� ���� �ٵ� �̸� ��ǥ�� �����̰� �־���Ѵ�.
		while (0)
		{
			printf("Connect npc ��ǥ : %d\n", (int)npc.m_NpcPos[0]);

			MovementSw = false;

			char npcPosMsg[256] = { 0, };
			*npcPosMsg = npc.m_NpcPos[0];
			npcPosMsg[1] = '\0';
			mainSendMsg(npcPosMsg);

			npc.Process(&player);
			Sleep(3000);
		}

		printf("[OnConnect] Ŭ���̾�Ʈ: Index(%d)\n", client_index_);
	}

	virtual void OnClose(const UINT32 client_index_) override
	{
		printf("[OnClose] Ŭ���̾�Ʈ: Index(%d)\n", client_index_);
	}

	virtual void OnReceive(const UINT32 client_index_, const UINT32 size_, char* P_data_) override
	{
		printf("[OnReceive] Ŭ���̾�Ʈ: Index(%d), dataSize(%d), data : %s\n", client_index_, size_, P_data_);

		// Ŭ���̾�Ʈ���� ���� Recv �����͸� ť�� �־��ֱ� ���ؼ� ��Ŷ���·� ��ȯ�Ѵ�.
		// PacketData : �ε���, ������ ������, ������ ���� ���� �ּ� �޴� ����ü
		PacketData packet;
		packet.Set(client_index_, size_, P_data_);

		// lock_guard : �� scope ����� lock�� ���� �˴ϴ�.
		std::lock_guard<std::mutex> guard(lock_);
		packet_data_queue_.push_back(packet);
	}

	void Run(const UINT32 maxClient)
	{
		is_run_process_thread_ = TRUE;
		// ProcessPacket() : ��Ŷ ó�� ������
		// Ŭ���̾�Ʈ���� Send�� ������� Send ������ PacketData Ÿ���� ť�� ������ ��Ű�� ������ �̸�, Send���� ������ �޾Ƽ� ť�� ������ ���� �� ���
		// Ŭ���̾�Ʈ���� �ٽ� ���� ������ Send �ϰ� �ϴ� ���� ���� �����̴�.
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
			// DequePacketData() ť���� ������ �о����
			auto packetData = DequePacketData();
			if (packetData.DataSize != 0)
			{
				SendMsg(packetData.ClientIndex, packetData.DataSize, packetData.pPacketData);
			}
			else
			{
				// ť�� �����Ͱ� ������ ������ �����
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}
	} // ProcessPacket()

	PacketData DequePacketData()
	{
		PacketData packetData;

		// lock_guard : �� scope ����� lock�� ���� �˴ϴ�.
		std::lock_guard<std::mutex> guard(lock_);

		// ��Ŷ ť�� �ƹ��͵� ������
		if (packet_data_queue_.empty())
		{
			return PacketData();
		}

		// ��ť ���� �տ� ������ packetData
		packetData.Set(packet_data_queue_.front());

		// ��ť ���� �� ���� ó�� delete pPacketData;
		packet_data_queue_.front().Release();
		// ��ť���� ���� �պκ� ����
		packet_data_queue_.pop_front();

		return packetData;

	}

	bool is_run_process_thread_ = false;

	std::thread process_thread_;

	std::mutex lock_;
	std::deque<PacketData> packet_data_queue_;
};

