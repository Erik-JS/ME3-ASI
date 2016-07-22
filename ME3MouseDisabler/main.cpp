#include <windows.h>
#include <stdio.h>

DWORD IATloc;

BOOL __stdcall FakeGetCursorPos(LPPOINT lpPoint)
{
	ShowCursor(FALSE);
	return FALSE;
}

DWORD WINAPI Start(LPVOID lpParam)
{
	DWORD membase = (DWORD)GetModuleHandle(NULL);
	PIMAGE_DOS_HEADER doshdr = (PIMAGE_DOS_HEADER)membase;
	PIMAGE_NT_HEADERS nthdr = (PIMAGE_NT_HEADERS)(membase + doshdr->e_lfanew);
	WORD sectioncount = nthdr->FileHeader.NumberOfSections;
	PIMAGE_SECTION_HEADER sechdr = IMAGE_FIRST_SECTION(nthdr);
	char *sectionname;
	for(int i = 0; i < sectioncount; i++)
	{
		sectionname = (char*)sechdr[i].Name;
		if(strcmp(sectionname,".rdata") != 0)
			continue;
		IATloc = membase + sechdr[i].VirtualAddress;
		break;
	}
	HMODULE hUser32 = GetModuleHandle("User32.dll");
	int GetCursorPosLocation = (int)GetProcAddress(hUser32,"GetCursorPos");
	int *p = (int*)IATloc;
	for(int i = 0; i < 0x200000; i++)
	{
		if(p[i] != GetCursorPosLocation)
			continue;
		p[i] = (int)FakeGetCursorPos;
		break;
	}
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hInst,DWORD reason,LPVOID)
{
	if (reason == DLL_PROCESS_ATTACH)
	{			
		DWORD dwThreadId, dwThrdParam = 1;
		HANDLE hThread;
		hThread = CreateThread(NULL,0, Start, &dwThrdParam, 0, &dwThreadId);
	}
	return 1;
}