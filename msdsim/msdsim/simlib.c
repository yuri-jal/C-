/***********************************************************************

			  (c) Copyright 2020 ariatech, Inc.
						  All rights reserved.

   This program contains confidential and proprietary information  of
   ntelia, and any reproduction, disclosure, or use in whole or in part
   is expressly  prohibited, except as may be specifically authorized
   by prior written agreement or permission of ntelia.

************************************************************************

*======================================================================*
*  FILE : simlib.c
*  �ۼ� : ������ ������ (devismylife@ariatech.kr)
*======================================================================*/

#include "msdsim_c.h"
#include <string.h>

int recvData(SOCKET sock, BYTE *arBuf)
{
	// Protocol: Length@com@path[@type@name@isPointer@maxArrayIdx] * com
	// Length = 4byte, integer
	// otherwise = string
	int length = 0, recvLen = 0, ret;

	// get length
	ret = recv(sock, (char *)&length, sizeof(int), 0);
	
	// get data
	for (; length > 0; length -= recvLen)
	{
		ret = recvLen = recv(sock, &arBuf[recvLen], length, 0);
		if (ret == SOCKET_ERROR) break;
	}
	
	if (ret == SOCKET_ERROR && WSAGetLastError() == WSAETIMEDOUT)
	{
		MessageBox(g_hMainWindow, TEXT("������ ������ ���������ϴ�."), PROGRAM_TITLE, MB_OK | MB_ICONERROR);
		SendMessage(g_hMainWindow, WM_COMMAND, (WPARAM)MAKEWORD(ID_DISCONNECT, 0), (LPARAM)0);
	}

	return recvLen;
}

/*
arguments
 - _OUT_ tokenBuf: save token buffer
 - arBuf: recvData

return
 - The first offset where tokens were found in arBuf
 - but, If *arBuf is '\ 0', -1 is returned.
*/
int getToken(char *tokenBuf, const BYTE *arBuf)
{
	const char *p;

	p = strchr(arBuf, '@');
	if (p)
	{
		strncpy(tokenBuf, arBuf, p - arBuf);
		tokenBuf[p - arBuf] = '\0';
	}
	else
	{
		strcpy(tokenBuf, arBuf);
	}

	return p - arBuf;
}

void tcstombs(char *dest, TCHAR *origin)
{
#pragma warning(push)
#pragma warning(4:4133)
	*dest = '\0';
	if (sizeof(char) == sizeof(TCHAR))
	{
		strcpy(dest, origin);
	}
	else
	{
		//wcstombs(dest, origin, wcslen(origin));		
		wsprintfA(dest, "%S", origin);
	}
#pragma warning(pop)
}

void mbstotcs(TCHAR *dest, char *origin)
{
#pragma warning(push)
#pragma warning(4:4133)
	*dest = TEXT('\0');
	if (sizeof(char) == sizeof(TCHAR))
	{
		strcpy(dest, origin);
	}
	else
	{
		//mbstowcs(dest, origin, strlen(origin) * sizeof(TCHAR));
		wsprintfW(dest, L"%S", origin);
	}
#pragma warning(pop)
}

/*
arguments
 - dest: Member array's pointer
 - arBuf: recvData

return
 - count of member
*/
int createMemberList(Member **dest, const BYTE *arBuf)
{
	char tok[256];
	TCHAR tbuf[256], item[256];
	int ret, com, i;

	// 0. arBuf[0] == '@' => PASS
	arBuf++;

	// 1. get com & memory allocation
	ret = getToken(tok, arBuf);
	arBuf += ret + 1;
	com = atoi(tok);
	*dest = malloc(sizeof(Member) * com);
	
	// 2. get path & set path
	ret = getToken(tok, arBuf);
	arBuf += ret + 1;
	mbstotcs(tbuf, tok);
	wsprintf(item, TEXT("PATH: %s"), tbuf);
	SetWindowText(g_hPath, item);

	// 3. loop => set Member Struct & add List
	for (i = 0; i < com; i++)
	{
		// 3-1. type get
		ret = getToken(tok, arBuf);
		arBuf += ret + 1;
		mbstotcs(tbuf, tok);
		lstrcpy((*dest)[i].type, tbuf);

		// 3-2. name get
		ret = getToken(tok, arBuf);
		arBuf += ret + 1;
		mbstotcs(tbuf, tok);
		lstrcpy((*dest)[i].name, tbuf);

		// 3-3. isPointer get
		ret = getToken(tok, arBuf);
		arBuf += ret + 1;
		(*dest)[i].isPointer = !!(*tok - '0');

		// 3-4. maxArrayIdx get
		ret = getToken(tok, arBuf);
		arBuf += ret + 1;
		(*dest)[i].maxArrayIdx = atoi(tok);

		wsprintf(item, TEXT("%-32s%s"), (*dest)[i].type, (*dest)[i].name);
		SendMessage(g_hMemberList, LB_ADDSTRING, (WPARAM)0, (LPARAM)item);
	}
	updateMemberList(*dest, com);

	return com;
}

void updateMemberList(const Member *arMem, int com)
{
	int i;
	TCHAR line[256], buf[32], temp[32];
	LPARAM data;

	// clear list
	SendMessage(g_hMemberList, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);

	// list update
	for (i = 0; i < com; i++)
	{
		wsprintf(buf, TEXT("%s%s%s"), 
			arMem[i].maxArrayIdx == 0 ? TEXT("") : TEXT("["),
			arMem[i].maxArrayIdx == 0 ? TEXT("") : _itot(arMem[i].maxArrayIdx, temp, 10),
			arMem[i].maxArrayIdx == 0 ? TEXT("") : TEXT("]")
			);
		wsprintf(line, TEXT("%-32s%s%s%s"),
			arMem[i].type,
			arMem[i].isPointer ? "*" : "",
			arMem[i].name,
			buf
		);
		SendMessage(g_hMemberList, LB_INSERTSTRING, (WPARAM)i, (LPARAM)line);
		// ���� 1��Ʈ: isPointer(TRUE/FALSE), ���� 31��Ʈ: �迭 ũ��
		data = (arMem[i].isPointer ? 1 << 31 : 0) | arMem[i].maxArrayIdx;
		SendMessage(g_hMemberList, LB_SETITEMDATA, (WPARAM)i, (LPARAM)data);
	}
}

void deleteMemberList(Member **p)
{
	free(*p);
	*p = NULL;
}

static LPARAM getBiggestDataFromSelectedList(void)
{
	LPARAM big = -1, data; // Minimum = 0, Maximum = MAXLONG_PTR
	int cnt, i;

	cnt = SendMessage(g_hSelectedList, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
	for (i = 0; i < cnt; i++)
	{
		data = SendMessage(g_hSelectedList, LB_GETITEMDATA, (WPARAM)i, (LPARAM)0);
		if (big < data) big = data;
	}

	return big;
}

void updateSelectedList(char *buf)
{
	TCHAR tbuf[256] = TEXT("");
	int cnt = 0, i;
	LPARAM itemdata = -1;

	// ���� ���ڿ��� = �������� �ڸ�
	mbstotcs(tbuf, buf);
	_tcstok(tbuf, TEXT("="));

	// ���� �׸�� �ߺ��Ǵ��� Ȯ��
	i = SendMessage(g_hSelectedList, LB_FINDSTRING, -1, (LPARAM)tbuf);
	if (i != LB_ERR)
	{
		itemdata = SendMessage(g_hSelectedList, LB_GETITEMDATA, i, 0);
		SendMessage(g_hSelectedList, LB_DELETESTRING, i, 0);
	}
	else
	{
		itemdata = getBiggestDataFromSelectedList() + 1;
	}

	// ����, ���� (�ű� �׸��̸� �ܼ� ����)
	mbstotcs(tbuf, buf);
	cnt = SendMessage(g_hSelectedList, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
	SendMessage(g_hSelectedList, LB_INSERTSTRING, (WPARAM)cnt, (LPARAM)tbuf);
	SendMessage(g_hSelectedList, LB_SETITEMDATA, (WPARAM)cnt, itemdata);
}

/*
return
 - ���� �� ����Ʈ �ڽ� �׸��� ������ ������, �ƹ��͵� �������� ���� ��� -1
*/
int deleteSelected(void)
{
	int idx, cnt, loop;
	LPARAM data, ret;

	// Ŀ���� ��ġ�� �о��
	idx = SendMessage(g_hSelectedList, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
	if (idx < 0) return -1;

	// �׸� �����͸� ���� �� ����
	data = SendMessage(g_hSelectedList, LB_GETITEMDATA, (WPARAM)idx, (LPARAM)0);
	SendMessage(g_hSelectedList, LB_DELETESTRING, (WPARAM)idx, (LPARAM)0);

	// �� �׸� �����ͺ��� ū �׸� �����͸� 1�� ����
	cnt = SendMessage(g_hSelectedList, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
	for (loop = 0; loop < cnt; loop++)
	{
		ret = SendMessage(g_hSelectedList, LB_GETITEMDATA, (WPARAM)loop, (LPARAM)0);
		if (ret > data)
		{
			SendMessage(g_hSelectedList, LB_SETITEMDATA, (WPARAM)loop, (LPARAM)(ret - 1));
		}
	}

	return (int)data;
}

void clearSelectedList(void)
{
	SendMessage(g_hSelectedList, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
}

void debugf(BOOL isClear, const TCHAR *format, ...)
{
#ifdef DEBUG_MODE
	TCHAR buf[2048] = TEXT("");
	va_list ap;
	int len = 0;

	if (isClear == FALSE)
	{
		len = GetWindowText(g_hDebugStatic, buf, 2048);
	}

	va_start(ap, format);
	wvsprintf(&buf[len], format, ap);
	SetWindowText(g_hDebugStatic, buf);
	va_end(ap);
#endif
}