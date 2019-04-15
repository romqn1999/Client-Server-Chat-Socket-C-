
#pragma once
#include <vector>

#define PORT 25000
#define WM_SOCKET_READ WM_USER+1
#define WM_SOCKET WM_USER+2
#define WM_SOCKET_LOGIN WM_USER+3
#define WM_SOCKET_CLIENT WM_USER+4
#define WM_THREAD_FILE_ENDED WM_USER+5

#define FLAG_REGISTRATION 0
#define FLAG_LOGIN 1
#define FLAG_MESSAGE 2
#define FLAG_LOGOUT 3
#define FLAG_TO_WHOM 4
#define FLAG_ONLINE_LIST 5
#define FLAG_FILEINFO 6
#define FLAG_FILEDATA 7
#define FLAG_FILELIST 8
#define FLAG_CLIENT_DOWNLOAD 9
#define SERVER_CHAT _T("Everyone")

#define MAX_BUFF_SIZE (4 * 1024)

class ClientSocket : public CWnd
{
private:
	CString strDelimiter = _T("\r\n");
	SOCKET sClient;
	sockaddr_in servAdd;
	CString Command;
	int	buffLength;
	//CString strResult[2];
	std::vector<CString> strResult;
	CString userOnline;
	int level;
	int isLogon = 0;

	CString m_msgString;
	CString IP = _T("127.0.0.1");
	CString m_userName;
protected:
	//DECLARE_MESSAGE_MAP()
public:
	ClientSocket();
	~ClientSocket();
	SOCKET GetSocket() { return sClient; }
	int SetHWND(HWND hWnd) { return WSAAsyncSelect(sClient, hWnd, WM_SOCKET, FD_READ | FD_CLOSE); }
	int Connect();
	//LRESULT SockMsg(WPARAM wParam, LPARAM lParam);
	char* ConvertToChar(const CString &s);
	void Split(CString src, CString des[2]);
	void Split(CString src, std::vector<CString> &des);
	void mSend(CString Command);
	int mRecv(CString &Command);
	BOOL SendFile(const CString &FileName, const CString &to);
	bool islogon() { return isLogon; }
	void Close();
	void changeIP(CString newIp) { IP = newIp; }
	int mRecv(char* buf);
};

