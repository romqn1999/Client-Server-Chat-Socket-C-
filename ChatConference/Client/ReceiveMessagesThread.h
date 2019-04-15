#pragma once
#include <afxwin.h>
#include "ClientSocket.h"

using namespace std;

struct File
{
	CString m_FileName, m_ToWhom, m_PathFile;
	int m_FileSize, m_SizeReceived;
	CFile m_destFile;
	bool m_bFileIsOpen;
};

class ReceiveMessagesThread :
	public CWinThread
{
	DECLARE_DYNCREATE(ReceiveMessagesThread)
protected:
	virtual BOOL InitInstance();
	DECLARE_MESSAGE_MAP()

public:
	ReceiveMessagesThread();
	ReceiveMessagesThread(HWND hWnd, vector<CString> *strResult, File *fileInfo);
	~ReceiveMessagesThread();

public:
	HWND m_pMainHWnd;
	CString strDelimiter = _T("\r\n");
	vector<CString> *m_pStrResult;
	char m_Buffer[MAX_BUFF_SIZE];
	File *m_fileInfo = nullptr;

	void Split(CString src, std::vector<CString> &des);
	void SockMsg(WPARAM wParam, LPARAM lParam);
	bool openDestFile(const CString &fileName);
	bool closeDestFile();
	BOOL GetFileFromRemoteSender();
};

