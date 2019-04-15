// ClientDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Client.h"
#include "ClientDlg.h"
#include "afxdialogex.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CClientDlg dialog



CClientDlg::CClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CLIENT_DIALOG, pParent)
	, m_Username(_T(""))
	, m_EditMessage(_T(""))
	, m_msgString(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_fileInfo = new File();
}


CClientDlg::~CClientDlg()
{
	delete m_fileInfo;
}


void CClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_USERNAME, m_Username);
	DDX_Text(pDX, IDC_EDIT_MESSAGE_BOX, m_EditMessage);
	DDX_Control(pDX, IDC_COMBO_TO, m_ToBox);
	DDX_Text(pDX, IDC_MESSAGE_LIST, m_msgString);
	DDX_Control(pDX, IDC_LIST_FILES, m_listFiles);
}

BEGIN_MESSAGE_MAP(CClientDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_SOCKET_READ, UpdateUI)
	ON_MESSAGE(WM_SOCKET, SockMsg)
	ON_MESSAGE(WM_THREAD_FILE_ENDED, TurnOnSendFileBtn)
	ON_BN_CLICKED(IDC_BUTTON_LOGOUT, &CClientDlg::OnBnClickedButtonLogout)
	ON_BN_CLICKED(IDC_BUTTON_SEND, &CClientDlg::OnBnClickedButtonSend)
	ON_CBN_SELCHANGE(IDC_COMBO_TO, &CClientDlg::OnCbnSelchangeComboTo)
	ON_BN_CLICKED(IDC_BUTTON_SENDFILE, &CClientDlg::OnBnClickedButtonSendfile)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_FILES, &CClientDlg::OnNMDblclkListFiles)
END_MESSAGE_MAP()


// CClientDlg message handlers

BOOL CClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	sClient.SetHWND(m_hWnd);

	pReceiveThread = new ReceiveMessagesThread(m_hWnd, &strResult, m_fileInfo);
	pReceiveThread->CreateThread();
	
	sClient.mSend(_T("5\r\n"));
	CString choose = _T("4\r\n") + (CString)SERVER_CHAT + _T("\r\n");
	sClient.mSend(choose);
	
	choose.Format(_T("%d\r\n"), FLAG_FILELIST);
	choose += (CString)SERVER_CHAT + _T("\r\n");
	sClient.mSend(choose);

	m_listFiles.InsertColumn(0, _T("Shared files"), LVCFMT_CENTER, 134);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CClientDlg::OnPaint()
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
HCURSOR CClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


LRESULT CClientDlg::SockMsg(WPARAM wParam, LPARAM lParam)
{
	if (WSAGETSELECTERROR(lParam))
	{
		// Display the error and close the socket
		closesocket(wParam);
	}
	switch (WSAGETSELECTEVENT(lParam))
	{
	case FD_READ:
	{
		pReceiveThread->PostThreadMessageW(WM_SOCKET, wParam, lParam);
		break;
	}
	case FD_CLOSE:
	{
		GetDlgItemText(IDC_EDIT_MESSAGE_BOX, m_EditMessage);

		//Close end delete if sending or receiving file
		if (m_fileInfo != nullptr) {
			pReceiveThread->closeDestFile();
		}

		sClient.Close();
		
		m_msgString += _T("Server da dong ket noi\r\n");
		GetDlgItem(IDC_BUTTON_LOGOUT)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_SEND)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_SENDFILE)->EnableWindow(FALSE);

		setMessage();
		break;
	}
	}

	return 0;
}


LRESULT CClientDlg::UpdateUI(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case FLAG_ONLINE_LIST:
	{
		m_ToBox.ResetContent();
		m_ToBox.AddString(SERVER_CHAT);
		for (int i = 1; i < strResult.size(); ++i)
			if (strResult[i] != m_Username)
				m_ToBox.AddString(strResult[i]);
		m_ToBox.SetCurSel(m_ToBox.FindStringExact(0, SERVER_CHAT));
		break;
	}
	case FLAG_MESSAGE:
	{
		if (strResult[1] != m_Username) {
			GetDlgItemText(IDC_EDIT_MESSAGE_BOX, m_EditMessage);
			CString to;
			m_ToBox.GetLBText(m_ToBox.GetCurSel(), to);
			if ((strResult[3] == to && to == SERVER_CHAT) || (strResult[1] == to && strResult[3] != SERVER_CHAT))
				m_msgString += strResult[1] + ": " + strResult[2] + _T("\r\n");
			if (strResult[3] != SERVER_CHAT && to != strResult[1])
				m_msgString += strResult[1] + _T(" texted you\r\n");
			setMessage();
		}
		break;
	}
	case FLAG_TO_WHOM:
	{
		GetDlgItemText(IDC_EDIT_MESSAGE_BOX, m_EditMessage);
		if (strResult[1] != m_Username)
			m_msgString += strResult[1] + ": " + strResult[2] + _T("\r\n");
		else
			m_msgString += _T("Me: ") + strResult[2] + _T("\r\n");
		setMessage();
		break;
	}
	case FLAG_LOGIN:
	{
		GetDlgItemText(IDC_EDIT_MESSAGE_BOX, m_EditMessage);
		if (m_Username != strResult[2]) {
			m_ToBox.AddString(strResult[2]);
			m_msgString += strResult[2] + _T(" logged in\r\n");
			setMessage();
		}
		break;
	}
	case FLAG_LOGOUT:
	{
		GetDlgItemText(IDC_EDIT_MESSAGE_BOX, m_EditMessage);
		if (m_ToBox.GetCurSel() == m_ToBox.FindStringExact(0, SERVER_CHAT))
			m_msgString += strResult[1] + _T(" logged out\r\n");
		else {
			if (m_ToBox.GetCurSel() == m_ToBox.FindStringExact(0, strResult[1])) {
				m_ToBox.SetCurSel(m_ToBox.FindStringExact(0, SERVER_CHAT));
				OnCbnSelchangeComboTo();
			}
		}
		m_ToBox.DeleteString(m_ToBox.FindStringExact(0, strResult[1]));

		setMessage();
		break;
	}
	case FLAG_FILELIST:
	{
		CString fileName = strResult[1];
		int id = _ttoi(strResult[2]);
		CString fromUser = strResult[3];
		CString toUser = strResult[4];
		CString userChatWith;
		m_ToBox.GetLBText(m_ToBox.GetCurSel(), userChatWith);

		if (userChatWith != SERVER_CHAT) {
			if ((fromUser != m_Username && fromUser != userChatWith) || (toUser != m_Username && toUser != userChatWith)) break;
		}
		else {
			if (toUser != SERVER_CHAT) break;
		}

		bool isNew = true;
		//for (pair<int, CString> itemFile : m_ListItemFile) {
		//	if (itemFile.first == id) { //this item file already exist in list item file
		//		isNew = false;
		//		break;
		//	}
		//}
		if (isNew) {
			m_ListItemFile.push_back(pair<int, CString>(id, fileName));
			int nIndex = m_listFiles.InsertItem(0, fileName);
		}
		break;
	}
	case FLAG_FILEINFO:
	{
		m_msgString += _T("Downloading file ") + m_fileInfo->m_FileName + _T("\r\n");
		setMessage();
		break;
	}
	case FLAG_FILEDATA:
	{
		m_msgString += _T("Downloaded file ") + m_fileInfo->m_FileName + _T("\r\n");
		setMessage();
		break;
	}
	}

	return 0;
}


void CClientDlg::OnBnClickedButtonLogout()
{
	// TODO: Add your control notification handler code here
	CString command = _T("3\r\n");
	sClient.mSend(command);

	CDialog::OnCancel();
	CLoginDlg m_LoginDlg;
	//m_pMainWnd = &m_ClientDlg;
	INT_PTR nResponse = m_LoginDlg.DoModal();
}


void CClientDlg::OnBnClickedButtonSend()
{
	// TODO: Add your control notification handler code here
	GetDlgItemText(IDC_EDIT_MESSAGE_BOX, m_EditMessage);
	if (m_EditMessage == "")
		return;

	m_msgString += _T("Me: ") + m_EditMessage + _T("\r\n");

	CString to;
	m_ToBox.GetLBText(m_ToBox.GetCurSel(), to);
	CString command = _T("2\r\n") + m_Username + "\r\n" + m_EditMessage + "\r\n" + to + "\r\n";

	m_EditMessage = "";
	setMessage();

	sClient.mSend(command);
}


void CClientDlg::OnCbnSelchangeComboTo()
{
	// TODO: Add your control notification handler code here
	GetDlgItemText(IDC_EDIT_MESSAGE_BOX, m_EditMessage);
	m_msgString = "";
	setMessage();
	CString to;
	m_ToBox.GetLBText(m_ToBox.GetCurSel(), to);
	CString command = _T("4\r\n") + to + _T("\r\n");
	sClient.mSend(command);
	m_listFiles.DeleteAllItems();
	m_ListItemFile.clear();
	command.Format(_T("%d\r\n"), FLAG_FILELIST);
	command += to + _T("\r\n");
	sClient.mSend(command);
}

DWORD WINAPI FuncSendFile(LPVOID lp) {
	ThreadSendFile *a;
	a = (ThreadSendFile*)lp;
	a->sk->SendFile(a->m_FileName, a->m_toUser);
	SendMessage(a->hWnd, WM_THREAD_FILE_ENDED, 0, 0);
	return 0;
}

LRESULT CClientDlg::TurnOnSendFileBtn(WPARAM wParam, LPARAM lParam)
{
	GetDlgItem(IDC_BUTTON_SENDFILE)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_SENDFILE)->SetWindowTextW(_T("Send file"));
	return 0;
}

void CClientDlg::OnBnClickedButtonSendfile()
{
	// TODO: Add your control notification handler code here
	CFileDialog dlgFile(TRUE);
	int iRet = dlgFile.DoModal();
	if (iRet == IDCANCEL) return;
	GetDlgItem(IDC_BUTTON_SENDFILE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_SENDFILE)->SetWindowTextW(_T("Sending file..."));
	
	m_ToBox.GetLBText(m_ToBox.GetCurSel(), sendFileStruct.m_toUser);
	sendFileStruct.m_FileName = dlgFile.GetPathName();

	sendFileStruct.sk = &sClient;
	sendFileStruct.hWnd = m_hWnd;
	hSendFileThreadId = CreateThread(NULL, 0, FuncSendFile, (LPVOID)&sendFileStruct, 0, &dwSendFileThreadId);
	//sClient.SendFile(dlgFile.GetPathName(), to);

	setMessage();
}

void CClientDlg::setMessage()
{
	UpdateData(FALSE);
	CEdit* e = (CEdit*)GetDlgItem(IDC_MESSAGE_LIST);
	e->SetFocus();
	e->SetSel(-1);
	GetDlgItem(IDC_EDIT_MESSAGE_BOX)->SetFocus();
}


void CClientDlg::OnNMDblclkListFiles(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	int row = pNMItemActivate->iItem;
	row = m_ListItemFile.size() - 1 - row;
	/*CString str;
	str.Format(_T("%d, %d, %s"), row, m_ListItemFile[row].first, m_ListItemFile[row].second);
	AfxMessageBox(str);*/
	CString command;
	command.Format(_T("%d\r\n%d%s\r\n"), FLAG_CLIENT_DOWNLOAD, m_ListItemFile[row].first, GetFileExtension(m_ListItemFile[row].second));
	//AfxMessageBox(command);

	CFileDialog dlgFile(FALSE, NULL, m_ListItemFile[row].second);
	int iRet = dlgFile.DoModal();
	if (iRet == IDCANCEL) return;
	
	GetDlgItem(IDC_BUTTON_SENDFILE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_SENDFILE)->SetWindowTextW(_T("Downloading.."));
	
	m_fileInfo->m_PathFile = dlgFile.GetPathName();
	m_fileInfo->m_FileName = m_ListItemFile[row].second;

	sClient.mSend(command);

	*pResult = 0;
}


CString CClientDlg::GetFileExtension(CString fileName)
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

