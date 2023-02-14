#include <utility>
#include <cstring>

#include "UserManager.h"
#include "PacketManager.h"

void PacketManager::Init(const UINT32 max_client)
{
	recv_funtion_dictionary_ = std::unordered_map<int, PROCESS_RECV_PACKET_FUNCTION>();

	recv_funtion_dictionary_[(int)PACKET_ID::SYS_USER_CONNECT] = &PacketManager::ProcessuserConnect;
	recv_funtion_dictionary_[(int)PACKET_ID::SYS_USER_DISCONNECT] = &PacketManager::ProcessUserDisConnect;
	recv_funtion_dictionary_[(int)PACKET_ID::LOGIN_REQUEST] = &PacketManager::ProcessLogin;

	CreateCompent(max_client);
}

void PacketManager::CreateCompent(const UINT32 max_client)
{
	user_manager_ = new UserManager;
	user_manager_->Init(max_client);
}

bool PacketManager::Run()
{
	//이 부분을 패킷 처리 부분으로 이동 시킨다.
	is_run_process_thread_ = true;
	// 패킷 쓰레드 생성
	process_thread_ = std::thread([this]() {ProcessPacket(); });

	return true;
}

void PacketManager::End()
{
	is_run_process_thread_ = false;

	if (process_thread_.joinable())
	{
		process_thread_.join();
	}
}

void PacketManager::ReceivePacketData(const UINT32 client_index, const UINT32 size, char* P_data)
{
	// GetUserByConnIdx 어떤 클라이언트가 메세지를 보냈는지 확인
	auto P_user = user_manager_->GetUserByConnIdx(client_index);

	// 메세지를 보낸 클라이언트 객체에 데이터를 담음
	P_user->SetPacketData(size, P_data);

	// 큐에 리시브가 발생했다는 것을 알려줌
	EnqueuePacketData(client_index);
}

void PacketManager::ProcessPacket()
{
	while (is_run_process_thread_)
	{
		bool is_idle = true;

		// 이미 연결이 된 유저가 보낸 요청이 있는 경우
		if (auto packet_data = DequePacketData(); packet_data.client_index_ > (UINT16)PACKET_ID::SYS_END)
		{
			is_idle = false;
			// 요청이 있는 경우 처리
			ProcessRecvPacket(packet_data.client_index_, packet_data.packet_id_, packet_data.data_size_, packet_data.P_data_ptr_);
		}

		// 시스템 패킷 (연결 & 연결 종료가 발생한 경우)
		if (auto packet_data = DequeSystemPacketData(); packet_data.packet_id_ != 0)
		{
			is_idle = false;
			// 요청이 있는 경우 처리
			ProcessRecvPacket(packet_data.client_index_, packet_data.packet_id_, packet_data.data_size_, packet_data.P_data_ptr_);
		}

		if (is_idle)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
}

void PacketManager::EnqueuePacketData(const UINT32 client_index)
{
	std::lock_guard<std::mutex> guard(lock_);
	in_coming_packet_user_index_.push_back(client_index);
}

PacketInfo PacketManager::DequePacketData()
{
	UINT32 user_index = 0;

	{
		std::lock_guard<std::mutex> grard(lock_);
		// 현재 요청을 보낸 유저가 있는지 확인
		if (in_coming_packet_user_index_.empty())
		{
			return PacketInfo();
		}

		// 요청한 데이터가 있다면 index 추출
		user_index = in_coming_packet_user_index_.front();
		in_coming_packet_user_index_.pop_front();
	}

	// 알아낸 index로 유저 객체 가져옴
	auto P_user = user_manager_->GetUserByConnIdx(user_index);
	auto packet_data = P_user->GetPacket();
	packet_data.client_index_ = user_index;

	return packet_data;
}

void PacketManager::PushSystemPacket(PacketInfo packet)
{
	std::lock_guard<std::mutex> guard(lock_);
	// 패킷 헤더에 따라서 처리
	system_packet_queue_.push_back(packet);
}

PacketInfo PacketManager::DequeSystemPacketData()
{
	std::lock_guard<std::mutex> guard(lock_);
	if (system_packet_queue_.empty())
	{
		return PacketInfo();
	}

	auto packet_data = system_packet_queue_.front();
	system_packet_queue_.pop_front();

	return packet_data;
}

void PacketManager::ProcessRecvPacket(const UINT32 client_index, const UINT16 packet_id, const UINT16 packet_size, char* P_packet)
{
	// 요청한 패킷 id로 클라이언트 객체 찾음
	auto iter = recv_funtion_dictionary_.find(packet_id);
	if (iter != recv_funtion_dictionary_.end())
	{
		// 찾은 객체로 처리
		(this->*(iter->second))(client_index, packet_size, P_packet);
	}
}

void PacketManager::ProcessuserConnect(UINT32 client_index, UINT16 packet_size, char* P_packet)
{
	// 새로 연결 받기
	printf("[ProcessUserConnect] clientIndex: %d\n", client_index);
	// 하나의 user_manger 객체 할당
	auto P_user = user_manager_->GetUserByConnIdx(client_index);
	// 객체 초기화
	P_user->Clear();
}


void PacketManager::ProcessUserDisConnect(UINT32 client_index, UINT16 packet_size, char* P_packet)
{
	printf("[ProcessUserDisConnect] clientIndex: %d\n", client_index);
	// 연결이 끊어진 경우 user 반환
	ClearConnectionInfo(client_index);
}

void PacketManager::ProcessLogin(UINT32 client_index, UINT16 packet_size, char* P_packet)
{
	if (LOGIN_REQUEST_PACKET_SZIE != packet_size)
	{
		return;
	}

	auto  P_login_req_packet = reinterpret_cast<LOGIN_REQUEST_PACKET*>(P_packet);

	auto P_user_id = P_login_req_packet->user_id_;
	printf("requested user id = %s\n", P_user_id);

	LOGIN_RESPONSE_PACKET login_res_packet;
	login_res_packet.packet_id_ = (UINT16)PACKET_ID::LOGIN_RESPONSE;
	login_res_packet.packet_length_ = sizeof(LOGIN_RESPONSE_PACKET);

	if (user_manager_->GetCurrentUserCnt() >= user_manager_->GetMaxUserCnt())
	{
		//접속자수가 최대수를 차지해서 접속불가
		login_res_packet.result_ = (UINT16)ERROR_CODE::LOGIN_USER_USED_ALL_OBJ;
		SendPacketFunc(client_index, sizeof(LOGIN_RESPONSE_PACKET), (char*)&login_res_packet);
		return;
	}

	//여기에서 이미 접속된 유저인지 확인하고, 접속된 유저라면 실패한다.
	if (user_manager_->FindUserIndexByID(P_user_id) == -1)
	{
		login_res_packet.result_ = (UINT16)ERROR_CODE::NONE;
		SendPacketFunc(client_index, sizeof(LOGIN_RESPONSE_PACKET), (char*)&login_res_packet);
	}
	else
	{
		//접속중인 유저여서 실패를 반환한다.
		login_res_packet.result_ = (UINT16)ERROR_CODE::LOGIN_USER_ALREADY;
		SendPacketFunc(client_index, sizeof(LOGIN_REQUEST_PACKET), (char*)&login_res_packet);
		return;
	}
}


void PacketManager::ClearConnectionInfo(INT32 client_index)
{
	auto P_req_user = user_manager_->GetUserByConnIdx(client_index);

	if (P_req_user->GetDomainState() != User::DOMAIN_STATE::NONE)
	{
		user_manager_->DeleteUserInfo(P_req_user);
	}
}









