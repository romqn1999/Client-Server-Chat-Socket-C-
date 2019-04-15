// ClientDlg.h : header file
//

#pragma once

#include "CLoginDlg.h"
#include "ReceiveMessagesThread.h"

using namespace std;

struct ThreadSendFile
{
	ClientSocket *sk;
	CString m_FileName, m_toUser;
	HWND hWnd;
};

// CClientDlg dialog
class CClientDlg : public CDialogEx
{
	// Construction
public:
	CClientDlg(CWnd* pParent = nullptr);	// standard constructor
	~CClientDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CLIENT_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()


public:
	LRESULT SockMsg(WPARAM wParam, LPARAM lParam);
	LRESULT TurnOnSendFileBtn(WPARAM wParam, LPARAM lParam);
	LRESULT UpdateUI(WPARAM wParam, LPARAM lParam);
	CString GetFileExtension(CString fileName);
	void setUsername(CString un) { m_Username = un; }
	void setMessage();

private:
	CComboBox m_ToBox;
	CListCtrl m_listFiles;
	CString m_Username, m_EditMessage, m_msgString;
	vector<pair<int, CString>> m_ListItemFile;
	vector<CString> strResult;
	char m_Buffer[MAX_BUFF_SIZE];

	File *m_fileInfo = nullptr;

	DWORD dwSendFileThreadId;
	HANDLE hSendFileThreadId;
	ThreadSendFile sendFileStruct;

	ReceiveMessagesThread *pReceiveThread;
public:
	afx_msg void OnBnClickedButtonSend();
	afx_msg void OnBnClickedButtonLogout();
	afx_msg void OnCbnSelchangeComboTo();
	afx_msg void OnBnClickedButtonSendfile();
	afx_msg void OnNMDblclkListFiles(NMHDR *pNMHDR, LRESULT *pResult);
};

