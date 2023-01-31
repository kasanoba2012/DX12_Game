#include "Iocp.h"
unsigned WINAPI Iocp::WorkProc(LPVOID arg)
{
	Iocp* pIocp = (Iocp*)arg;
	DWORD dwTransfer;
	ULONG_PTR KeyValue;
	// OVERLAPPED : 비동기 파일 I/O Windows Api
	OVERLAPPED* pOV;
	while (1)
	{
		// WaitForSingleObject : 커널 오브젝트 상태 확인
		// 프로세서 생성 Non-Signal, 프로세서 종료 Signal
		DWORD dwEvent = WaitForSingleObject(pIocp->m_hEventFinish, 0);

		// WAIT_FAILED : GetLastError()으로 원인을 알 수 있다.
		// WAIT_ABANDONED : Event object를 reset 하고 다시 WaitForSinalgeObject() 호출
		// WAIT_OBJECT_0 : 기다리던 Event가 Signal 된 경우
		// WAIT_TEMEOUT : time-out이 된 경우
		if (dwEvent == WAIT_OBJECT_0)
		{
			break;
		}

		// 비동기 읽기 완성 여부 판단
		// GetQueuedCompletionStatus : IOCP 입출력 완료 대기열로부터 입출력 완료를 기다림
		BOOL bRet = ::GetQueuedCompletionStatus(pIocp->m_hIOCP,
			&dwTransfer,
			&KeyValue,
			&pOV, INFINITE);

		OVERLAPPED2* pOV2 = (OVERLAPPED2*)pOV;
		SessionUser* user = (SessionUser*)KeyValue;

		// OVERLAPPED2를 통해 send recv accept 처리
		if (bRet == TRUE)
		{
			// 읽기 또는 쓰기 비동기 완성
			if (OVERLAPPED2::MODE_RECV == pOV2->iType)
			{
				user->DispatchRead(dwTransfer, pOV2);
				user->RecvMsg();
			}
			if (OVERLAPPED2::MODE_SEND == pOV2->iType)
			{
				user->DispatchSend(dwTransfer, pOV2);
			}
		}
		else
		{
			DWORD dwError = GetLastError();
			if (dwError == ERROR_NETNAME_DELETED)
			{
				user->m_bDisConnect = true;
			}
		}
	}
	return 0;
}
bool	Iocp::Init()
{
	// CreateIoCompletionPort : IOCP 커널 객체 생성하거나 IOCP 디바이스를 연결
	m_hIOCP = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	
	// CreateEvent : (NULL, SetEvent시 Signal 상태 유지 할건지?, Signal 상태로 생성할건지?, 이벤트이름)
	m_hEventFinish = ::CreateEvent(0, TRUE, FALSE, 0);

	for (int iThread = 0; iThread < MAX_WORKER_THREAD; iThread++)
	{
		m_hWorkThread[iThread] =
			// _beginthreadex() : 멀티 쓰레드에 적합
			_beginthreadex(NULL, 0, WorkProc, this, 0,
				&m_iThreadID[iThread]);

	}
	return true;
}
bool	Iocp::SetBind(SOCKET sock, ULONG_PTR key)
{
	// CreateIoCompletionPort : IOCP 커널 객체 생성하거나 IOCP 디바이스를 연결
	::CreateIoCompletionPort((HANDLE)sock, m_hIOCP, key, 0);

	return true;
}
bool	Iocp::Run()
{
	return true;
}
bool	Iocp::Release()
{
	// CloseHandle : 핸들의 프로세서를 종료시켜주는게 아니라 이제 이 프로세서를 참조하지 않겠다는 뜻이다.
	CloseHandle(m_hIOCP);
	for (int iThread = 0; iThread < MAX_WORKER_THREAD; iThread++)
	{
		CloseHandle((HANDLE)m_hWorkThread[iThread]);
	}
	return true;
}