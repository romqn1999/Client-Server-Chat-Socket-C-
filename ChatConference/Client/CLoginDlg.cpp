// CLoginDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Client.h"
#include "CLoginDlg.h"
#include "afxdialogex.h"


// CLoginDlg dialog

IMPLEMENT_DYNAMIC(CLoginDlg, CDialogEx)

CLoginDlg::CLoginDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_LOGIN_DLG, pParent)
	, m_Username(_T(""))
	, m_Password(_T(""))
	, m_serverIP(_T("127.0.0.1"))
{
	
}

CLoginDlg::~CLoginDlg()
{
}

void CLoginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_USERNAME, m_EditUsername);
	DDX_Text(pDX, IDC_USERNAME, m_Username);
	DDX_Text(pDX, IDC_PASSWORD, m_Password);
	DDX_Text(pDX, IDC_EDIT_IP, m_serverIP);
}


BEGIN_MESSAGE_MAP(CLoginDlg, CDialogEx)
	ON_MESSAGE(WM_SOCKET, SockMsg)
	ON_BN_CLICKED(IDC_NEWACCOUNT_BTN, &CLoginDlg::OnBnClickedNewaccountBtn)
	ON_BN_CLICKED(IDOK, &CLoginDlg::OnBnClickedOk)
	ON_EN_CHANGE(IDC_EDIT_IP, &CLoginDlg::OnEnChangeEditIp)
END_MESSAGE_MAP()


// CLoginDlg message handlers


void CLoginDlg::Split(CString src, std::vector<CString> &des)
{
	int start = 0, pos = 0;
	des.clear();

	while ((pos = src.Find(strDelimiter, start)) != -1) {
		des.push_back(src.Mid(start, pos - start));
		start = pos + strDelimiter.GetLength();
	}

}

LRESULT CLoginDlg::SockMsg(WPARAM wParam, LPARAM lParam)
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
		CString temp;
		if (sClient.mRecv(temp) < 0)
			break;

		Split(temp, strResult);
		int flag1 = _ttoi(strResult[0]);
		switch (flag1)
		{
		case FLAG_LOGIN:
		{
			int flag2 = _ttoi(strResult[1]);
			if (flag2 == 1) {
				OnCancel();
				CClientDlg m_ClientDlg;
				m_ClientDlg.setUsername(strResult[2]);
				INT_PTR nResponse = m_ClientDlg.DoModal();
			}
			else
			{
				AfxMessageBox(_T("Invalid Credentials. Please try again"));
				this->m_EditUsername.SetFocus();
			}

			UpdateData(FALSE);
			break;
		}
		}
		break;
	}
	case FD_CLOSE:
	{
		sClient.Close();
		break;
	}

	}
	return 0;
}

void CLoginDlg::OnBnClickedNewaccountBtn()
{
	// TODO: Add your control notification handler code here
	CNewAccountDlg Dlg;
	Dlg.changeIP(m_serverIP);

	//sClient.Connect();
	Dlg.DoModal();
}


void CLoginDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	UpdateData();

	//char UsernameFromFile[20], PasswordFromFile[20];
	//FILE *fleCredentials;
	bool ValidLogin = false;

	if (m_Username == "")
	{
		AfxMessageBox(_T("You must provide a username and a password or click Cancel"));
		return;
	}
	if (m_Password == "")
	{
		AfxMessageBox(_T("Invalid Login"));
		return;
	}

	int err = 0;
	if (!sClient.islogon())
		err = sClient.Connect();
	if (err == -1)
		return;
	//WSAAsyncSelect(sClient.GetSocket(), m_hWnd, WM_SOCKET, FD_READ | FD_CLOSE);
	sClient.SetHWND(m_hWnd);
	try {
		CString command = _T("1\r\n") + m_Username + _T("\r\n") + m_Password + _T("\r\n");
		sClient.mSend(command);
	}
	catch (...)
	{
		AfxMessageBox(_T("Could not validate the credentials"));
	}

	UpdateData(FALSE);
	//CDialogEx::OnOK();
}


void CLoginDlg::OnEnChangeEditIp()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	UpdateData(TRUE);
	sClient.changeIP(m_serverIP);
}
