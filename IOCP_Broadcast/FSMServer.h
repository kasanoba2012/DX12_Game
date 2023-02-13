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

//TODO redis 연동. hiredis 포함하기

class FSMServer : public IOCP_Net_Server
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

	virtual void OnConnect(const UINT32 client_index) override
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
		
		printf("[OnConnect] 클라이언트: Index(%d)\n", client_index);

		PacketInfo packet{ client_index, (UINT16)PACKET_ID::SYS_USER_DISCONNECT, 0 };
		// 워크쓰레드에서 받은 패킷을 패킷 매니저로 보내서 패킷 쓰레드에서 처리하는거 확인
		P_packet_manager_->PushSystemPacket(packet);
	}

	virtual void OnClose(const UINT32 client_index_) override
	{
		printf("[OnClose] 클라이언트: Index(%d)\n", client_index_);

		PacketInfo packet{ client_index_, (UINT16)PACKET_ID::SYS_USER_DISCONNECT, 0 };
		P_packet_manager_->PushSystemPacket(packet);
	}

	virtual void OnReceive(const UINT32 client_index, const UINT32 size, char* P_data) override
	{
		printf("[OnReceive] 클라이언트: Index(%d), dataSize(%d), data : %s\n", client_index, size, P_data);

		P_packet_manager_->ReceivePacketData(client_index, size, P_data);
		//Receive 호출 Send 호출해서 에코 서버 형태로 만듬
		P_packet_manager_->SendPacketFunc(client_index, size, P_data);
	}

	void Run(const UINT32 max_client)
	{
		// 람다로 funtion 객체에 SendMsg 담음
		auto send_packet_func = [&](UINT32 client_index, UINT16 packet_size, char* P_send_packet)
		{
			SendMsg(client_index, packet_size, P_send_packet);
		};

		// 패킷 매니저 생성
		P_packet_manager_ = std::make_unique<PacketManager>();
		P_packet_manager_->SendPacketFunc = send_packet_func;
		P_packet_manager_->Init(max_client);
		
		// 패킷쓰레드 생성
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

