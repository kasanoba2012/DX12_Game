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
//참고 Roaming::RoamingNotInitializedException C++ 예외 발생시 accessdatabaseengine_X64.exe 설치
using RECORD = std::vector<std::wstring>;

struct dbitem
{
	std::wstring name;
	std::wstring pass;
	int			level;
};

// 1개의 필드 정보
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
	SQLHENV   g_hEnv; // 환경핸들
	SQLHDBC	  g_hDbc; // 연결핸들
	SQLHSTMT  g_hStmt; // 명령핸들
	SQLHSTMT  g_hReadStmt; // SQLExecute 핸들

	std::vector<TColDescription>	m_ColumnList;
	std::vector<RECORD>				m_dbDataList;
	std::vector<dbitem>				m_dbList;

	/// <summary>
	/// 
	/// </summary>
	SQLHSTMT  g_hSelectAllStmt; // SQLPrepare SelectAll 핸들
	TCHAR m_szSelectName[64] = { 0, };
	SQLINTEGER m_iDataLength;
	SQLLEN m_cbColumn;
	/// <summary>
	/// 
	/// </summary>
	SQLHSTMT  g_hInsertStmt; // SQLPrepare Insert 핸들
	TCHAR	   m_szInsertName[64] = { 0, };
	TCHAR	   m_szInsertPass[64] = { 0, };
	SQLINTEGER m_iSelectLevel;
	/// <summary>
	/// 
	/// </summary>
	SQLHSTMT  g_hUpdateStmt; // SQLPrepare Update 핸들
	TIMESTAMP_STRUCT m_ts;
	/// <summary>
	/// 
	/// </summary>
	SQLHSTMT  g_hDeleteStmt; // SQLPrepare Delete 핸들
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