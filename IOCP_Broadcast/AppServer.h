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

//TODO redis 연동. hiredis 포함하기
class AppServer : public IocpNetServer
{
public:
	FSM fsm;
	Player player;
	// TODO 원하는 npc만큼 담는 pool 형태로 만들어야함
	BlueNpc blue_npc_;
	BlueNpc blue_npc1_;
	std::vector<BlueNpc*> npc_pool_;
	RedNpc red_npc_;
	// npc 갯수
	UINT32 max_npc_cnt_ = 10;

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
		// 접속 되면 플레이어 생성 미리 쭉 생성
		//Player player;

		// TODO 접속하면 npc 따라가기 시작 근데 미리 좌표는 움직이고 있어야한다.

		printf("[OnConnect] 클라이언트: Index(%d)\n", client_index);

		// PacketInfo 정보
		// UINT32 client_index_ = 0;
		// UINT16 packet_id_ = 0;
		// UINT16 data_size_ = 0;
		PacketInfo packet{ client_index, (UINT16)PACKET_ID::SYS_USER_CONNECT, 0 };

		// SYS_USER_CONNECT 패킷을 전달
		P_packet_manager_->PushSystemPacket(packet);
		
		// TODO 로그인 헤더 접속하면 보내기
		//HeaderSendMsg(client_index, PACKET_CHATNAME_REQ);
	}

	virtual void OnClose(const UINT32 client_index_) override
	{
		printf("[OnClose] 클라이언트: Index(%d)\n", client_index_);

		PacketInfo packet{ client_index_, (UINT16)PACKET_ID::SYS_USER_DISCONNECT, 0 };
		P_packet_manager_->PushSystemPacket(packet);
	}

	virtual void OnReceive(const UINT32 client_index, const UINT32 size, char* P_recv_data) override
	{
		printf("[OnReceive] 클라이언트: Index(%d), dataSize(%d), recvData : %s\n", client_index, size, P_recv_data);

		// client_index 어떤 클라가 보냈는지 알수 있는 index
		P_packet_manager_->ReceivePacketData(client_index, size, P_recv_data);

		//Receive 호출 Send 호출해서 에코 서버 형태로 만듬
		//P_packet_manager_->SendPacketFunc(client_index, size, P_recv_data);
		SendMsg(client_index, size, P_recv_data);
	}

	void NpcInit()
	{
		// 가만히 서있다가 시간 지나면 움직이기
		fsm.AddTransition(STATE_STAND, EVENT_TIMEMOVE, STATE_MOVE);
		// 이동 시작
		fsm.AddTransition(STATE_STAND, EVENT_STARTMOVE, STATE_MOVE);
		// 방향 전환
		fsm.AddTransition(STATE_MOVE, EVENT_TRUNMOVE, STATE_MOVE);
		// 타켓 발견하면 타켓에게 다가가기
		fsm.AddTransition(STATE_STAND, EVENT_POINTMOVE, STATE_POINT_MOVE);
		// 가만히 서있다가 타켓 발견하면 공격
		fsm.AddTransition(STATE_STAND, EVENT_FINDTARGET, STATE_ATTACK);
		// 움직이다가 멈추기
		fsm.AddTransition(STATE_MOVE, EVENT_STOPMOVE, STATE_STAND);
		// 공격하다가 타켓 없어지면 멈추기
		fsm.AddTransition(STATE_ATTACK, EVENT_LOSTTARGET, STATE_STAND);
		
		NpcCreate(max_npc_cnt_);
		SetNpcFsm(max_npc_cnt_);
	}


	void NpcProcess()
	{
		while (1)
		{
			//blue_npc_.Process(&player, &red_npc_);
			//
			//if (blue_npc_.NpcChangeDirection() == true)
			//{
			//	std::cout << "AppSever : NpcChangeDirection()\n";
			//	printf("Npc : %d : %d 방향 : %d\n", (int)blue_npc_.npc_info_.npc_pos_[0], (int)blue_npc_.npc_info_.npc_pos_[1], blue_npc_.npc_info_.npc_pos_dir_);
			//	
			//	BroadcastSendMsg(&blue_npc_);
			//}

			for (UINT32 i = 0; i < max_npc_cnt_; ++i)
			{
				npc_pool_[i]->Process(&player, &red_npc_);

				if (npc_pool_[i]->NpcChangeDirection() == true)
				{
					//std::cout << "AppSever : NpcChangeDirection()\n";

					//printf("npc_pool_ : %d : %d 방향 : %d npc Index : %d\n", (int)npc_pool_[i]->npc_info_.npc_pos_[0], (int)npc_pool_[i]->npc_info_.npc_pos_[1], npc_pool_[i]->npc_info_.npc_pos_dir_, i);

					//npc_pool_[i];
					BroadcastSendMsg(npc_pool_[i]);
				}
			}

			//npc_pool_[0]->Process(&player, &red_npc_);

			//if (npc_pool_[0]->NpcChangeDirection() == true)
			//{
			//	std::cout << "AppSever : NpcChangeDirection()\n";

			//	printf("npc_pool_ : %d : %d 방향 : %d\n", (int)npc_pool_[0]->npc_info_.npc_pos_[0], (int)npc_pool_[0]->npc_info_.npc_pos_[1], npc_pool_[0]->npc_info_.npc_pos_dir_);
			//}


			//blue_npc1_.Process(&player, &red_npc_);

			//if (blue_npc1_.NpcChangeDirection() == true)
			//{
			//	std::cout << "AppSever1 : NpcChangeDirection()\n";
			//	printf("Npc : %d : %d 방향 : %d\n", (int)blue_npc1_.npc_info_.npc_pos_[0], (int)blue_npc1_.npc_info_.npc_pos_[1], blue_npc1_.npc_info_.npc_pos_dir_);

			//	BroadcastSendMsg(&blue_npc_);
			//}

			Sleep(100);
		}
	}

	void NpcRun()
	{
		npc_process_thread_ = std::thread([this]() {NpcProcess(); });
	}

	void NpcCreate(UINT32 max_npc_cnt)
	{
		for (UINT32 i = 0; i < max_npc_cnt; ++i)
		{
			auto npc = new BlueNpc();
			npc->init(i);
			npc_pool_.push_back(npc);
		}
	}

	void SetNpcFsm(UINT32 max_npc_cnt)
	{
		for (UINT32 i = 0; i < max_npc_cnt; ++i)
		{
			npc_pool_[i]->SetFsm(&fsm);
		}
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
		// 클라이언트 수 만큼 user 생성 index, packet_data_buffer 초기화
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

