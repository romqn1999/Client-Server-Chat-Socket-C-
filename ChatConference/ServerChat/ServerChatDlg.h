#define _CRT_SECURE_NO_WARNINGS
//#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#define _CRT_SECURE_NO_WARNINGS
// ServerChatDlg.h : header file
//

#pragma once

#include "odbcinst.h"
#include "afxdb.h"
#include <vector>
#include <direct.h>
//#include <stdlib.h>
//#include <stdio.h>
using namespace std;

#define PORT 25000
#define WM_SOCKET WM_USER+1
#define WM_THREAD_SENDFILE_ENDED WM_USER+2

#define FLAG_REGISTRATION 0
#define FLAG_LOGIN 1
#define FLAG_CHAT 2
#define FLAG_LOGOUT 3
#define FLAG_SELECT_CLIENT 4
#define FLAG_SEND_LIST_ONLINE 5
#define FLAG_FILEINFO 6
#define FLAG_FILEDATA 7
#define FLAG_FILELIST 8
#define FLAG_CLIENT_DOWNLOAD 9

#define MAX_BUFFER_SIZE (8 * 1024)
#define MAX_BUFF_SIZE (4 * 1024)

#define SERVER_CHAT _T("Everyone")
#define FILE_DIRECTORY "Files"

struct File
{
	CString m_FileName, m_ToWhom;
	int m_FileSize, m_SizeReceived, m_FileID;
	CFile m_destFile;
	bool m_bFileIsOpen;
};

struct SockName;

struct ThreadSendFile
{
	SockName *sn;
	CString m_FilePath;
	HWND hWnd;
	int post;
};

struct SockName
{
	SOCKET sockClient;
	bool HasUser = false;
	char Name[200];
	File *fileInfo = nullptr;
	DWORD dwSendFileThreadId;
	HANDLE hSendFileThreadId;
	ThreadSendFile sendFileStruct;

	BOOL SendFile(const CString &FileName);

	SockName &operator = (const SockName& other) {
		sockClient = other.sockClient;
		HasUser = other.HasUser;
		strcpy_s(Name, other.Name);
		fileInfo = other.fileInfo;
		return *this;
	}
};

// CServerChatDlg dialog
class CServerChatDlg : public CDialogEx
{
	// Construction
public:
	CServerChatDlg(CWnd* pParent = nullptr);	// standard constructor
	~CServerChatDlg();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SERVERCHAT_DIALOG };
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
	CDatabase m_Database;
	void InitDatabase();

	CString strDelimiter = _T("\r\n");

	LRESULT SockMsg(WPARAM wParam, LPARAM lParam);
	char* ConvertToChar(const CString &s);
	void Split(CString src, std::vector<CString> &des);
	void mSend(SOCKET sk, CString Command);
	int mRecv(SOCKET sk, CString &Command);
	int mSend(SOCKET sk, const char* buf, int len);
	int mRecv(SOCKET sk, char* buf);
	void setMessage();
	void appendOnlinesToCStr(CString &str);
	BOOL GetFileFromRemoteSender(int sockNum);
	bool openDestFile(int sockNum, const CString &fileName);
	bool closeDestFile(int sockNum);
	CString GetFileExtension(CString fileName);
	LRESULT threadEnded(WPARAM wParam, LPARAM lParam);

	SOCKET sockServer;
	struct sockaddr_in serverAdd;
	int buffLength, number_Socket, m_numFiles;
	SockName *pSock;
	std::vector<CString> strResult;
	CString Command;
	char m_Buffer[MAX_BUFFER_SIZE];

	CString m_msgString, m_Edit_OnlineList;
	afx_msg void OnBnClickedListen();
	afx_msg void OnBnClickedCancel();
};
