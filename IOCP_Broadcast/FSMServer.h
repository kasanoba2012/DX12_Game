#pragma once
#include "IOCP_Net_Server.h"
#include "Npc.h"
#include "Player.h"
#include "PacketManager.h"
#include "Packet.h"

#include <vector>
#include <deque>
#include <thread>
#include <mutex>
#include <memory>

//TODO redis ����. hiredis �����ϱ�

class FSMServer : public IOCP_Net_Server
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

	virtual void OnConnect(const UINT32 client_index) override
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
		
		printf("[OnConnect] Ŭ���̾�Ʈ: Index(%d)\n", client_index);

		PacketInfo packet{ client_index, (UINT16)PACKET_ID::SYS_USER_DISCONNECT, 0 };
		// ��ũ�����忡�� ���� ��Ŷ�� ��Ŷ �Ŵ����� ������ ��Ŷ �����忡�� ó���ϴ°� Ȯ��
		P_packet_manager_->PushSystemPacket(packet);
	}

	virtual void OnClose(const UINT32 client_index_) override
	{
		printf("[OnClose] Ŭ���̾�Ʈ: Index(%d)\n", client_index_);

		PacketInfo packet{ client_index_, (UINT16)PACKET_ID::SYS_USER_DISCONNECT, 0 };
		P_packet_manager_->PushSystemPacket(packet);
	}

	virtual void OnReceive(const UINT32 client_index, const UINT32 size, char* P_data) override
	{
		printf("[OnReceive] Ŭ���̾�Ʈ: Index(%d), dataSize(%d), data : %s\n", client_index, size, P_data);

		P_packet_manager_->ReceivePacketData(client_index, size, P_data);
	}

	void Run(const UINT32 max_client)
	{
		auto send_packet_func = [&](UINT32 client_index, UINT16 packet_size, char* P_send_packet)
		{
			SendMsg(client_index, packet_size, P_send_packet);
		};

		// ��Ŷ �Ŵ��� ����
		P_packet_manager_ = std::make_unique<PacketManager>();
		P_packet_manager_->SendPacketFunc = send_packet_func;
		P_packet_manager_->Init(max_client);
		
		// ��Ŷ������ ����
		P_packet_manager_->Run();

		StartServer(max_client);
	}

	void End()
	{
		P_packet_manager_->End();

		DestroyThread();
	}

private:

	std::unique_ptr<PacketManager> P_packet_manager_;
};

