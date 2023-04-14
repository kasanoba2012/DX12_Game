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
#include <iostream>
//#pragma comment(lib, "legacy_stdio_definitions.lib")
//���� Roaming::RoamingNotInitializedException C++ ���� �߻��� accessdatabaseengine_X64.exe ��ġ
using RECORD = std::vector<std::wstring>;

struct dbitem
{
	std::wstring name;
	std::wstring pass;
	int			level;
	int			sex;
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
	SWORD sRet;
	SQLINTEGER cbRet;

	SQLHENV   g_hEnv; // ȯ���ڵ�
	SQLHDBC	  g_hDbc; // �����ڵ�
	SQLHSTMT  g_hStmt; // �����ڵ�
	SQLHSTMT  g_hReadStmt; // SQLExecute �ڵ�
	SQLHSTMT  g_hDeleteStmt; // SQLPrepare Delete �ڵ�
	SQLHSTMT  g_hLoginCheckStmt; // SQLPrepare LoginCheck �ڵ�


	std::vector<TColDescription>	m_ColumnList;
	std::vector<RECORD>				m_dbDataList;
	std::vector<dbitem>				m_dbList;
	TCHAR m_szSelectName[10] = { 0, };

	/// <summary>
	/// 
	/// </summary>
	SQLHSTMT    g_hSelectAllStmt;
	TCHAR		m_szReadName[20] = { 0, };
	SQLINTEGER  m_iDataLength;
	SQLLEN      m_cbColumn;
	void Load();
	/// <summary>
	/// 
	/// </summary>
	SQLHSTMT  g_hInsertStmt; // SQLPrepare Insert �ڵ�
	TCHAR	   m_szInsertName[20] = { 0, };
	TCHAR	   m_szTestInsertName[20] = { 0, };
	TCHAR	   m_szInsertPass[20] = { 0, };
	SQLINTEGER m_iSelectLevel;
	/// <summary>
	/// 
	/// </summary>
	SQLHSTMT  g_hUpdateStmt; // SQLPrepare Update �ڵ�
	TCHAR	   m_szUpdateName[20] = { 0, };
	TCHAR	   m_szUpdatePass[20] = { 0, };
	SQLINTEGER m_iUpdateLevel;
	SQLINTEGER m_iUpdateSex;
	TIMESTAMP_STRUCT m_ts;
	/// <summary>
	/// 
	/// </summary>
	SQLHSTMT  g_hPassStmt;
	TCHAR	  m_szDeleteName[64] = { 0, };

	// LoginCheck
	TCHAR	   m_szLoginId[10] = { 0, };
	TCHAR	   m_szLoginPw[10] = { 0, };
	SQLINTEGER  m_iLoginIdDataLength;
	SQLINTEGER  m_iLoginPwDataLength;

public:
	void Init();
	bool CreatePrepare();
	void ErrorMsg(SQLHSTMT  stmt);
	void Connect(std::wstring dbName);
	void ConnetMssql(std::wstring dbName);
	void DisConnect();
	bool AddSQL(dbitem& record);
	bool UpdateSQL(dbitem& record, std::wstring selectName);
	bool ReadRecord(std::wstring selectName);
	bool DeleteSQL(const TCHAR* szName);
	bool LoginCheckSQL(const TCHAR* szName, const TCHAR* szPw);
public:
	bool CreateUserAllSelect();
	bool CreateSelectWhereName();
	bool CreateInsertAccount();
	bool CreateUpdate();
	bool DeleteLoingInfo();
	bool LoginCheck();
	

public:
	int    retID;					SQLLEN  lID;
	TCHAR  retName[25] = { 0, };	SQLLEN  lName;
	TCHAR  retPass[25] = { 0, };	SQLLEN  lPass;
	int    retLevel;				SQLLEN  lLevel;
	int    retSex;					SQLLEN  lSex;
	TIMESTAMP_STRUCT accountTS;		SQLLEN  lAccount;
	TIMESTAMP_STRUCT loginTS;		SQLLEN  llogin;
	TIMESTAMP_STRUCT logoutTS;		SQLLEN  llogout;
};