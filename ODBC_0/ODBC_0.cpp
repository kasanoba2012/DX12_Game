#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <tchar.h>
#include <string>
#include <vector>
#include "resource.h"

SQLHENV   g_hEnv; // 환경핸들
SQLHDBC	  g_hDbc; // 연결핸들
SQLHSTMT  g_hStmt; // 명령핸들

HWND g_hDlgList;  // ListBox
HWND g_hDlgName; // 첫번째 EditBox
HWND g_hDlgPrice; // 두번째 EditBox
HWND g_hDlgKorean; // 국산 여부 CheckBox



struct dbitem
{
	std::wstring name;
	int			price;
	bool		korean;
};

dbitem g_record;

std::vector<dbitem> g_dbList;

void ErrorMsg();
// Connect() 환경, 연결, 명령 핸들 초기화 및 sql에 접속
void Connect()
{
	// 환경핸들(g_hEnv), 연결핸들(g_hDbc), 명령핸들(g_hStmt)
	
	// SQLAllocHandle : 핸들의 할당 함수 
	SQLRETURN hr = SQLAllocHandle(SQL_HANDLE_ENV, // 할당하고자하는 핸들 타입
		SQL_NULL_HANDLE, // 생성할 부모 핸들 지정
		&g_hEnv // 생성할 핸들의 주소
	);

	if (hr != SQL_SUCCESS)
	{
		return;
	}

	// SQLSetEnvAttr : 환경핸들 속성 지정
	if (SQLSetEnvAttr(g_hEnv, // 환경속성 핸들
		SQL_ATTR_ODBC_VERSION, // 속성의 종류 지정
		(SQLPOINTER)SQL_OV_ODBC3, //속성 값 지정
		SQL_IS_INTEGER) != SQL_SUCCESS // 속성 값의 길이
		)

	{
		ErrorMsg();
		return;
	}

	if (hr == SQL_SUCCESS)
	{
		//연결핸들(g_hDbc)
		// SQLAllocHandle : 핸들의 할당 함수
		hr = SQLAllocHandle(SQL_HANDLE_DBC, // 할당하고자하는 핸들 타입
			g_hEnv, // 생성할 부모 핸들 지정
			&g_hDbc // 생성할 핸들의 주소
		);

		if (hr != SQL_SUCCESS)
		{
			ErrorMsg();
			return;
		}


		TCHAR  inConnect[255] = { 0, };
		TCHAR  outConnect[255] = { 0, };
		TCHAR  dir[MAX_PATH] = { 0, };
		GetCurrentDirectory(MAX_PATH, dir); // GetCurrentDirectory 현대 폴더 경로 가져오기
		// inConnect에 파일 및 경로 넣기
		_stprintf(inConnect, _T("DRIVER={Microsoft Access Driver (*.mdb, *.accdb)};DBQ=%s\\cigarette.accdb;"), dir);

		SQLSMALLINT  cbOutCon;

		// SQLDriverConnect : SQL 연결 함수
		hr = SQLDriverConnect(g_hDbc, // 연결 핸들
			NULL, // 윈도우 핸들
			inConnect, // 연결 정보 문자열
			_countof(inConnect), // 연결 문자열 길이
			outConnect,
			_countof(outConnect),
			&cbOutCon,
			SQL_DRIVER_NOPROMPT
		);
		if (hr != SQL_SUCCESS)
		{
			ErrorMsg();
			return;
		}

		//명령핸들(g_hStmt)
		// SQLAllocHandle : 핸들의 할당 함수 
		SQLAllocHandle(SQL_HANDLE_STMT, // 할당하고자하는 핸들 타입
			g_hDbc, // 생성할 부모 핸들 지정
			&g_hStmt // 생성할 핸들의 주소
		);
	}
}

// DisConnect() : 핸들 연결 해제
void DisConnect()
{
	// SQLFreeHandle : 핸들 연결 해제
	if (g_hEnv) SQLFreeHandle(SQL_HANDLE_STMT, // 연결하고자 하는 핸들 타입
		g_hEnv // 해제될 핸들
	);
	if (g_hDbc) SQLFreeHandle(SQL_HANDLE_DBC, g_hDbc);
	if (g_hStmt) SQLFreeHandle(SQL_HANDLE_ENV, g_hStmt);
}

bool AddSQL(dbitem& record);
bool UpdateSQL(dbitem& record, const TCHAR* name);

// Edit 창에서 정보 긁어오기
void GetEidtInfo()
{
	SQLTCHAR name[255] = { 0, };
	SQLTCHAR price[255] = { 0, };
	int      korean;
	GetWindowText(g_hDlgName, name, 255);
	GetWindowText(g_hDlgPrice, price, 255);
	BOOL ret = (SendMessage(g_hDlgKorean, BM_GETCHECK, 0, 0) == BST_CHECKED);
	g_record.name = name;
	g_record.price = _ttoi(price);
	g_record.korean = ret;
}

void Load()
{
	// dbList 내용 모두 지우기
	g_dbList.clear();

	// 전체 레코드 조회, 추가, 수정, 삭제
	TCHAR sql[] = L"select name, price, korean from tblCigar";
	
	TCHAR retName[25] = { 0, };
	SQLLEN  lName;
	
	int    retPrice;
	SQLLEN  lPrice;
	
	int    retKorean;
	SQLLEN  lKorean;
	
	SQLBindCol(g_hStmt, // 명령 핸들
		1, // 컬럼 번호
		SQL_UNICODE, // 바인딩 되는 변수의 데이터 타입
		retName, // 결과값이 저장될 버퍼
		_countof(retName), // 결과값 버퍼의 길이
		&lName // 컬럼의 길이나 상태를 리턴
	);
	SQLBindCol(g_hStmt, 2, SQL_INTEGER, &retPrice, 0, &lPrice);
	SQLBindCol(g_hStmt, 3, SQL_C_ULONG, &retKorean, 0, &lKorean);

	// SQLExecDirect : sql 명령 실행
	SQLRETURN hr = SQLExecDirect(g_hStmt, // 연결 핸들
		sql, // 실행할 SQL문
		SQL_NTS // SQL 문자열 길이
	);

	// SQLFetch : 데이터 다음행 집합 가져오기
	while (SQLFetch(g_hStmt) != SQL_NO_DATA)
	{
		// sql 결과문 item에 저장 후 g_dbList에 담기
		dbitem item;
		item.name = retName;
		item.price = retPrice;
		item.korean = retKorean;
		g_dbList.push_back(item);
	}
	// SQLCloseCursor : 명령핸들에 열려진 커서를 닫고 결과셋을 모두 버리는 함수
	if (g_hStmt) SQLCloseCursor(g_hStmt);
}
void SelectReadRecord()
{
	// g_hDlgList에서 선택된 항목의 인덱스 id에 저장
	int id = SendMessage(g_hDlgList, LB_GETCURSEL, 0, 0);

	if (id != -1)
	{
		SetWindowText(g_hDlgName, g_dbList[id].name.c_str());
		//SetWindowText(g_hDlgName, L"빵꾸똥꾸");
		SetWindowText(g_hDlgPrice, std::to_wstring(g_dbList[id].price).c_str());
		SendMessage(g_hDlgKorean, BM_SETCHECK,
			(g_dbList[id].korean ? BST_CHECKED : BST_UNCHECKED), 0);
	}
}

// g_hDlgList 목록 선택시 EditBox에 선택한 레코드 값 나오게 하는 함수
void ReadRecord()
{
	// g_hDlgList에서 선택된 항목의 인덱스 id에 저장
	int id = SendMessage(g_hDlgList, LB_GETCURSEL, 0, 0);

	if (id != -1)
	{
		TCHAR selectName[256] = { 0, };
		SendMessage(g_hDlgList, LB_GETTEXT, id, (LPARAM)selectName);
		TCHAR sql[256] = { 0, };
		_stprintf(sql, L"select name, price, korean from tblCigar where name='%s'", selectName);

		TCHAR retName[25] = { 0, };
		SQLLEN  lName;
		int    retPrice;
		SQLLEN  lPrice;
		int    retKorean;
		SQLLEN  lKorean;

		SQLBindCol(g_hStmt, 1, SQL_UNICODE, retName, _countof(retName), &lName);
		SQLBindCol(g_hStmt, 2, SQL_INTEGER, &retPrice, 0, &lPrice);
		SQLBindCol(g_hStmt, 3, SQL_C_ULONG, &retKorean, 0, &lKorean);

		// SQLExecDirect : sql 실행
		SQLRETURN hr = SQLExecDirect(g_hStmt, sql, SQL_NTS);

		if (hr == SQL_SUCCESS)
		{
			if (SQLFetch(g_hStmt) != SQL_NO_DATA)
			{
				SetWindowText(g_hDlgName, retName);
				SetWindowText(g_hDlgPrice, std::to_wstring(retPrice).c_str());
				SendMessage(g_hDlgKorean, BM_SETCHECK,
					(retKorean ? BST_CHECKED : BST_UNCHECKED), 0);
			}
		}
		if (g_hStmt) SQLCloseCursor(g_hStmt);
	}
}

void ErrorMsg()
{
	int value = -1;
	SQLTCHAR sqlState[10] = { 0, };
	SQLTCHAR msg[SQL_MAX_MESSAGE_LENGTH + 1] = { 0, };
	SQLTCHAR errorMsg[SQL_MAX_MESSAGE_LENGTH + 1] = { 0, };
	SQLSMALLINT msgLen;
	SQLINTEGER nativeError = 0;

	SQLRETURN hr;
	// 복합에러
	/*while (hr = SQLGetDiagRec(SQL_HANDLE_STMT, g_hStmt, value, sqlState, &nativeError, msg,
		_countof(msg), &msgLen) != SQL_NO_DATA)*/
		//단순에러
	SQLError(g_hEnv, g_hDbc, g_hStmt,
		sqlState, &nativeError, msg, SQL_MAX_MESSAGE_LENGTH + 1, &msgLen);
	{
		_stprintf(errorMsg, L"SQLSTATE:%s, 진단정보:%s, 에러코드:%d",
			sqlState, msg, nativeError);
		::MessageBox(NULL, errorMsg, L"진단정보", 0);
	}
}

bool AddSQL(dbitem& record)
{
	TCHAR sql[256] = { 0, };
	_stprintf(sql, L"insert into tblCigar (name,price,korean) values('%s',%d,%d)",
		record.name.c_str(), record.price, (int)record.korean);
	SQLRETURN hr = SQLExecDirect(g_hStmt, sql, SQL_NTS);
	if (hr != SQL_SUCCESS)
	{
		ErrorMsg();
		return false;
	}
	if (g_hStmt) SQLCloseCursor(g_hStmt);
	return true;
}

bool UpdateSQL(dbitem& record, const TCHAR* name)
{
	TCHAR sql[256] = { 0, };
	_stprintf(sql, L"update tblCigar set name='%s',price=%d,korean=%d where name='%s'",
		record.name.c_str(), record.price, (int)record.korean, name);
	SQLRETURN hr = SQLExecDirect(g_hStmt, sql, SQL_NTS);
	if (hr != SQL_SUCCESS)
	{
		ErrorMsg();
		return false;
	}
	if (g_hStmt) SQLCloseCursor(g_hStmt);
	return true;
}
void DeleteRecord()
{
	int id = SendMessage(g_hDlgList, LB_GETCURSEL, 0, 0);
	TCHAR selectName[256] = { 0, };
	if (id != -1)
	{
		SendMessage(g_hDlgList, LB_GETTEXT, id, (LPARAM)selectName);
	}
	TCHAR sql[256] = { 0, };
	_stprintf(sql, L"delete from tblCigar where name='%s'", selectName);
	SQLRETURN hr = SQLExecDirect(g_hStmt, sql, SQL_NTS);
	if (hr == SQL_SUCCESS)
	{
		if (g_hStmt) SQLCloseCursor(g_hStmt);
		SendMessage(g_hDlgList, LB_RESETCONTENT, 0, 0);
		Load();
		for (auto data : g_dbList)
		{
			SendMessage(g_hDlgList, LB_ADDSTRING, 0, (LPARAM)data.name.c_str());
		}
	}
}
void InsertRecord()
{
	GetEidtInfo();

	if (AddSQL(g_record))
	{
		SendMessage(g_hDlgList, LB_RESETCONTENT, 0, 0);
		Load();
		for (auto data : g_dbList)
		{
			SendMessage(g_hDlgList, LB_ADDSTRING, 0, (LPARAM)data.name.c_str());
		}
	}
}

void UpdateRecord()
{
	GetEidtInfo();

	TCHAR selectUpdateName[255] = { 0, };
	int id = SendMessage(g_hDlgList, LB_GETCURSEL, 0, 0);
	if (id != -1)
	{
		SendMessage(g_hDlgList, LB_GETTEXT, id, (LPARAM)selectUpdateName);
	}
	if (UpdateSQL(g_record, selectUpdateName))
	{
		SendMessage(g_hDlgList, LB_RESETCONTENT, 0, 0);
		Load();
		for (auto data : g_dbList)
		{
			SendMessage(g_hDlgList, LB_ADDSTRING, 0, (LPARAM)data.name.c_str());
		}
	}
}
LRESULT CALLBACK DlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage)
	{
	case WM_INITDIALOG:
	{
		g_hDlgList = GetDlgItem(hDlg, IDC_LIST1);
		g_hDlgName = GetDlgItem(hDlg, IDC_NAME);
		g_hDlgPrice = GetDlgItem(hDlg, IDC_PRICE);
		g_hDlgKorean = GetDlgItem(hDlg, IDC_KOREAN);
		for (auto data : g_dbList)
		{
			SendMessage(g_hDlgList, LB_ADDSTRING, 0, (LPARAM)data.name.c_str());
		}
	}return TRUE;
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDOK:  EndDialog(hDlg, IDOK); return TRUE;
		case IDCANCEL:  EndDialog(hDlg, IDOK); return TRUE;
		case IDC_DELETE: DeleteRecord(); return TRUE;
		case IDC_INSERT: InsertRecord(); return TRUE;
		case IDC_UPDATE: UpdateRecord(); return TRUE;
		case IDC_LIST1:
			switch (HIWORD(wParam))
			{
			case LBN_SELCHANGE:
				ReadRecord();
				return TRUE;
			}
		}
	}
	}
	return FALSE;
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance
	, LPSTR lpszCmdParam, int nCmdShow)
{
	Connect();
	Load();
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), HWND_DESKTOP, (DLGPROC)DlgProc);
	DisConnect();
}