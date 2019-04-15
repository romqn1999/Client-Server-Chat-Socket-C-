

// ServerChatDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ServerChat.h"
#include "ServerChatDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CServerChatDlg dialog



CServerChatDlg::CServerChatDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SERVERCHAT_DIALOG, pParent)
	, m_msgString(_T(""))
	, m_Edit_OnlineList(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}


void CServerChatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_BOXCHAT, m_msgString);
	DDX_Text(pDX, IDC_EDIT_ONLINE_LIST, m_Edit_OnlineList);
}

BEGIN_MESSAGE_MAP(CServerChatDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_SOCKET, SockMsg)
	ON_MESSAGE(WM_THREAD_SENDFILE_ENDED, threadEnded)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_LISTEN, &CServerChatDlg::OnBnClickedListen)
	ON_BN_CLICKED(IDC_CANCEL, &CServerChatDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CServerChatDlg message handlers

BOOL CServerChatDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	InitDatabase();

	CString strSQLQuery = _T("Delete from [ONLINE]");
	m_Database.ExecuteSQL(strSQLQuery);

	strSQLQuery = _T("Delete from [MESSAGE]");
	m_Database.ExecuteSQL(strSQLQuery);

	strSQLQuery = _T("Delete from [FILE]");
	m_Database.ExecuteSQL(strSQLQuery);

	_mkdir(FILE_DIRECTORY);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CServerChatDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CServerChatDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CServerChatDlg::InitDatabase() {
	TRY{
		// Open the database
		m_Database.Open(NULL);
	}CATCH(CDBException, e) {
		// If a database exception occured, show error msg
		AfxMessageBox(L"Database error: " + e->m_strError);
	}
	END_CATCH;
}

CServerChatDlg::~CServerChatDlg() {
	delete[]pSock;
	m_Database.Close();

	for (int i = 0; i < number_Socket; ++i)
		if (pSock[i].fileInfo != nullptr)
			delete pSock[i].fileInfo;
}

void CServerChatDlg::Split(CString src, std::vector<CString> &des)
{
	int start = 0, pos = 0;
	des.clear();

	while ((pos = src.Find(strDelimiter, start)) != -1) {
		des.push_back(src.Mid(start, pos - start));
		start = pos + strDelimiter.GetLength();
	}

}

char* CServerChatDlg::ConvertToChar(const CString &s)
{
	int nSize = s.GetLength();
	char *pAnsiString = new char[nSize + 1];
	memset(pAnsiString, 0, nSize + 1);
	wcstombs(pAnsiString, s, nSize + 1);
	return pAnsiString;
}

void CServerChatDlg::mSend(SOCKET sk, CString Command)
{
	/*int Len = Command.GetLength();
	Len += Len;
	PBYTE sendBuff = new BYTE[1000];
	memset(sendBuff, 0, 1000);
	memcpy(sendBuff, (PBYTE)(LPCTSTR)Command, Len);
	send(sk, (char*)&Len, sizeof(Len), 0);
	send(sk, (char*)sendBuff, Len, 0);
	delete sendBuff;*/

	int Len = (Command.GetLength() + 1) << 1;
	PBYTE sendBuff = new BYTE[1000];
	memcpy(sendBuff, (PBYTE)&Len, sizeof(Len));
	memcpy(sendBuff + sizeof(Len), (PBYTE)(LPCTSTR)Command, Len);
	Len += sizeof(Len);

	int sent = 0, sentSoFar;
	while (sent < Len) {
		sentSoFar = send(sk, (char*)(sendBuff + sent), Len - sent, 0);
		if (sentSoFar != SOCKET_ERROR)
			sent += sentSoFar;
	}

	delete[] sendBuff;
}

int CServerChatDlg::mRecv(SOCKET sk, CString &Command)
{
	/*int received = 0, receivedSoFar;
	while (received < 4) {
		receivedSoFar = recv(sk, (char*)(&buffLength + received), sizeof(int) - received, 0);
		if (receivedSoFar != SOCKET_ERROR)
			received += receivedSoFar;
	}*/
	recv(sk, (char*)&buffLength, sizeof(int), 0);
	PBYTE buffer = new BYTE[buffLength + 2];
	memset(buffer, 0, buffLength + 2);
	recv(sk, (char*)buffer, buffLength, 0);

	int flag = -1;
	TCHAR* ttc = (TCHAR*)buffer;
	Command = ttc;

	/*if (Command.GetLength() == 0)
		return -1;
	else */
	if (Command[0] == '0')
		flag = 0;
	else if (Command[0] == '1')
		flag = 1;
	else if (Command[0] == '2')
		flag = 2;
	else if (Command[0] == '3')
		flag = 3;
	else if (Command[0] == '4')
		flag = 4;
	else if (Command[0] == '5')
		flag = 5;
	else if (Command[0] == '6')
		flag = 6;
	else if (Command[0] == '7')
		flag = 7;
	else if (Command[0] == '8')
		flag = 8;
	else if (Command[0] == '9')
		flag = 9;

	delete[]buffer;

	return flag;
}

int CServerChatDlg::mSend(SOCKET sk, const char* buf, int len)
{
	/*m_BuffLen = sizeof(m_BuffLen) + len;
	memcpy(m_Buffer, &len, sizeof(m_BuffLen));
	memcpy(m_Buffer + sizeof(m_BuffLen), buf, len);
	return send(sk, m_Buffer, m_BuffLen, 0);*/

	int m_BuffLen = sizeof(m_BuffLen) + len;

	memcpy(m_Buffer, &len, sizeof(len));
	memcpy(m_Buffer + sizeof(len), buf, len);

	int sent = 0, sentSoFar;
	while (sent < m_BuffLen) {
		sentSoFar = send(sk, m_Buffer + sent, m_BuffLen - sent, 0);
		if (sentSoFar != SOCKET_ERROR)
			sent += sentSoFar;
	}

	return m_BuffLen;
}

int CServerChatDlg::mRecv(SOCKET sk, char* buf)
{
	/*recv(sk, (char*)&buffLength, sizeof(buffLength), 0);
	return recv(sk, buf, buffLength, 0);*/

	int received = 0, m_BuffLen, receivedSoFar;
	while (received < 4) {
		receivedSoFar = recv(sk, (char*)(&m_BuffLen + received), sizeof(m_BuffLen) - received, 0);
		if (receivedSoFar != SOCKET_ERROR)
			received += receivedSoFar;
	}

	received = 0;
	while (received < m_BuffLen) {
		receivedSoFar = recv(sk, buf + received, m_BuffLen - received, 0);
		if (receivedSoFar != SOCKET_ERROR)
			received += receivedSoFar;
	}

	return m_BuffLen;
}

void CServerChatDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}


LRESULT CServerChatDlg::threadEnded(WPARAM wParam, LPARAM lParam)
{
	m_msgString += _T("Done sending file to ") + (CString)pSock[wParam].Name + _T("\r\n");
	setMessage();
	return 0;
}


DWORD WINAPI FuncSendFile(LPVOID lp) {
	ThreadSendFile *a;
	a = (ThreadSendFile*)lp;
	a->sn->SendFile(a->m_FilePath);
	SendMessage(a->hWnd, WM_THREAD_SENDFILE_ENDED, a->post, 0);
	return 0;
}


LRESULT CServerChatDlg::SockMsg(WPARAM wParam, LPARAM lParam)
{
	if (WSAGETSELECTERROR(lParam))
	{
		// Display the error and close the socket
		closesocket(wParam);
	}
	switch (WSAGETSELECTEVENT(lParam))
	{
	case FD_ACCEPT:
	{
		pSock[number_Socket++].sockClient = accept(wParam, NULL, NULL);
		GetDlgItem(IDC_LISTEN)->EnableWindow(FALSE);
		break;
	}
	case FD_READ:
	{

		int post = -1, dpos = -1;

		for (int i = 0; i < number_Socket; i++)
		{
			if (pSock[i].sockClient == wParam)
			{
				post = i;
			}
		}

		CString temp;
		if (mRecv(wParam, temp) < 0)
			break;
		Split(temp, strResult);
		if (strResult.size() == 0)
			break;

		int flag = _ttoi(strResult[0]);
		switch (flag)
		{
		case FLAG_REGISTRATION:
		{
			CString strSQLQuery = CString("SELECT * FROM [USER]");
			CRecordset recUser(&m_Database);
			CString inputUsername = strResult[1], inputPassword = strResult[2];
			recUser.m_strFilter = _T("Username = '") + inputUsername + _T("'");
			recUser.Open(CRecordset::snapshot, strSQLQuery, CRecordset::readOnly);

			if (recUser.IsBOF() == 0) //username already exists
			{
				Command = _T("0\r\n0\r\n");
			}
			else {
				Command = _T("0\r\n1\r\n");
				strSQLQuery = _T("INSERT INTO [USER] VALUES('") + inputUsername + _T("', '") + inputPassword + _T("')");
				m_Database.ExecuteSQL(strSQLQuery);
			}

			recUser.Close();

			mSend(wParam, Command);
			setMessage();
			break;
		}
		case FLAG_LOGIN://Login
		{
			CString strSQLQuery = CString("SELECT * FROM [USER]");
			CRecordset recUser(&m_Database);
			CString inputUsername = strResult[1], inputPassword = strResult[2];
			recUser.m_strFilter = _T("Username = '") + inputUsername + _T("' and Password = '") + inputPassword + _T("'");
			recUser.Open(CRecordset::snapshot, strSQLQuery, CRecordset::readOnly);

			int post2 = -1;
			bool check = 1;

			for (int i = 0; i < number_Socket; i++)
			{
				if (pSock[i].Name == inputUsername)
				{
					post2 = i;
				}
			}

			if (post2 != -1 && pSock[post2].HasUser == true) check = 0;

			if (recUser.IsBOF() == 0 && check == 1)
			{
				pSock[post].HasUser = true;
				char *tem = ConvertToChar(inputUsername);
				strcpy(pSock[post].Name, tem);
				delete[]tem;
				
				Command = _T("1\r\n1\r\n") + inputUsername + _T("\r\n");
				for (int i = 0; i < number_Socket; i++)
				{
					if (pSock[i].HasUser) mSend(pSock[i].sockClient, Command);
				}

				m_msgString += strResult[1] + _T(" logged in\r\n");
				m_Edit_OnlineList += inputUsername + _T("\r\n");

				strSQLQuery = _T("INSERT INTO [ONLINE] VALUES('") + inputUsername + _T("')");
				m_Database.ExecuteSQL(strSQLQuery);
			}
			else
			{
				/*CRecordset recUser1(&m_Database);
				recUser1.m_strFilter = _T("Username = '") + inputUsername + _T("'");
				recUser1.Open(CRecordset::snapshot, strSQLQuery, CRecordset::readOnly);
				if (recUser1.IsBOF() == 0)
					Command = _T("1\r\n0\r\n0\r\n");
				else
					Command = _T("1\r\n0\r\n1\r\n");
				recUser1.Close();*/
				Command = _T("1\r\n0\r\n");
			}

			recUser.Close();

			if (!pSock[post].HasUser) mSend(wParam, Command);

			setMessage();
			break;
		}
		case FLAG_LOGOUT:
		{
			m_msgString += pSock[post].Name;
			m_msgString += " logged out\r\n";
			pSock[post].HasUser = false;

			//Close and delete if sending or receiving file
			if (pSock[post].fileInfo != nullptr) {
				closeDestFile(post);
				delete pSock[post].fileInfo;
				pSock[post].fileInfo = nullptr;
			}

			TerminateThread(pSock[post].hSendFileThreadId, pSock[post].dwSendFileThreadId);
			
			CString strSQLQuery = _T("DELETE FROM [ONLINE] WHERE Username = '") + (CString)pSock[post].Name + _T("'");
			m_Database.ExecuteSQL(strSQLQuery);

			m_Edit_OnlineList = "";
			appendOnlinesToCStr(m_Edit_OnlineList);

			Command = _T("3\r\n") + (CString)(pSock[post].Name) + _T("\r\n");
			for (int i = 0; i < number_Socket; i++)
			{
				if (pSock[i].HasUser)
					mSend(pSock[i].sockClient, Command);
			}

			setMessage();
			break;
		}

		case FLAG_SELECT_CLIENT:
		{
			CString strSQLQuery;
			CRecordset recUser(&m_Database);

			if (strResult[1] == SERVER_CHAT)
			{
				strSQLQuery = _T("SELECT * FROM [MESSAGE] Where To_User = '") + (CString)SERVER_CHAT + _T("'");
			}
			else
			{
				strSQLQuery = _T("SELECT * FROM [MESSAGE] Where (Username = '") + strResult[1] + _T("' and To_user = '") + (CString)pSock[post].Name + _T("') or ( To_user = '") + strResult[1] + _T("' and Username = '") + (CString)pSock[post].Name + _T("')");
			}

			recUser.Open(CRecordset::forwardOnly, strSQLQuery, CRecordset::readOnly);

			while (!recUser.IsEOF())
			{
				CString message, username;
				recUser.GetFieldValue(_T("Username"), username);
				recUser.GetFieldValue(_T("Mess"), message);

				Command = _T("4\r\n") + username + _T("\r\n");
				Command += message + _T("\r\n");
				mSend(wParam, Command);

				recUser.MoveNext();
			}

			recUser.Close();

			setMessage();
			break;
		}

		case FLAG_CHAT:
		{
			CString strSQLQuery;
			CRecordset recUser(&m_Database);
			CString usernameFrom = strResult[1], message = strResult[2], usernameTo = strResult[3];

			Command = _T("2\r\n");
			Command += usernameFrom + _T("\r\n") + message + _T("\r\n") + usernameTo + _T("\r\n");

			m_msgString += pSock[post].Name;
			if (usernameTo != SERVER_CHAT)
			{
				m_msgString += _T(" to ") + usernameTo;
			}
			m_msgString += _T(" : ") + message + _T("\r\n");

			strSQLQuery = _T("INSERT INTO [MESSAGE] VALUES('") + usernameFrom + _T("', N'") + message + _T("', '") + usernameTo + _T("')");
			m_Database.ExecuteSQL(strSQLQuery);

			recUser.Close();
			if (usernameTo == SERVER_CHAT)
			{
				for (int i = 0; i < number_Socket; i++)
				{
					if (pSock[i].HasUser) mSend(pSock[i].sockClient, Command);
				}
			}
			else
			{
				for (int i = 0; i < number_Socket; i++)
				{
					if (pSock[i].HasUser && (CString)pSock[i].Name == usernameTo)
					{
						mSend(pSock[i].sockClient, Command);
						break;
					}
				}
			}

			setMessage();
			break;
		}
		case FLAG_SEND_LIST_ONLINE:
		{
			Command = _T("5\r\n");
			appendOnlinesToCStr(Command);
			mSend(wParam, Command);

			m_msgString += (CString)pSock[post].Name + _T(" wants to read user online list\r\n");

			setMessage();
			break;
		}
		case FLAG_FILEINFO:
		{
			m_msgString += _T("Received a file request from ") + (CString)pSock[post].Name + _T("\r\n");
			setMessage();

			mRecv(wParam, m_Buffer);
			if (pSock[post].fileInfo == nullptr)
				pSock[post].fileInfo = new File;
			
			memcpy(&pSock[post].fileInfo->m_FileSize, m_Buffer, sizeof(pSock[post].fileInfo->m_FileSize));
			int byteSizeRead = sizeof(pSock[post].fileInfo->m_FileSize);
			
			pSock[post].fileInfo->m_FileName = (LPCTSTR)(m_Buffer + byteSizeRead);
			byteSizeRead += ((pSock[post].fileInfo->m_FileName.GetLength() + 1) << 1);

			pSock[post].fileInfo->m_ToWhom = (LPCTSTR)(m_Buffer + byteSizeRead);
			
			pSock[post].fileInfo->m_SizeReceived = 0;

			pSock[post].fileInfo->m_FileID = m_numFiles;

			CString idName;
			idName.Format(_T("Files/%d"), m_numFiles++);
			idName += GetFileExtension(pSock[post].fileInfo->m_FileName);
			openDestFile(post, idName);

			break;
		}
		case FLAG_FILEDATA:
		{
			GetFileFromRemoteSender(post);
			break;
		}
		case FLAG_FILELIST:
		{
			CString strSQLQuery;
			CRecordset recUser(&m_Database);

			if (strResult[1] == SERVER_CHAT)
			{
				strSQLQuery = _T("SELECT * FROM [FILE] Where TOWHOM = '") + (CString)SERVER_CHAT + _T("'");
			}
			else
			{
				strSQLQuery = _T("SELECT * FROM [FILE] Where (FROMUSER = '") + strResult[1] + _T("' and TOWHOM = '") + (CString)pSock[post].Name + _T("') or ( TOWHOM = '") + strResult[1] + _T("' and FROMUSER = '") + (CString)pSock[post].Name + _T("')");
			}

			recUser.Open(CRecordset::forwardOnly, strSQLQuery, CRecordset::readOnly);

			while (!recUser.IsEOF())
			{
				CString fileName, ID, from, to;
				recUser.GetFieldValue(_T("ID"), ID);
				recUser.GetFieldValue(_T("NAME"), fileName);
				recUser.GetFieldValue(_T("FROMUSER"), from);
				recUser.GetFieldValue(_T("TOWHOM"), to);

				Command = _T("8\r\n") + fileName + _T("\r\n") + ID + _T("\r\n") + from + _T("\r\n") + to + _T("\r\n");
				mSend(wParam, Command);

				recUser.MoveNext();
			}

			recUser.Close();

			setMessage();
			break;
		}
		case FLAG_CLIENT_DOWNLOAD:
		{
			m_msgString += (CString)pSock[post].Name + _T(" requested file ") + strResult[1] + _T("\r\n");
			setMessage();

			//SendFile(filePath, (CString)pSock[post].Name, wParam);
			pSock[post].sendFileStruct.m_FilePath = (CString)FILE_DIRECTORY + _T("/") + strResult[1];
			pSock[post].sendFileStruct.sn = &pSock[post];
			pSock[post].sendFileStruct.hWnd = m_hWnd;
			pSock[post].sendFileStruct.post = post;

			pSock[post].hSendFileThreadId = CreateThread(NULL, 0, FuncSendFile, (LPVOID)&pSock[post].sendFileStruct, 0, &pSock[post].dwSendFileThreadId);

			break;
		}
		}
		break;
	}

	case FD_CLOSE:
	{
		int post = -1;
		for (int i = 0; i < number_Socket; i++)
		{
			if (pSock[i].sockClient == wParam)
			{
				post = i;
			}
		}

		if (pSock[post].HasUser) {
			m_msgString += pSock[post].Name;
			m_msgString += " logged out\r\n";
			pSock[post].HasUser = false;

			//Close end delete if sending or receiving file
			if (pSock[post].fileInfo != nullptr) {
				closeDestFile(post);
				delete pSock[post].fileInfo;
				pSock[post].fileInfo = nullptr;
			}

			TerminateThread(pSock[post].hSendFileThreadId, pSock[post].dwSendFileThreadId);

			Command = _T("3\r\n") + (CString)(pSock[post].Name) + _T("\r\n");
			for (int i = 0; i < number_Socket; i++)
			{
				if (pSock[i].HasUser) mSend(pSock[i].sockClient, Command);
			}

			CString strSQLQuery = _T("DELETE FROM [ONLINE] WHERE Username = '") + (CString)pSock[post].Name + _T("'");
			m_Database.ExecuteSQL(strSQLQuery);

			m_Edit_OnlineList = "";
			appendOnlinesToCStr(m_Edit_OnlineList);
		}
		
		closesocket(wParam);
		for (int j = post; j < number_Socket - 1; j++)
		{
			pSock[j] = pSock[j + 1];
		}
		number_Socket--;

		setMessage();
		break;
	}
	}
	return 0;
}


void CServerChatDlg::setMessage()
{
	UpdateData(FALSE);
	CEdit* e = (CEdit*)GetDlgItem(IDC_BOXCHAT);
	e->SetFocus();
	e->SetSel(-1);
	e->HideCaret();
}


void CServerChatDlg::appendOnlinesToCStr(CString &str)
{
	CString strSQLQuery = CString("SELECT * FROM [ONLINE]");
	CRecordset recUser(&m_Database);
	recUser.Open(CRecordset::forwardOnly, strSQLQuery, CRecordset::readOnly);

	while (!recUser.IsEOF())
	{
		CString user;
		recUser.GetFieldValue(_T("Username"), user);
		str += user + _T("\r\n");
		recUser.MoveNext();
	}

	recUser.Close();
}


void CServerChatDlg::OnBnClickedListen()
{
	// TODO: Add your control notification handler code here
	UpdateData();

	sockServer = socket(AF_INET, SOCK_STREAM, 0);
	serverAdd.sin_family = AF_INET;
	serverAdd.sin_port = htons(PORT);
	serverAdd.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(sockServer, (SOCKADDR*)&serverAdd, sizeof(serverAdd));
	listen(sockServer, 5);
	int err = WSAAsyncSelect(sockServer, m_hWnd, WM_SOCKET, FD_READ | FD_ACCEPT | FD_CLOSE);
	if (err)
		MessageBox((LPCTSTR)"Cant call WSAAsyncSelect");
	GetDlgItem(IDC_LISTEN)->EnableWindow(FALSE);
	number_Socket = 0;
	m_numFiles = 0;
	pSock = new SockName[200];
}

BOOL CServerChatDlg::GetFileFromRemoteSender(int sockNum)
{
	//Receive all bytes of current file chunk to m_Buffer
	int chunkSize = mRecv(pSock[sockNum].sockClient, m_Buffer);
	pSock[sockNum].fileInfo->m_destFile.Write(m_Buffer, chunkSize);
	pSock[sockNum].fileInfo->m_SizeReceived += chunkSize;

	// only close file if it's open (open might have failed above)
	if (pSock[sockNum].fileInfo->m_SizeReceived == pSock[sockNum].fileInfo->m_FileSize) {
		m_msgString += _T("Received file ") + pSock[sockNum].fileInfo->m_FileName
			+ _T(" from ") + (CString)pSock[sockNum].Name
			+ _T(" to ") + pSock[sockNum].fileInfo->m_ToWhom +_T("\r\n");
		setMessage();

		//Insert into database
		CRecordset recUser(&m_Database);

		CString strSQLQuery;
		strSQLQuery.Format(_T("INSERT INTO [FILE] VALUES(%d"), pSock[sockNum].fileInfo->m_FileID);
		strSQLQuery += _T(", N'") + pSock[sockNum].fileInfo->m_FileName + _T("', N'")
			+ pSock[sockNum].Name + _T("', N'") + pSock[sockNum].fileInfo->m_ToWhom + _T("')");
		m_Database.ExecuteSQL(strSQLQuery);

		recUser.Close();

		//Send new successfully received file name
		strSQLQuery.Format(_T("%d"), pSock[sockNum].fileInfo->m_FileID);
		Command = _T("8\r\n") + pSock[sockNum].fileInfo->m_FileName + _T("\r\n")
			+ strSQLQuery + _T("\r\n") + (CString)pSock[sockNum].Name + _T("\r\n")
			+ pSock[sockNum].fileInfo->m_ToWhom + _T("\r\n");
		if (pSock[sockNum].fileInfo->m_ToWhom == SERVER_CHAT) {
			for (int i = 0; i < number_Socket; ++i)
				if (pSock[i].HasUser == true)
					mSend(pSock[i].sockClient, Command);
		}
		else {
			mSend(pSock[sockNum].sockClient, Command);
			for (int i = 0; i < number_Socket; ++i)
				if (pSock[i].Name == pSock[sockNum].fileInfo->m_ToWhom) {
					mSend(pSock[i].sockClient, Command);
					break;
				}
		}

		closeDestFile(sockNum);
		delete pSock[sockNum].fileInfo;
		pSock[sockNum].fileInfo = nullptr;
	}

	return TRUE;
}


bool CServerChatDlg::openDestFile(int sockNum, const CString &fileName)
{
	CFileException fe;
	// open/create target file that receives the transferred data
	if (!(pSock[sockNum].fileInfo->m_bFileIsOpen = pSock[sockNum].fileInfo->m_destFile.Open(fileName, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary, &fe)))
	{
		TCHAR strCause[256];
		fe.GetErrorMessage(strCause, 255);
		TRACE("GetFileFromRemoteSender encountered an error while opening the local file\n"
			"\tFile name = %s\n\tCause = %s\n\tm_cause = %d\n\tm_IOsError = %d\n",
			fe.m_strFileName, strCause, fe.m_cause, fe.m_lOsError);
		return false;
	}

	return true;
}


bool CServerChatDlg::closeDestFile(int sockNum)
{
	if (pSock[sockNum].fileInfo->m_bFileIsOpen) {
		pSock[sockNum].fileInfo->m_destFile.Close();
		pSock[sockNum].fileInfo->m_bFileIsOpen = false;
		return true;
	}

	return false;
}


CString CServerChatDlg::GetFileExtension(CString fileName)
{
	// Convert a TCHAR string to a LPCSTR
	CT2CA pszConvertedAnsiString(fileName);
	// construct a std::string using the LPCSTR input
	string nameStr(pszConvertedAnsiString);
	
	size_t dotPos = nameStr.find_last_of('.');
	
	string extension = nameStr.substr(dotPos);
	
	CString res(extension.c_str());
	return res;
}


BOOL SockName::SendFile(const CString &FileName)
{
	// return value
	BOOL bRet = TRUE;
	// used to monitor the progress of a sending operation
	int fileLength, cbLeftToSend;
	// pointer to buffer for sending data
	// (memory is allocated after sending file size)
	BYTE* sendData = NULL;
	sendData = new BYTE[MAX_BUFF_SIZE + 200];

	CFile sourceFile;
	CFileException fe;
	BOOL bFileIsOpen = FALSE;
	CString command;
	CString fileName;

	if (!(bFileIsOpen = sourceFile.Open(FileName,
		CFile::modeRead | CFile::typeBinary | CFile::shareDenyNone, &fe)))
	{
		TCHAR strCause[256];
		fe.GetErrorMessage(strCause, 255);
		TRACE("SendFileToRemoteRecipient encountered an error while opening the local file\n"
			"\tFile name = %s\n\tCause = %s\n\tm_cause = %d\n\tm_IOsError = %d\n",
			fe.m_strFileName, strCause, fe.m_cause, fe.m_lOsError);

		/* you should handle the error here */

		bRet = FALSE;
		goto PreReturnCleanup;
	}

	// first send info of file

	fileLength = sourceFile.GetLength();
	//fileLength = htonl(fileLength);
	fileName = sourceFile.GetFileName();

	// send: [sizeof(FLAG) | FLAG | size of file info | file size | file name | receiver | NULL]
	//                                                  ----------------------------  => file info
	command.Format(_T("%d\r\n"), FLAG_FILEINFO);
	int len = (command.GetLength() + 1) * 2;
	memcpy(sendData, (PBYTE)&len, sizeof(len));											//sizeof(FLAG)
	memcpy(sendData + sizeof(len), (PBYTE)(LPCTSTR)command, len);						//FLAG
	len += sizeof(len);
	/*mSend(command);
	len = 0;*/
	int lenInfo = sizeof(fileLength) + (fileName.GetLength() + 1) * 2 + (((CString)Name).GetLength() + 1) * 2;
	memcpy(sendData + len, (PBYTE)&lenInfo, sizeof(lenInfo));							//size of file info
	len += sizeof(lenInfo);
	memcpy(sendData + len, (PBYTE)&fileLength, sizeof(fileLength));						//file size
	len += sizeof(fileLength);
	memcpy(sendData + len, (PBYTE)(LPCTSTR)fileName, (fileName.GetLength() + 1) * 2);	//file name include null-terminate
	len += (fileName.GetLength() + 1) * 2;
	memcpy(sendData + len, Name, (((CString)Name).GetLength() + 1) * 2);			//receiver include null-terminate
	len += (((CString)Name).GetLength() + 1) * 2;

	//send(sClient, (char*)sendData, len, 0);
	cbLeftToSend = len;
	do
	{
		int cbBytesSent;
		cbBytesSent = send(sockClient, (char*)sendData + (len - cbLeftToSend), cbLeftToSend, 0);

		// test for errors and get out if they occurred
		if (cbBytesSent == SOCKET_ERROR)
		{
			int iErr = ::GetLastError();
			TRACE("SendFileToRemoteRecipient returned a socket error while sending file length\n"
				"\tNumber of Bytes sent = %d\n"
				"\tGetLastError = %d\n", cbBytesSent, iErr);

			/* you should handle the error here */

			bRet = FALSE;
			goto PreReturnCleanup;
		}

		// data was successfully sent, so account
		// for it with already-sent data
		cbLeftToSend -= cbBytesSent;
	} while (cbLeftToSend > 0);


	// now send the file's data    
	// send: [sizeof(FLAG) | FLAG | size of message data | data size | file data]
	//                                                     ---------------------  => message data
	command.Format(_T("%d\r\n"), FLAG_FILEDATA);
	len = (command.GetLength() + 1) * 2;
	memcpy(sendData, (PBYTE)&len, sizeof(len));									//sizeof(FLAG)
	memcpy(sendData + sizeof(len), (PBYTE)(LPCTSTR)command, len);				//FLAG
	len += sizeof(len);															//len: size of header

	cbLeftToSend = fileLength;
	do
	{
		// read next chunk of SEND_BUFFER_SIZE bytes from file
		int sendThisTime, doneSoFar, buffOffset;

		sendThisTime = sourceFile.Read(sendData + len + sizeof(sendThisTime), MAX_BUFF_SIZE);						//file data
		memcpy(sendData + len, (PBYTE)&sendThisTime, sizeof(sendThisTime));											//data size

		buffOffset = 0;
		cbLeftToSend -= sendThisTime;
		sendThisTime += len + sizeof(sendThisTime);
		do
		{
			doneSoFar = send(sockClient, (char*)sendData + buffOffset, sendThisTime, 0);

			// test for errors and get out if they occurred
			if (doneSoFar == SOCKET_ERROR)
			{
				int iErr = ::GetLastError();
				TRACE("SendFileToRemoteRecipient returned a socket error while sending chunked file data\n"
					"\tNumber of Bytes sent = %d\n"
					"\tGetLastError = %d\n", doneSoFar, iErr);

				/* you should handle the error here */

				/*if (iErr != WSAECONNABORTED) {
					bRet = FALSE;
					goto PreReturnCleanup;
				}*/
			}
			else {
				// data was successfully sent,
				// so account for it with already-sent data
				buffOffset += doneSoFar;
				sendThisTime -= doneSoFar;
			}

		} while (sendThisTime > 0);

	} while (cbLeftToSend > 0);

PreReturnCleanup: // labelled goto destination

	// free allocated memory
	// if we got here from a goto that skipped allocation,
	// delete of NULL pointer
	// is permissible under C++ standard and is harmless
	delete[] sendData;

	// only close file if it's open (open might have failed above)
	if (bFileIsOpen)
		sourceFile.Close();

	//return bRet;
	return bRet;
}