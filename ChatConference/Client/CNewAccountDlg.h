#pragma once


// CNewAccountDlg dialog

class CNewAccountDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CNewAccountDlg)

public:
	CNewAccountDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CNewAccountDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_NEWACCOUNT_DLG };
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
	CEdit m_EditPassword;
	CString m_Password;
	CString m_ConfirmPassword;
	void changeIP(CString newIP);
	afx_msg void OnBnClickedOk();
private:
	CString m_serverIP;
public:
	afx_msg void OnEnChangeEditIp();
};
