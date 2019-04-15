#pragma once

#include "CNewAccountDlg.h"
#include "ClientDlg.h"

// CLoginDlg dialog

class CLoginDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CLoginDlg)

public:
	CLoginDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CLoginDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LOGIN_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	LRESULT SockMsg(WPARAM wParam, LPARAM lParam);
	void Split(CString src, std::vector<CString> &des);
	CString strDelimiter = _T("\r\n");
	std::vector<CString> strResult;
	CEdit m_EditUsername;
	CString m_Username;
	CString m_Password;
	afx_msg void OnBnClickedNewaccountBtn();
	afx_msg void OnBnClickedOk();
	CString m_serverIP;
	afx_msg void OnEnChangeEditIp();
};
