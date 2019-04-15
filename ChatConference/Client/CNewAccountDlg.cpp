// CNewAccountDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Client.h"
#include "CNewAccountDlg.h"
#include "afxdialogex.h"


// CNewAccountDlg dialog

IMPLEMENT_DYNAMIC(CNewAccountDlg, CDialogEx)

CNewAccountDlg::CNewAccountDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_NEWACCOUNT_DLG, pParent)
	, m_Username(_T(""))
	, m_Password(_T(""))
	, m_ConfirmPassword(_T(""))
	, m_serverIP(_T("127.0.0.1"))
{
}

CNewAccountDlg::~CNewAccountDlg()
{
}

void CNewAccountDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_USERNAME, m_EditUsername);
	DDX_Text(pDX, IDC_USERNAME, m_Username);
	DDX_Control(pDX, IDC_PASSWORD, m_EditPassword);
	DDX_Text(pDX, IDC_PASSWORD, m_Password);
	DDX_Text(pDX, IDC_CONFIRM_PASSWORD, m_ConfirmPassword);
	DDX_Text(pDX, IDC_EDIT_IP, m_serverIP);
}


BEGIN_MESSAGE_MAP(CNewAccountDlg, CDialogEx)
	ON_MESSAGE(WM_SOCKET, SockMsg)
	ON_BN_CLICKED(IDOK, &CNewAccountDlg::OnBnClickedOk)
	ON_EN_CHANGE(IDC_EDIT_IP, &CNewAccountDlg::OnEnChangeEditIp)
END_MESSAGE_MAP()


// CNewAccountDlg message handlers

void CNewAccountDlg::Split(CString src, std::vector<CString> &des)
{
	int start = 0, pos = 0;
	des.clear();

	while ((pos = src.Find(strDelimiter, start)) != -1) {
		des.push_back(src.Mid(start, pos - start));
		start = pos + strDelimiter.GetLength();
	}

}

LRESULT CNewAccountDlg::SockMsg(WPARAM wParam, LPARAM lParam)
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
		case FLAG_REGISTRATION:
		{
			int flag2 = _ttoi(strResult[1]);
			if (flag2 == 1) {
				AfxMessageBox(_T("Account successfully created."), MB_OK | MB_ICONASTERISK);
				OnCancel();
			}
			else
			{
				AfxMessageBox(_T("Username already exists"));
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

void CNewAccountDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	UpdateData();

	// Make sure the user provided a username
	if (m_Username == "")
	{
		AfxMessageBox(_T("You must provide a username"));
		m_EditUsername.SetFocus();
		return;
	}

	if (m_Username == SERVER_CHAT || m_Username == "Me")
	{
		CString info = m_Username + " is not a valid username, please use another name for your account";
		AfxMessageBox(info);
		m_EditUsername.SetFocus();
		return;
	}

	// Don't allow a blank password
	if (m_Password == "")
	{
		AfxMessageBox(_T("Blank passwords are not allowed\n"
			"Please provide a password"));
		m_EditPassword.SetFocus();
		return;
	}

	// The password and the blank password must be the same
	if (m_ConfirmPassword != m_Password)
	{
		AfxMessageBox(_T("The passwords you provided are not the same"));
		m_EditPassword.SetFocus();
		return;
	}

	sClient.Connect();
	sClient.SetHWND(m_hWnd);
	try {
		CString command = _T("0\r\n") + m_Username + _T("\r\n") + m_Password + _T("\r\n");
		sClient.mSend(command);
	}
	catch (...)
	{
		// Did something go wrong???
		AfxMessageBox(_T("Could not validate the credentials"));
	}

	UpdateData(FALSE);
	//CDialogEx::OnOK();
}


void CNewAccountDlg::changeIP(CString newIP)
{
	m_serverIP = newIP;
}

void CNewAccountDlg::OnEnChangeEditIp()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	UpdateData(TRUE);
	sClient.changeIP(m_serverIP);
}
