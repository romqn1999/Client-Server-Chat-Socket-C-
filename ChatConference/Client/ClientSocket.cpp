#include "stdafx.h"
#include "ClientSocket.h"


ClientSocket::ClientSocket()
{
}


ClientSocket::~ClientSocket()
{
}

//BEGIN_MESSAGE_MAP(ClientSocket, CWnd)
//	ON_MESSAGE(WM_SOCKET, SockMsg)
//END_MESSAGE_MAP()

//Conect socket to server.
//Return 0 if success otherwise return -1.
int ClientSocket::Connect() {
	if (isLogon) {
		Close();
	}
	sClient = socket(AF_INET, SOCK_STREAM, 0);
	hostent* host = NULL;
	if (sClient == INVALID_SOCKET)
	{
		AfxMessageBox(_T("socket() failed"));
		return -1;
	}

	servAdd.sin_family = AF_INET;
	servAdd.sin_port = htons(PORT);

	char* cIP = ConvertToChar(IP);

	servAdd.sin_addr.s_addr = inet_addr(cIP);

	CStringA cpy_IP(IP);

	if (servAdd.sin_addr.s_addr == INADDR_NONE)
	{
		host = (gethostbyname(cpy_IP));
		if (host == NULL)
		{
			AfxMessageBox(_T("Khong the ket noi den server."));
		}
		CopyMemory(&servAdd.sin_addr, host->h_addr_list[0],
			host->h_length);
		return -1;
	}

	int err = connect(sClient, (struct sockaddr*)&servAdd, sizeof(servAdd));
	if (err == SOCKET_ERROR) {
		AfxMessageBox(_T("Ket noi that bai"));
		return -1;
	}

	WSAAsyncSelect(sClient, m_hWnd, WM_SOCKET, FD_READ | FD_CLOSE);
	isLogon = 1;

	delete cIP;

	return 0;
}

void ClientSocket::Split(CString src, CString des[2])
{
	int p1, p2;

	p1 = src.Find(_T("\r\n"), 0);
	des[0] = src.Mid(0, p1);

	p2 = src.Find(_T("\r\n"), p1 + 1);
	des[1] = src.Mid(p1 + 2, p2 - (p1 + 2));

}

void ClientSocket::Split(CString src, std::vector<CString> &des)
{
	int start = 0, pos = 0;
	des.clear();

	while ((pos = src.Find(strDelimiter, start)) != -1) {
		des.push_back(src.Mid(start, pos - start));
		start = pos + strDelimiter.GetLength();
	}

}

char* ClientSocket::ConvertToChar(const CString &s)
{
	int nSize = s.GetLength();
	char *pAnsiString = new char[nSize + 1];
	memset(pAnsiString, 0, nSize + 1);
	wcstombs(pAnsiString, s, nSize + 1);
	return pAnsiString;
}

void ClientSocket::mSend(CString Command)
{
	/*int Len = Command.GetLength();
	Len += Len;
	PBYTE sendBuff = new BYTE[MAX_BUFF_SIZE + 1000];
	memset(sendBuff, 0, MAX_BUFF_SIZE + 1000);
	memcpy(sendBuff, (PBYTE)(LPCTSTR)Command, Len);
	send(sClient, (char*)&Len, sizeof(Len), 0);
	send(sClient, (char*)sendBuff, Len, 0);*/

	int Len = (Command.GetLength() + 1) << 1;
	PBYTE sendBuff = new BYTE[MAX_BUFF_SIZE + 100];
	memcpy(sendBuff, (PBYTE)&Len, sizeof(Len));
	memcpy(sendBuff + sizeof(Len), (PBYTE)(LPCTSTR)Command, Len);
	Len += sizeof(Len);
	
	int sent = 0, sentSoFar;
	while (sent < Len) {
		sentSoFar = send(sClient, (char*)(sendBuff + sent), Len - sent, 0);
		if (sentSoFar != SOCKET_ERROR)
			sent += sentSoFar;
	}

	delete[] sendBuff;
}

int ClientSocket::mRecv(CString &Command)
{
	int received = 0, receivedSoFar;
	while (received < 4) {
		receivedSoFar = recv(sClient, (char*)(&buffLength + received), sizeof(int) - received, 0);
		if (receivedSoFar != SOCKET_ERROR)
			received += receivedSoFar;
	}
	//recv(sClient, (char*)&buffLength, sizeof(int), 0);
	PBYTE buffer = new BYTE[buffLength + 2];
	memset(buffer, 0, buffLength + 2);
	recv(sClient, (char*)buffer, buffLength, 0);

	TCHAR* ttc = (TCHAR*)buffer;
	Command = ttc;

	delete[] buffer;
	if (Command.GetLength() == 0)
		return -1;
	return 0;
}


void ClientSocket::Close() {
	if (isLogon)
		closesocket(sClient);
	isLogon = 0;
}


BOOL ClientSocket::SendFile(const CString &FileName, const CString &to)
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
		CFile::modeRead | CFile::typeBinary, &fe)))
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
	int lenInfo = sizeof(fileLength) + (fileName.GetLength() + 1) * 2 + (to.GetLength() + 1) * 2;
	memcpy(sendData + len, (PBYTE)&lenInfo, sizeof(lenInfo));							//size of file info
	len += sizeof(lenInfo);
	memcpy(sendData + len, (PBYTE)&fileLength, sizeof(fileLength));						//file size
	len += sizeof(fileLength);
	memcpy(sendData + len, (PBYTE)(LPCTSTR)fileName, (fileName.GetLength() + 1) * 2);	//file name include null-terminate
	len += (fileName.GetLength() + 1) * 2;
	memcpy(sendData + len, (PBYTE)(LPCTSTR)to, (to.GetLength() + 1) * 2);			//receiver include null-terminate
	len += (to.GetLength() + 1) * 2;

	//send(sClient, (char*)sendData, len, 0);
	cbLeftToSend = len;
	do
	{
		int cbBytesSent;
		cbBytesSent = send(sClient, (char*)sendData + (len - cbLeftToSend), cbLeftToSend, 0);

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
			doneSoFar = send(sClient, (char*)sendData + buffOffset, sendThisTime, 0);

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


int ClientSocket::mRecv(char* buf)
{
	int received = 0, m_BuffLen, receivedSoFar;
	while (received < 4) {
		receivedSoFar = recv(sClient, (char*)(&m_BuffLen + received), sizeof(m_BuffLen) - received, 0);
		if (receivedSoFar != SOCKET_ERROR)
			received += receivedSoFar;
	}

	received = 0;
	while (received < m_BuffLen) {
		receivedSoFar = recv(sClient, buf + received, m_BuffLen - received, 0);
		if (receivedSoFar != SOCKET_ERROR)
			received += receivedSoFar;
	}

	return m_BuffLen;
}