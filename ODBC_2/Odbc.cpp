#include "Odbc.h"

void Odbc::Init()
{
	// ȯ���ڵ�(g_hEnv), �����ڵ�(g_hDbc), ����ڵ�(g_hStmt)

	// SQLAllocHandle : �ڵ��� �Ҵ� �Լ� 
	SQLRETURN hr = SQLAllocHandle(SQL_HANDLE_ENV, // �Ҵ��ϰ����ϴ� �ڵ� Ÿ��
		SQL_NULL_HANDLE, // ������ �θ� �ڵ� ����
		&g_hEnv // ������ �ڵ��� �ּ�
	);
	if (hr != SQL_SUCCESS)
	{
		return;
	}

	// SQLSetEnvAttr : ȯ���ڵ� �Ӽ� ����
	if (SQLSetEnvAttr(g_hEnv, // ȯ��Ӽ� �ڵ�
		SQL_ATTR_ODBC_VERSION, // �Ӽ��� ���� ����
		(SQLPOINTER)SQL_OV_ODBC3_80, //�Ӽ� �� ����
		SQL_IS_INTEGER) != SQL_SUCCESS // �Ӽ� ���� ����
		)
	{
		ErrorMsg();
		return;
	}

	if (hr == SQL_SUCCESS)
	{
		//�����ڵ�(g_hDbc)
		// SQLAllocHandle : �ڵ��� �Ҵ� �Լ�
		hr = SQLAllocHandle(SQL_HANDLE_DBC, // �Ҵ��ϰ����ϴ� �ڵ� Ÿ��
			g_hEnv, // ������ �θ� �ڵ� ����
			&g_hDbc // ������ �ڵ��� �ּ�
		);
		if (hr != SQL_SUCCESS)
		{
			ErrorMsg();
			return;
		}
	}
}

bool Odbc::CreatePrepare()
{
	// Select All
	SQLRETURN ret;
	std::wstring sql = L"select * from user_table";
	// SQLAllocHandle : �ڵ��� �Ҵ� �Լ�
	ret = SQLAllocHandle(SQL_HANDLE_STMT,  // �Ҵ��ϰ����ϴ� �ڵ� Ÿ��
		g_hDbc, // ������ �θ� �ڵ� ����
		&g_hSelectAllStmt // ������ �ڵ��� �ּ�
	);

	// SQLPrepare �Ķ���͸� �ٲ㼭 sql ����
	ret = SQLPrepare(g_hSelectAllStmt, // Handle
		(SQLTCHAR*)sql.c_str(), // ������ sql ��
		SQL_NTS // TextLength?
	);

	if (ret != SQL_SUCCESS)
	{
		ErrorMsg();
		return false;
	}

	// Select where
	std::wstring sql2 = L"select * from user_table where Name=?";
	// SQLAllocHandle : �ڵ��� �Ҵ� �Լ�
	ret = SQLAllocHandle(SQL_HANDLE_STMT, // �Ҵ��ϰ����ϴ� �ڵ� Ÿ��
		g_hDbc, // ������ �θ� �ڵ� ����
		&g_hReadStmt // ������ �ڵ��� �ּ�
	);
	// SQLPrepare �Ķ���͸� �ٲ㼭 sql ����
	ret = SQLPrepare(g_hReadStmt, (SQLTCHAR*)sql2.c_str(), SQL_NTS);
	if (ret != SQL_SUCCESS)
	{
		ErrorMsg();
		return false;
	}

	m_iDataLength = sizeof(m_szSelectName);
	m_cbColumn = SQL_NTS;

	// ?�� �Ķ���� ��Ŀ��� �ϸ�, ������ ���ε��� ���������� ��ü ? ��ŭ SQLBindParameter �Ҵ� �ؾ� �Ѵ�.
	ret = SQLBindParameter(g_hReadStmt, // ����ڵ�
		1, // �Ķ���� ��ȣ
		SQL_PARAM_INPUT, // �Ķ���� �뵵
		SQL_UNICODE, // �Ķ���� ������ Ÿ��(C��)
		SQL_CHAR, // �Ķ���� ������ Ÿ��(SQL��)
		m_iDataLength, // �Ķ���� ������ ũ��
		0, // �Ķ���� ������ �ڸ���
		m_szSelectName, // ���� �Ķ���Ϳ� ����� ������ �ּ�
		m_iDataLength, // �Ķ������ ���ڿ��̳� �������϶� ������ ũ��
		&m_cbColumn // ���̿� ���� �μ�
	);
	if (ret != SQL_SUCCESS)
	{
		ErrorMsg();
		return false;
	}

	// Insert
	TCHAR sql3[256] = L"insert into user_table (Name,Pass,GameLevel) values(?,?,?)";
	
	// SQLAllocHandle : �ڵ��� �Ҵ� �Լ�
	ret = SQLAllocHandle(SQL_HANDLE_STMT, // �Ҵ��ϰ����ϴ� �ڵ� Ÿ��
		g_hDbc, // ������ �θ� �ڵ� ����
		&g_hInsertStmt // ������ �ڵ��� �ּ�
	);
	// SQLPrepare �Ķ���͸� �ٲ㼭 sql ����
	ret = SQLPrepare(g_hInsertStmt, (SQLTCHAR*)sql3, SQL_NTS);
	if (ret != SQL_SUCCESS)
	{
		ErrorMsg();
		return false;
	}

	m_iDataLength = sizeof(m_szInsertName);
	m_cbColumn = SQL_NTS;

	// ?�� �Ķ���� ��Ŀ��� �ϸ�, ������ ���ε��� ���������� ��ü ? ��ŭ SQLBindParameter �Ҵ� �ؾ� �Ѵ�.
	ret = SQLBindParameter(g_hInsertStmt, 1, SQL_PARAM_INPUT, SQL_UNICODE, SQL_CHAR,
		m_iDataLength, 0, m_szInsertName,
		m_iDataLength, &m_cbColumn);
	if (ret != SQL_SUCCESS)
	{
		ErrorMsg();
		return false;
	}
	ret = SQLBindParameter(g_hInsertStmt, 2, SQL_PARAM_INPUT, SQL_UNICODE, SQL_CHAR,
		m_iDataLength, 0, m_szInsertPass,
		m_iDataLength, &m_cbColumn);
	if (ret != SQL_SUCCESS)
	{
		ErrorMsg();
		return false;
	}
	ret = SQLBindParameter(g_hInsertStmt, 3, SQL_PARAM_INPUT, SQL_INTEGER, SQL_INTEGER,
		0, 0, &m_iSelectLevel,
		0, &m_cbColumn);
	if (ret != SQL_SUCCESS)
	{
		ErrorMsg();
		return false;
	}


	// Update
	TCHAR sql4[256] = L"update user_table set Name=?,Pass=?,GameLevel=?,LoginTime=? where Name=?";

	// SQLAllocHandle : �ڵ��� �Ҵ� �Լ�
	ret = SQLAllocHandle(SQL_HANDLE_STMT,  // �Ҵ��ϰ����ϴ� �ڵ� Ÿ��
		g_hDbc, // ������ �θ� �ڵ� ����
		&g_hUpdateStmt // ������ �ڵ��� �ּ�
	);
	// SQLPrepare �Ķ���͸� �ٲ㼭 sql ����
	ret = SQLPrepare(g_hUpdateStmt, (SQLTCHAR*)sql4, SQL_NTS);
	if (ret != SQL_SUCCESS)
	{
		ErrorMsg();
		return false;
	}

	m_iDataLength = sizeof(m_szInsertName);
	m_cbColumn = SQL_NTS;
	::ZeroMemory(&m_ts, sizeof(m_ts));

	// ?�� �Ķ���� ��Ŀ��� �ϸ�, ������ ���ε��� ���������� ��ü ? ��ŭ SQLBindParameter �Ҵ� �ؾ� �Ѵ�.
	ret = SQLBindParameter(g_hUpdateStmt, 1, SQL_PARAM_INPUT, SQL_UNICODE, SQL_CHAR,
		m_iDataLength, 0, m_szInsertName,
		m_iDataLength, &m_cbColumn);
	if (ret != SQL_SUCCESS)
	{
		ErrorMsg();
		return false;
	}
	ret = SQLBindParameter(g_hUpdateStmt, 2, SQL_PARAM_INPUT, SQL_UNICODE, SQL_CHAR,
		m_iDataLength, 0, m_szInsertPass,
		m_iDataLength, &m_cbColumn);
	if (ret != SQL_SUCCESS)
	{
		ErrorMsg();
		return false;
	}
	ret = SQLBindParameter(g_hUpdateStmt, 3, SQL_PARAM_INPUT, SQL_INTEGER, SQL_INTEGER,
		0, 0, &m_iSelectLevel,
		0, &m_cbColumn);
	if (ret != SQL_SUCCESS)
	{
		ErrorMsg();
		return false;
	}
	ret = SQLBindParameter(g_hUpdateStmt, 4, SQL_PARAM_INPUT, SQL_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP,
		SQL_TIMESTAMP_LEN, 0, &m_ts,
		sizeof(TIMESTAMP_STRUCT), &m_cbColumn);
	if (ret != SQL_SUCCESS)
	{
		ErrorMsg();
		return false;
	}
	ret = SQLBindParameter(g_hUpdateStmt, 5, SQL_PARAM_INPUT, SQL_UNICODE, SQL_CHAR,
		m_iDataLength, 0, m_szSelectName,
		m_iDataLength, &m_cbColumn);
	if (ret != SQL_SUCCESS)
	{
		ErrorMsg();
		return false;
	}

	// Delete
	std::wstring sql5 = L"delete from user_table where Name=?";
	// SQLAllocHandle : �ڵ��� �Ҵ� �Լ�
	ret = SQLAllocHandle(SQL_HANDLE_STMT,  // �Ҵ��ϰ����ϴ� �ڵ� Ÿ��
		g_hDbc, // ������ �θ� �ڵ� ����
		&g_hDeleteStmt // ������ �ڵ��� �ּ�
	);

	// SQLPrepare �Ķ���͸� �ٲ㼭 sql ����
	ret = SQLPrepare(g_hDeleteStmt, // Handle
		(SQLTCHAR*)sql5.c_str(), // ������ sql ��
		SQL_NTS // TextLength?
	);

	m_iDataLength = sizeof(m_szDeleteName);
	m_cbColumn = SQL_NTS;

	// ?�� �Ķ���� ��Ŀ��� �ϸ�, ������ ���ε��� ���������� ��ü ? ��ŭ SQLBindParameter �Ҵ� �ؾ� �Ѵ�.
	ret = SQLBindParameter(g_hDeleteStmt, // ����ڵ�
		1, // �Ķ���� ��ȣ
		SQL_PARAM_INPUT, // �Ķ���� �뵵
		SQL_UNICODE, // �Ķ���� ������ Ÿ��(C��)
		SQL_CHAR, // �Ķ���� ������ Ÿ��(SQL��)
		m_iDataLength, // �Ķ���� ������ ũ��
		0, // �Ķ���� ������ �ڸ���
		m_szDeleteName, // ���� �Ķ���Ϳ� ����� ������ �ּ�
		m_iDataLength, // �Ķ������ ���ڿ��̳� �������϶� ������ ũ��
		&m_cbColumn // ���̿� ���� �μ�
	);
	if (ret != SQL_SUCCESS)
	{
		ErrorMsg();
		return false;
	}

	return true;
}

void Odbc::ErrorMsg()
{
	int value = -1;
	SQLTCHAR sqlState[10] = { 0, };
	SQLTCHAR msg[SQL_MAX_MESSAGE_LENGTH + 1] = { 0, };
	SQLTCHAR errorMsg[SQL_MAX_MESSAGE_LENGTH + 1] = { 0, };
	SQLSMALLINT msgLen;
	SQLINTEGER nativeError = 0;

	SQLRETURN hr;
	// ���տ���
	/*while (hr = SQLGetDiagRec(SQL_HANDLE_STMT, g_hStmt, value, sqlState, &nativeError, msg,
		_countof(msg), &msgLen) != SQL_NO_DATA)*/
		//�ܼ�����
	SQLError(g_hEnv, g_hDbc, g_hStmt,
		sqlState, &nativeError, msg, SQL_MAX_MESSAGE_LENGTH + 1, &msgLen);
	{
		_stprintf(errorMsg, L"SQLSTATE:%s, ��������:%s, �����ڵ�:%d",
			sqlState, msg, nativeError);
		::MessageBox(NULL, errorMsg, L"��������", 0);
	}
}

void Odbc::Connect(std::wstring dbName)
{
	TCHAR  inConnect[255] = { 0, };
	TCHAR  outConnect[255] = { 0, };
	_stprintf(inConnect, _T("DRIVER={Microsoft Access Driver (*.mdb, *.accdb)};DBQ=%s;"), dbName.c_str());

	SQLSMALLINT  cbOutCon;
	SQLRETURN hr = SQLDriverConnect(g_hDbc, NULL, inConnect, _countof(inConnect),
		outConnect, _countof(outConnect),
		&cbOutCon, SQL_DRIVER_NOPROMPT);
	if (hr != SQL_SUCCESS && hr != SQL_SUCCESS_WITH_INFO)
	{
		ErrorMsg();
		return;
	}

	//����ڵ�(g_hStmt)
	// SQLSetEnvAttr : ȯ���ڵ� �Ӽ� ����
	SQLAllocHandle(SQL_HANDLE_STMT, // �Ҵ��ϰ����ϴ� �ڵ� Ÿ��
		g_hDbc, // ������ �θ� �ڵ� ����
		&g_hStmt // ������ �ڵ��� �ּ�
	);

	CreatePrepare();
}

void Odbc::ConnetMssql(std::wstring dbName)
{
	TCHAR  inConnect[255] = { 0, };
	TCHAR  outConnect[255] = { 0, };
	TCHAR  dir[MAX_PATH] = { 0, };
	GetCurrentDirectory(MAX_PATH, dir);

	TCHAR dsn[] = L"Driver={SQL Server};Server=shader.kr;Address=127.0.01,1433;Network=dbmssocn;Database=test_DB;Uid=sa;Pwd=dpsjwl2012;";
	SQLSMALLINT  cbOutCon;
	SQLRETURN hr = SQLDriverConnect(g_hDbc, NULL, dsn, _countof(dsn),
		outConnect, _countof(outConnect),
		&cbOutCon, SQL_DRIVER_NOPROMPT);

	// Todo �� ���� �̰ɷ� �����ϸ� �ȵ�?....
	//SQLRETURN hr = SQLConnect(g_hDbc,
	//	(SQLTCHAR*)L"test_DB", SQL_NTS,
	//	(SQLTCHAR*)L"sa", SQL_NTS,
	//	(SQLTCHAR*)L"dpsjwl2012", SQL_NTS);

	if (hr != SQL_SUCCESS && hr != SQL_SUCCESS_WITH_INFO)
	{
		ErrorMsg();
		return;
	}

	//����ڵ�(g_hStmt)
	SQLAllocHandle(SQL_HANDLE_STMT, g_hDbc, &g_hStmt);
	CreatePrepare();
}

void Odbc::DisConnect()
{
	if (g_hStmt) SQLFreeHandle(SQL_HANDLE_STMT, g_hStmt);
	if (g_hSelectAllStmt) SQLFreeHandle(SQL_HANDLE_STMT, g_hSelectAllStmt);
	if (g_hInsertStmt) SQLFreeHandle(SQL_HANDLE_STMT, g_hInsertStmt);
	if (g_hUpdateStmt) SQLFreeHandle(SQL_HANDLE_STMT, g_hUpdateStmt);
	if (g_hDeleteStmt) SQLFreeHandle(SQL_HANDLE_STMT, g_hDeleteStmt);

	if (g_hDbc) SQLFreeHandle(SQL_HANDLE_DBC, g_hDbc);
	if (g_hEnv) SQLFreeHandle(SQL_HANDLE_ENV, g_hEnv);
}

bool Odbc::AddSQL(dbitem& record)
{
	ZeroMemory(m_szInsertName, sizeof(m_szInsertName));
	CopyMemory(m_szInsertName, record.name.c_str(), record.name.size() * sizeof(TCHAR));
	ZeroMemory(m_szInsertPass, sizeof(m_szInsertPass));
	CopyMemory(m_szInsertPass, record.pass.c_str(), record.pass.size() * sizeof(TCHAR));

	m_iSelectLevel = record.level;

	// SQLExecute : �غ�� �Ķ���͸� ��ü�ؼ� �ٷ� �����Ѵ�
	SQLRETURN hr = SQLExecute(g_hInsertStmt);
	if (hr != SQL_SUCCESS)
	{
		ErrorMsg();
		return false;
	}
	if (g_hInsertStmt) SQLCloseCursor(g_hInsertStmt);
	// SQLFreeStmt : �ڵ�� ���õ� ��� �ڿ��� ����
	SQLFreeStmt(g_hInsertStmt, SQL_CLOSE);
	return true;
}

bool Odbc::UpdateSQL(dbitem& record, std::wstring selectName)
{
	ZeroMemory(m_szSelectName, sizeof(m_szSelectName));
	CopyMemory(m_szSelectName, selectName.c_str(), selectName.size() * sizeof(TCHAR));

	ZeroMemory(m_szInsertName, sizeof(m_szInsertName));
	CopyMemory(m_szInsertName, record.name.c_str(), record.name.size() * sizeof(TCHAR));

	ZeroMemory(m_szInsertPass, sizeof(m_szInsertPass));
	CopyMemory(m_szInsertPass, record.pass.c_str(), record.pass.size() * sizeof(TCHAR));

	m_iSelectLevel = record.level;

	std::time_t now = std::time(NULL); // 1970,01,01, 0��
	std::tm* ptm = std::localtime(&now);

	m_ts.year = ptm->tm_year + 1900;
	m_ts.month = ptm->tm_mon + 1;
	m_ts.day = ptm->tm_mday;
	m_ts.hour = ptm->tm_hour;
	m_ts.minute = ptm->tm_min;
	m_ts.second = ptm->tm_sec;
	m_ts.fraction = 0;

	// SQLExecute : �غ�� �Ķ���͸� ��ü�ؼ� �ٷ� �����Ѵ�
	SQLRETURN hr = SQLExecute(g_hUpdateStmt);

	if (hr != SQL_SUCCESS)
	{
		ErrorMsg();
		return false;
	}
	if (g_hUpdateStmt) SQLCloseCursor(g_hUpdateStmt);
	SQLFreeStmt(g_hUpdateStmt, SQL_CLOSE);
	return true;
}

bool Odbc::ReadRecord(const TCHAR* szName)
{
	if (szName != nullptr)
	{
		//ZeroMemory(m_szSelectName, sizeof(m_szSelectName));
		//CopyMemory(m_szSelectName, szName, sizeof(szName));
		ZeroMemory(m_szSelectName, sizeof(TCHAR) * 64);
		CopyMemory(m_szSelectName, szName, _tcslen(szName));

		// SQLExecute : �غ�� �Ķ���͸� ��ü�ؼ� �ٷ� �����Ѵ�
		SQLRETURN hr = SQLExecute(g_hReadStmt);

		if (hr == SQL_SUCCESS)
		{
			// SQLFetch ��� ���տ��� ���ε��� �����͸� ��ȯ
			if (SQLFetch(g_hReadStmt) != SQL_NO_DATA)
			{
				if (g_hReadStmt) SQLCloseCursor(g_hReadStmt);
				SQLFreeStmt(g_hReadStmt, SQL_CLOSE);
				return true;
			}
		}
	}

	ErrorMsg();
	if (g_hReadStmt) SQLCloseCursor(g_hReadStmt);
	SQLFreeStmt(g_hReadStmt, SQL_CLOSE);
	return false;
}

bool Odbc::DeleteSQL(const TCHAR* szName)
{
	if (szName != nullptr)
	{
		ZeroMemory(m_szDeleteName, sizeof(m_szDeleteName));
		CopyMemory(m_szDeleteName, szName, sizeof(szName));

		// SQLExecute : �غ�� �Ķ���͸� ��ü�ؼ� �ٷ� �����Ѵ�
		SQLRETURN hr = SQLExecute(g_hDeleteStmt);

		if (hr == SQL_SUCCESS)
		{
			// SQLFetch ��� ���տ��� ���ε��� �����͸� ��ȯ
			if (SQLFetch(g_hDeleteStmt) != SQL_NO_DATA)
			{
				if (g_hDeleteStmt) SQLCloseCursor(g_hDeleteStmt);
				SQLFreeStmt(g_hDeleteStmt, SQL_CLOSE);
				return true;
			}
		}
	}

	ErrorMsg();
	if (g_hDeleteStmt) SQLCloseCursor(g_hDeleteStmt);
	SQLFreeStmt(g_hDeleteStmt, SQL_CLOSE);
	return false;

}