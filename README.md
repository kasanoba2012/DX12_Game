# IOCP_Server

## IOCP API
  - CreateIoCompletionPort();
    - 1차 생성 IOCP 객체 생성
    - 2차 생성 Socket을 IOCP에 연결
  - GetQueueCompletionStatus();
    - IOCP로 부터 I/O 결과를 얻음
    
## IOCP 서버 설계 (멀티 쓰레드 버전)
- 초기화
  - 윈속 초기화, bind, listen, IOCP 객체 및 핸들 생성
- 접속 클라이언트 정보 미리 생성
- Listen 소켓 IOCP 등록 후 AcceptEx 호출
- 서버 메인 루프
  - 새로운 소켓 접속 하면 IOCP 연결 후 WSARecv 호출
  - 클라이언트 Send 발생 시 서버는 Recv 처리 후 Recv Data Send queue에 담아서 클라이언트에게 동일 내용 Send

## IOCP 기능 설명

- Init();
  - WinSocket 초기화
  - listen_socket_ 소켓 생성
  
- BindAndListen();
  - listen_socket_ 주소 할당
  - listen_socket_ 수신열 추가
  - Iocp 객체 생성
  - listen_socket_으로 CompletionKey 없는 Iocp 연결
  
- Run();
  - CreateClient();
    - index, iocp_handle(Iocp 객체)를 담아서 접속을 허용할 클라이언트 수만큼 클라이언트 정보 미리 생성
      - 미리 생성된 모든 클라이언트 정보는 client_Infos_ 담고있다
  - CreateAccepterThread();
    - ReadyAccept();
      - 인터넷 연결 가능 소켓 할당
      - 인터넷 가능 소켓 & listen_socket으로 AccectEx
      - 미리 생성된 모든 클라이언트 객체 Accept 예약 완료
  - CreateWokerThread();
    - GetQueuedCompletionStatus()
      - IOCP로 부터 I/O 결과를 얻음
    - Accept 처리
      - Client에서 Connect이 발생하면 GetQueuedCompletionStatus의 CompletionKey로 호출 socket I/O 확인
      - 호출 socket session index를 통해 client_Infos_에서 동일 index 클라이언트 정보를 가져옴
      - client_Infos_에 저장되어 있는 socket과 iocp 객체를 통해 CreateIoCompletionPort Lisent_socket으로 할당 한 것을 client_Infos_ Socket으로 변경
      - 접속 클라이언트 Socket 연결 완료 후 Recv()로 수신 준비
    - Recv 처리
      - GetQueuedCompletionStatus의 CompletionKey로 호출 소켓 I/O 확인
      - 호출 socket의 index, RecvBuffer 내용을 콘솔 출력
      - 동일 메시지 WsaSend를 통해 Echo Server 구성
    - Send 처리
      - GetQueuedCompletionStatus의 CompletionKey로 호출 소켓 I/O 확인
      - send_data_queue_ 제일 앞 부분 삭제
      - send_data_queue_ 비어 있지 않은 상태면 WSASend() 호출
      
- SendMsg
  - SendMsg(client_index, size, P_recv_data); (클라이언트 인덱스, 보낼 데이터 사이즈, 보낼 데이터)
    - client_index를 통해 client_Infos_에서 클라이언트 정보 가져옴
    - 가져온 클라이언트 정보에서 SendMsg(data_size, P_send_data);
  - SendMsg(data_size, P_send_data);(보낼 데이터 사이즈, 보낼 데이터)
    - send_overlapped 구조체에 데이터 포장 후 send_data_queue_.push(send_overlapped_ex)
    - send_data_queue_ size 확인 후 보낼 데이터가 있으면 WsaSend() 호출
  - ProcessPacket()
    - 향후 추가

사용 쓰레드
accepter_thread_ 1개
io_worker_threads_ 4개
