#include "header.h"
#include <windows.h>
#include "resource.h"
#include <Richedit.h>
#include <string.h>

struct LogWindowStruct {
	HWND hWindow;
	HWND hRichEdit;
} logWindow;

HINSTANCE hDLL;

#define LOC_START 0x452B01
#define LOC_EXIT 0x452B09

void * pointer;
int stringHeader;
int var1;

struct MsgStruct
{
	wchar_t * str;
	int size;
} *message;

void LogAppendText(LPCWSTR text)
{
	int currentlen = GetWindowTextLength(logWindow.hRichEdit);
	SendMessage(logWindow.hRichEdit, EM_SETSEL, (WPARAM)currentlen, (LPARAM)currentlen);
	SendMessage(logWindow.hRichEdit, EM_REPLACESEL, 0, (LPARAM)text);
	SendMessage(logWindow.hRichEdit, EM_REPLACESEL, 0, (LPARAM)L"\n");
	SendMessage(logWindow.hRichEdit, WM_VSCROLL, SB_BOTTOM, 0);
}

bool GetLocation(int * p)
{
	int intLoc = *(int*)0x01AB5634;
	intLoc = *(int*)(intLoc + 7747 * 4);
	//printf("intLoc: %p\n", (void*)intLoc);
	*p = intLoc;
	return true;
}

__declspec(naked) void ExposeMessageFunc()
{
	__asm
	{
		mov pointer,esi
		mov stringHeader,esp
		pushad
	}

	if(GetLocation(&var1) && pointer == (void*)var1)
	{
		//printf("pointer: %p ; var1: %p\n", pointer, (void*)var1);
		message = (MsgStruct*)stringHeader;
		LogAppendText(message->str);
	}


	__asm
	{
		popad
		mov eax,[esi]
		mov edx,[eax+0x000001A0]
		mov eax, LOC_EXIT
		jmp eax
	}
}

BOOL CALLBACK LogWindowProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_INITDIALOG:
		return TRUE;
	//case WM_COMMAND:
	//	switch (LOWORD(wParam))
	//	{
	//	case IDOK:
	//		EndDialog(hwnd, IDOK);
	//		break;
	//	case IDCANCEL:
	//		EndDialog(hwnd, IDCANCEL);
	//		break;
	//	}
	//	break;
	default:
		return FALSE;
	}
	return TRUE;
}

DWORD WINAPI LogWindowThread(LPVOID lpParam)
{
	LoadLibrary(L"riched20.dll");
	MSG msg;
	logWindow.hWindow = CreateDialog(hDLL, MAKEINTRESOURCE(IDD_DIALOG1), 0, LogWindowProc);
	logWindow.hRichEdit = GetDlgItem(logWindow.hWindow, IDC_RICHEDIT21);
	CHARFORMAT cf;
	cf.cbSize = sizeof(CHARFORMAT);
	cf.dwMask = CFM_FACE | CFM_SIZE;
	cf.yHeight = 200;
	wcscpy_s(cf.szFaceName, sizeof(cf.szFaceName) / sizeof(WCHAR), L"Consolas");
	SendMessage(logWindow.hRichEdit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
	ShowWindow(logWindow.hWindow, SW_NORMAL);
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!IsDialogMessage(logWindow.hWindow, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return 1;
}

DWORD WINAPI Start(LPVOID lpParam)
{
	DWORD hold = NULL;

	VirtualProtect((void*)LOC_START, 8, PAGE_EXECUTE_READWRITE, &hold);
	*(BYTE*)(LOC_START) = 0xE9;
	*(DWORD*)(LOC_START+1) = (unsigned long)&ExposeMessageFunc - (LOC_START + 5);
	*(BYTE*)(LOC_START+5) = 0x90;
	*(BYTE*)(LOC_START+6) = 0x90;
	*(BYTE*)(LOC_START+7) = 0x90;
	VirtualProtect((void*)LOC_START, 8, hold, NULL);

	CreateThread(0, NULL, LogWindowThread, NULL, NULL, NULL);

	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hDLL = hinstDLL;
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		DWORD dwThreadId, dwThrdParam = 1;
		HANDLE hThread;
		hThread = CreateThread(NULL, 0, Start, &dwThrdParam, 0, &dwThreadId);
	}
	return 1;
}
