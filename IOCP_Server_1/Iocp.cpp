#include "Iocp.h"
unsigned WINAPI Iocp::WorkProc(LPVOID arg)
{
	Iocp* pIocp = (Iocp*)arg;
	DWORD dwTransfer;
	ULONG_PTR KeyValue;
	// OVERLAPPED : �񵿱� ���� I/O Windows Api
	OVERLAPPED* pOV;
	while (1)
	{
		// WaitForSingleObject : Ŀ�� ������Ʈ ���� Ȯ��
		// ���μ��� ���� Non-Signal, ���μ��� ���� Signal
		DWORD dwEvent = WaitForSingleObject(pIocp->m_hEventFinish, 0);

		// WAIT_FAILED : GetLastError()���� ������ �� �� �ִ�.
		// WAIT_ABANDONED : Event object�� reset �ϰ� �ٽ� WaitForSinalgeObject() ȣ��
		// WAIT_OBJECT_0 : ��ٸ��� Event�� Signal �� ���
		// WAIT_TEMEOUT : time-out�� �� ���
		if (dwEvent == WAIT_OBJECT_0)
		{
			break;
		}

		// �񵿱� �б� �ϼ� ���� �Ǵ�
		// GetQueuedCompletionStatus : IOCP ����� �Ϸ� ��⿭�κ��� ����� �ϷḦ ��ٸ�
		BOOL bRet = ::GetQueuedCompletionStatus(pIocp->m_hIOCP,
			&dwTransfer,
			&KeyValue,
			&pOV, INFINITE);

		OVERLAPPED2* pOV2 = (OVERLAPPED2*)pOV;
		SessionUser* user = (SessionUser*)KeyValue;

		// OVERLAPPED2�� ���� send recv accept ó��
		if (bRet == TRUE)
		{
			// �б� �Ǵ� ���� �񵿱� �ϼ�
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
	// CreateIoCompletionPort : IOCP Ŀ�� ��ü �����ϰų� IOCP ����̽��� ����
	m_hIOCP = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	
	// CreateEvent : (NULL, SetEvent�� Signal ���� ���� �Ұ���?, Signal ���·� �����Ұ���?, �̺�Ʈ�̸�)
	m_hEventFinish = ::CreateEvent(0, TRUE, FALSE, 0);

	for (int iThread = 0; iThread < MAX_WORKER_THREAD; iThread++)
	{
		m_hWorkThread[iThread] =
			// _beginthreadex() : ��Ƽ �����忡 ����
			_beginthreadex(NULL, 0, WorkProc, this, 0,
				&m_iThreadID[iThread]);

	}
	return true;
}
bool	Iocp::SetBind(SOCKET sock, ULONG_PTR key)
{
	// CreateIoCompletionPort : IOCP Ŀ�� ��ü �����ϰų� IOCP ����̽��� ����
	::CreateIoCompletionPort((HANDLE)sock, m_hIOCP, key, 0);

	return true;
}
bool	Iocp::Run()
{
	return true;
}
bool	Iocp::Release()
{
	// CloseHandle : �ڵ��� ���μ����� ��������ִ°� �ƴ϶� ���� �� ���μ����� �������� �ʰڴٴ� ���̴�.
	CloseHandle(m_hIOCP);
	for (int iThread = 0; iThread < MAX_WORKER_THREAD; iThread++)
	{
		CloseHandle((HANDLE)m_hWorkThread[iThread]);
	}
	return true;
}