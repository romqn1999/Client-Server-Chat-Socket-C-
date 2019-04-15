#include "stdafx.h"
#include "ReceiveMessagesThread.h"
#include "Client.h"

IMPLEMENT_DYNCREATE(ReceiveMessagesThread, CWinThread)

ReceiveMessagesThread::ReceiveMessagesThread()
{
}


ReceiveMessagesThread::ReceiveMessagesThread(HWND hWnd, vector<CString> *strResult, File *fileInfo)
{
	m_pMainHWnd = hWnd;
	m_pStrResult = strResult;
	m_fileInfo = fileInfo;
}


ReceiveMessagesThread::~ReceiveMessagesThread()
{
}


BEGIN_MESSAGE_MAP(ReceiveMessagesThread, CWinThread)
	ON_THREAD_MESSAGE(WM_SOCKET, SockMsg)
END_MESSAGE_MAP()


BOOL ReceiveMessagesThread::InitInstance()
{
	CWinThread::InitInstance();
	return TRUE;
}


void ReceiveMessagesThread::SockMsg(WPARAM wParam, LPARAM lParam)
{
	CString temp;
	if (sClient.mRecv(temp) < 0)
		return;

	Split(temp, *m_pStrResult);
	if ((*m_pStrResult).size() == 0)
		return;

	int flag = _ttoi((*m_pStrResult)[0]);
	switch (flag)
	{
	case FLAG_ONLINE_LIST:
	{
		SendMessage(m_pMainHWnd, WM_SOCKET_READ, FLAG_ONLINE_LIST, 0);
		break;
	}
	case FLAG_MESSAGE:
	{
		SendMessage(m_pMainHWnd, WM_SOCKET_READ, FLAG_MESSAGE, 0);
		break;
	}
	case FLAG_TO_WHOM:
	{
		SendMessage(m_pMainHWnd, WM_SOCKET_READ, FLAG_TO_WHOM, 0);
		break;
	}
	case FLAG_LOGIN:
	{
		SendMessage(m_pMainHWnd, WM_SOCKET_READ, FLAG_LOGIN, 0);
		break;
	}
	case FLAG_LOGOUT:
	{
		SendMessage(m_pMainHWnd, WM_SOCKET_READ, FLAG_LOGOUT, 0);
		break;
	}
	case FLAG_FILELIST:
	{
		SendMessage(m_pMainHWnd, WM_SOCKET_READ, FLAG_FILELIST, 0);
		break;
	}
	case FLAG_FILEINFO:
	{
		sClient.mRecv(m_Buffer);

		memcpy(&m_fileInfo->m_FileSize, m_Buffer, sizeof(m_fileInfo->m_FileSize));
		int byteSizeRead = sizeof(m_fileInfo->m_FileSize);

		CString tmp;
		tmp = (LPCTSTR)(m_Buffer + byteSizeRead);
		byteSizeRead += ((tmp.GetLength() + 1) << 1);
		/*m_fileInfo->m_FileName = (LPCTSTR)(m_Buffer + byteSizeRead);
		byteSizeRead += ((m_fileInfo->m_FileName.GetLength() + 1) << 1);*/

		m_fileInfo->m_ToWhom = (LPCTSTR)(m_Buffer + byteSizeRead);

		m_fileInfo->m_SizeReceived = 0;

		openDestFile(m_fileInfo->m_PathFile);

		SendMessage(m_pMainHWnd, WM_SOCKET_READ, FLAG_FILEINFO, 0);
		break;
	}
	case FLAG_FILEDATA:
	{
		GetFileFromRemoteSender();
		break;
	}
	}
}


void ReceiveMessagesThread::Split(CString src, std::vector<CString> &des)
{
	int start = 0, pos = 0;
	des.clear();

	while ((pos = src.Find(strDelimiter, start)) != -1) {
		des.push_back(src.Mid(start, pos - start));
		start = pos + strDelimiter.GetLength();
	}
}


bool ReceiveMessagesThread::openDestFile(const CString &fileName)
{
	CFileException fe;
	// open/create target file that receives the transferred data
	if (!(m_fileInfo->m_bFileIsOpen = m_fileInfo->m_destFile.Open(fileName, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary, &fe)))
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


bool ReceiveMessagesThread::closeDestFile()
{
	if (m_fileInfo->m_bFileIsOpen) {
		m_fileInfo->m_destFile.Close();
		m_fileInfo->m_bFileIsOpen = false;
		return true;
	}

	return false;
}


BOOL ReceiveMessagesThread::GetFileFromRemoteSender()
{
	//Receive all bytes of current file chunk to m_Buffer
	int chunkSize = sClient.mRecv(m_Buffer);
	m_fileInfo->m_destFile.Write(m_Buffer, chunkSize);
	m_fileInfo->m_SizeReceived += chunkSize;

	// only close file if it's open (open might have failed above)
	if (m_fileInfo->m_SizeReceived == m_fileInfo->m_FileSize) {
		closeDestFile();
		SendMessage(m_pMainHWnd, WM_THREAD_FILE_ENDED, 0, 0);
		SendMessage(m_pMainHWnd, WM_SOCKET_READ, FLAG_FILEDATA, 0);
	}

	return TRUE;
}