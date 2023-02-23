#pragma once
#include "IocpNetServer.h"
#include "PacketManager.h"
#include "Packet.h"
//FSM
#include "FSM.h"
#include "BlueNpc.h"
#include "RedNpc.h"
#include "Player.h"

#include <vector>
#include <deque>
#include <thread>
#include <mutex>
#include <memory>

//typedef struct Player_Socket {
//	// ������ ���� ���
//	float x = 0, y = 0, z = 0;
//}Player_Socket;
//
//Player_Socket player;
//
//typedef struct Server_Player {
//	//�÷��̾� �����͸� ���������
//	Player_Socket player[4];
//}Server_Player;
//Server_Player server_data;

//TODO redis ����. hiredis �����ϱ�
class AppServer : public IocpNetServer
{


public:
	FSM fsm;
	Player player;
	BlueNpc blue_npc_;
	RedNpc red_npc_;

	std::thread npc_process_thread_;
public:
	AppServer();
	virtual ~AppServer() = default;

	void FsmInit();
	void ThreadTestfuntion();

	struct MyStruct
	{

	};
	virtual void OnConnect(const UINT32 client_index) override
	{
		// ���� �Ǹ� �÷��̾� ���� �̸� �� ����
		//Player player;

		// TODO �����ϸ� npc ���󰡱� ���� �ٵ� �̸� ��ǥ�� �����̰� �־���Ѵ�.

		printf("[OnConnect] Ŭ���̾�Ʈ: Index(%d)\n", client_index);

		// PacketInfo ����
		// UINT32 client_index_ = 0;
		// UINT16 packet_id_ = 0;
		// UINT16 data_size_ = 0;
		PacketInfo packet{ client_index, (UINT16)PACKET_ID::SYS_USER_CONNECT, 0 };

		// SYS_USER_CONNECT ��Ŷ�� ����
		P_packet_manager_->PushSystemPacket(packet);
	}

	virtual void OnClose(const UINT32 client_index_) override
	{
		printf("[OnClose] Ŭ���̾�Ʈ: Index(%d)\n", client_index_);

		PacketInfo packet{ client_index_, (UINT16)PACKET_ID::SYS_USER_DISCONNECT, 0 };
		P_packet_manager_->PushSystemPacket(packet);
	}

	virtual void OnReceive(const UINT32 client_index, const UINT32 size, char* P_recv_data) override
	{
		printf("[OnReceive] Ŭ���̾�Ʈ: Index(%d), dataSize(%d), recvData : %s\n", client_index, size, P_recv_data);

		P_packet_manager_->ReceivePacketData(client_index, size, P_recv_data);

		//Receive ȣ�� Send ȣ���ؼ� ���� ���� ���·� ����
		//P_packet_manager_->SendPacketFunc(client_index, size, P_recv_data);
		SendMsg(client_index, size, P_recv_data);
		//BroadcastSendMsg(P_recv_data);
	}

	void NpcInit()
	{
		// ������ ���ִٰ� �ð� ������ �����̱�
		fsm.AddTransition(STATE_STAND, EVENT_TIMEMOVE, STATE_MOVE);
		// �̵� ����
		fsm.AddTransition(STATE_STAND, EVENT_STARTMOVE, STATE_MOVE);
		// ���� ��ȯ
		fsm.AddTransition(STATE_MOVE, EVENT_TRUNMOVE, STATE_MOVE);
		// Ÿ�� �߰��ϸ� Ÿ�Ͽ��� �ٰ�����
		fsm.AddTransition(STATE_STAND, EVENT_POINTMOVE, STATE_POINT_MOVE);
		// ������ ���ִٰ� Ÿ�� �߰��ϸ� ����
		fsm.AddTransition(STATE_STAND, EVENT_FINDTARGET, STATE_ATTACK);
		// �����̴ٰ� ���߱�
		fsm.AddTransition(STATE_MOVE, EVENT_STOPMOVE, STATE_STAND);
		// �����ϴٰ� Ÿ�� �������� ���߱�
		fsm.AddTransition(STATE_ATTACK, EVENT_LOSTTARGET, STATE_STAND);
		
		blue_npc_.SetFsm(&fsm);
	}

	void NpcProcess()
	{
		while (1)
		{
			blue_npc_.Process(&player, &red_npc_);
			
			if (blue_npc_.SetTest() == true)
			{
				std::cout << "AppSever : SetTest()\n";
				printf("���� ������ .. : %d : %d ���� : %d\n", (int)blue_npc_.npc_info_.npc_pos_[0], (int)blue_npc_.npc_info_.npc_pos_[1], blue_npc_.npc_info_.npc_pos_dir_);
				
				// TODO �̻����� �ѱ�� �ʹ�
				blue_npc_.npc_info_.npc_pos_;
				//char temp_send_buf_[MAX_SOCKBUF];  //������ ����
				char temp_send_buf_[5] = "min";
				char temp_send_buf1_[1] = { blue_npc_.npc_info_.npc_pos_[0] };
				//temp_send_buf_.size();
				//BroadcastSendMsg(temp_send_buf1_);
				BroadcastSendMsg((char*)&blue_npc_.npc_info_.npc_pos_);
				//send();
			}
			Sleep(1000);
		}
	}

	void NpcRun()
	{
		npc_process_thread_ = std::thread([this]() {NpcProcess(); });
	}

	void Run(const UINT32 max_client)
	{
		// ���ٷ� funtion ��ü�� SendMsg ����
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

