﻿#define _CRT_SECURE_NO_WARNINGS
#include "Odbc.h"

#include "resource.h"

Odbc  g_odbc;

HWND g_hDlgList; // ListBox

HWND g_hDlgID;
HWND g_hDlgName;
HWND g_hDlgPass;
HWND g_hDlgLevel;
HWND g_hDlgSex;
HWND g_hDlgAccount;
HWND g_hDlgLogin;
HWND g_hDlgLogout;

void SelectReadRecord()
{
	int id = SendMessage(g_hDlgList, LB_GETCURSEL, 0, 0);
	if (id != -1)
	{
		SetWindowText(g_hDlgName, g_odbc.m_dbDataList[id][1].c_str());
		SetWindowText(g_hDlgPass, g_odbc.m_dbDataList[id][2].c_str());
		SetWindowText(g_hDlgLevel, g_odbc.m_dbDataList[id][3].c_str());
	}
}
void DeleteRecord()
{
	dbitem record;
	SQLTCHAR name[255] = { 0, };
	GetWindowText(g_hDlgName, name, 255);
	if (g_odbc.DeleteSQL(name))
	{
		SendMessage(g_hDlgList, LB_RESETCONTENT, 0, 0);
		g_odbc.Load();
		for (auto data : g_odbc.m_dbDataList)
		{
			SendMessage(g_hDlgList, LB_ADDSTRING, 0, (LPARAM)data[1].c_str());
		}
	}

}
void InsertRecord()
{
	dbitem record;
	SQLTCHAR name[255] = { 0, };
	SQLTCHAR pass[255] = { 0, };
	SQLTCHAR level[255] = { 0, };
	GetWindowText(g_hDlgName, name, 255);
	GetWindowText(g_hDlgPass, pass, 255);
	GetWindowText(g_hDlgLevel, level, 255);

	record.name = name;
	record.pass = pass;
	record.level = _ttoi(level);

	if (g_odbc.AddSQL(record))
	{
		SendMessage(g_hDlgList, LB_RESETCONTENT, 0, 0);
		g_odbc.Load();
		for (auto data : g_odbc.m_dbDataList)
		{
			SendMessage(g_hDlgList, LB_ADDSTRING, 0, (LPARAM)data[1].c_str());
		}
	}
}

void UpdateRecord()
{
	dbitem record;
	SQLTCHAR name[255] = { 0, };
	SQLTCHAR pass[255] = { 0, };
	SQLTCHAR level[255] = { 0, };
	SQLTCHAR sex[255] = { 0, };
	GetWindowText(g_hDlgName, name, 255);
	GetWindowText(g_hDlgPass, pass, 255);
	GetWindowText(g_hDlgLevel, level, 255);
	GetWindowText(g_hDlgSex, sex, 255);

	record.name = name;
	record.pass = pass;
	record.level = _ttol(level);
	record.sex = _ttol(sex);

	TCHAR selectUpdateName[255] = { 0, };
	int id = SendMessage(g_hDlgList, LB_GETCURSEL, 0, 0);
	if (id != -1)
	{
		SendMessage(g_hDlgList, LB_GETTEXT, id, (LPARAM)selectUpdateName);
	}
	if (g_odbc.UpdateSQL(record, selectUpdateName))
	{
		SendMessage(g_hDlgList, LB_RESETCONTENT, 0, 0);
		g_odbc.Load();
		for (auto data : g_odbc.m_dbDataList)
		{
			SendMessage(g_hDlgList, LB_ADDSTRING, 0, (LPARAM)data[1].c_str());
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
		g_hDlgID = GetDlgItem(hDlg, IDC_ID);
		g_hDlgName = GetDlgItem(hDlg, IDC_NAME2);
		g_hDlgPass = GetDlgItem(hDlg, IDC_PASS);
		g_hDlgLevel = GetDlgItem(hDlg, IDC_LEVEL);
		g_hDlgSex = GetDlgItem(hDlg, IDC_SEX);
		g_hDlgAccount = GetDlgItem(hDlg, IDC_ACCOUNTTIME);
		g_hDlgLogin = GetDlgItem(hDlg, IDC_LOGINTIME);
		g_hDlgLogout = GetDlgItem(hDlg, IDC_LOGOUTTIME);

		for (auto rec : g_odbc.m_dbDataList)
		{
			SendMessage(g_hDlgList, LB_ADDSTRING, 0, (LPARAM)rec[1].c_str());
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
			{
				int id = SendMessage(g_hDlgList, LB_GETCURSEL, 0, 0);
				if (id != -1)
				{
					TCHAR selectName[256] = { 0, };
					SendMessage(g_hDlgList, LB_GETTEXT, id, (LPARAM)selectName);

					// 현재 시간을 스트링화
					std::time_t now = std::time(NULL); // 1970,01,01, 0시
					std::tm* ptm = std::localtime(&now);
					std::string timestring = std::asctime(ptm);
					char buffer[255] = { 0, };
					std::strftime(buffer, 255, "%a, %d.%m.%Y %H:%M:%S", ptm);

					// 100일 경과
					//std::time_t difftime = std::time(NULL);
					//difftime += 1 * 24 * 60 * 60;
					//int iDiff = std::difftime(difftime, now);

					// Main List 화면 변경 하는곳
					if (g_odbc.ReadRecord(selectName))
					{
						SetWindowText(g_hDlgID, std::to_wstring(g_odbc.retID).c_str());
						SetWindowText(g_hDlgName, g_odbc.retName);
						SetWindowText(g_hDlgPass, g_odbc.retPass);
						SetWindowText(g_hDlgLevel, std::to_wstring(g_odbc.retLevel).c_str());
						SendMessage(g_hDlgSex, BM_SETCHECK, ((g_odbc.retSex) ? BST_CHECKED : BST_UNCHECKED), 0);
						std::wstring account;
						account += std::to_wstring(g_odbc.accountTS.year); account += L"년";
						account += std::to_wstring(g_odbc.accountTS.month); account += L"월";
						account += std::to_wstring(g_odbc.accountTS.day); account += L"일";
						account += std::to_wstring(g_odbc.accountTS.hour); account += L"시";
						account += std::to_wstring(g_odbc.accountTS.minute); account += L"분";
						account += std::to_wstring(g_odbc.accountTS.second); account += L"초";
						SetWindowText(g_hDlgAccount, account.c_str());

						std::wstring login;
						login += std::to_wstring(g_odbc.loginTS.year); login += L"년";
						login += std::to_wstring(g_odbc.loginTS.month); login += L"월";
						login += std::to_wstring(g_odbc.loginTS.day); login += L"일";
						login += std::to_wstring(g_odbc.loginTS.hour); login += L"시";
						login += std::to_wstring(g_odbc.loginTS.minute); login += L"분";
						login += std::to_wstring(g_odbc.loginTS.second); login += L"초";
						SetWindowText(g_hDlgLogin, login.c_str());

						std::wstring logout;
						logout += std::to_wstring(g_odbc.logoutTS.year); logout += L"년";
						logout += std::to_wstring(g_odbc.logoutTS.month); logout += L"월";
						logout += std::to_wstring(g_odbc.logoutTS.day); logout += L"일";
						logout += std::to_wstring(g_odbc.logoutTS.hour); logout += L"시";
						logout += std::to_wstring(g_odbc.logoutTS.minute); logout += L"분";
						logout += std::to_wstring(g_odbc.logoutTS.second); logout += L"초";
						SetWindowText(g_hDlgLogout, logout.c_str());
					}
					return TRUE;
				}
			}
			}
		}
	}
	}
	return FALSE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	g_odbc.Init();
	//g_odbc.Connect(L"../data/db/Account.accdb");
	g_odbc.ConnetMssql(L"odbc_test.dsn");
	g_odbc.Load();
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), HWND_DESKTOP, (DLGPROC)DlgProc);
	g_odbc.DisConnect();
}