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

	virtual void OnConnect(const UINT32 clientIndex_) override
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

		printf("[OnConnect] Ŭ���̾�Ʈ: Index(%d)\n", clientIndex_);
	}

	virtual void OnClose(const UINT32 clientIndex_) override
	{
		printf("[OnClose] Ŭ���̾�Ʈ: Index(%d)\n", clientIndex_);
	}

	virtual void OnReceive(const UINT32 clientIndex_, const UINT32 size_, char* pData_) override
	{
		printf("[OnReceive] Ŭ���̾�Ʈ: Index(%d), dataSize(%d), data : %s\n", clientIndex_, size_, pData_);

		// Ŭ���̾�Ʈ���� ���� Recv �����͸� ť�� �־��ֱ� ���ؼ� ��Ŷ���·� ��ȯ�Ѵ�.
		// PacketData : �ε���, ������ ������, ������ ���� ���� �ּ� �޴� ����ü
		PacketData packet;
		packet.Set(clientIndex_, size_, pData_);

		// lock_guard : �� scope ����� lock�� ���� �˴ϴ�.
		std::lock_guard<std::mutex> guard(_Lock);
		_PacketDataQueue.push_back(packet);
	}

	void Run(const UINT32 maxClient)
	{
		_IsRunProcessThread = TRUE;
		// ProcessPacket() : ��Ŷ ó�� ������
		// Ŭ���̾�Ʈ���� Send�� ������� Send ������ PacketData Ÿ���� ť�� ������ ��Ű�� ������ �̸�, Send���� ������ �޾Ƽ� ť�� ������ ���� �� ���
		// Ŭ���̾�Ʈ���� �ٽ� ���� ������ Send �ϰ� �ϴ� ���� ���� �����̴�.
		_ProcessThread = std::thread([this]() {ProcessPacket();  });

		StartServer(maxClient);
	}

	void End()
	{
		_IsRunProcessThread = false;

		if (_ProcessThread.joinable())
		{
			_ProcessThread.join();
		}

		DestroyThread();
	}

private:
	void ProcessPacket()
	{
		while (_IsRunProcessThread)
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
		std::lock_guard<std::mutex> guard(_Lock);

		// ��Ŷ ť�� �ƹ��͵� ������
		if (_PacketDataQueue.empty())
		{
			return PacketData();
		}

		// ��ť ���� �տ� ������ packetData
		packetData.Set(_PacketDataQueue.front());

		// ��ť ���� �� ���� ó�� delete pPacketData;
		_PacketDataQueue.front().Release();
		// ��ť���� ���� �պκ� ����
		_PacketDataQueue.pop_front();

		return packetData;

	}
	bool _IsRunProcessThread = false;

	std::thread _ProcessThread;

	std::mutex _Lock;
	std::deque<PacketData> _PacketDataQueue;
};

