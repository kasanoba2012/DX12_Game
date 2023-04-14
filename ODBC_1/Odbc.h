#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <tchar.h>
#include <string>
#include <time.h>
#include <ctime>
#include <vector>
//#pragma comment(lib, "legacy_stdio_definitions.lib")
//���� Roaming::RoamingNotInitializedException C++ ���� �߻��� accessdatabaseengine_X64.exe ��ġ
using RECORD = std::vector<std::wstring>;

struct dbitem
{
	std::wstring name;
	std::wstring pass;
	int			level;
};

// 1���� �ʵ� ����
struct TColDescription
{
	SQLUSMALLINT    icol;
	SQLWCHAR		szColName[80];
	SQLSMALLINT     cchColNameMax;
	SQLSMALLINT     pcchColName;
	SQLSMALLINT     pfSqlType;
	SQLULEN			pcbColDef;
	SQLSMALLINT     pibScale;
	SQLSMALLINT     pfNullable;
	SQLWCHAR		bindData[80];
	SQLINTEGER      bindValue;
	SQLLEN			byteSize;
};

class Odbc
{
public:
	SQLHENV   g_hEnv; // ȯ���ڵ�
	SQLHDBC	  g_hDbc; // �����ڵ�
	SQLHSTMT  g_hStmt; // �����ڵ�
	SQLHSTMT  g_hReadStmt; // SQLExecute �ڵ�

	std::vector<TColDescription>	m_ColumnList;
	std::vector<RECORD>				m_dbDataList;
	std::vector<dbitem>				m_dbList;

	/// <summary>
	/// 
	/// </summary>
	SQLHSTMT  g_hSelectAllStmt; // SQLPrepare SelectAll �ڵ�
	TCHAR m_szSelectName[64] = { 0, };
	SQLINTEGER m_iDataLength;
	SQLLEN m_cbColumn;
	/// <summary>
	/// 
	/// </summary>
	SQLHSTMT  g_hInsertStmt; // SQLPrepare Insert �ڵ�
	TCHAR	   m_szInsertName[64] = { 0, };
	TCHAR	   m_szInsertPass[64] = { 0, };
	SQLINTEGER m_iSelectLevel;
	/// <summary>
	/// 
	/// </summary>
	SQLHSTMT  g_hUpdateStmt; // SQLPrepare Update �ڵ�
	TIMESTAMP_STRUCT m_ts;
	/// <summary>
	/// 
	/// </summary>
	SQLHSTMT  g_hDeleteStmt; // SQLPrepare Delete �ڵ�
	TCHAR	  m_szDeleteName[64] = { 0, };
public:
	void Init();
	bool CreatePrepare();
	void ErrorMsg();
	void Connect(std::wstring dbName);
	void DisConnect();
	bool AddSQL(dbitem& record);
	bool UpdateSQL(dbitem& record, std::wstring selectName);
	bool ReadRecord(const TCHAR* szName);
	bool DeleteSQL(const TCHAR* szName);
};