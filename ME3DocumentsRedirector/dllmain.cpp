#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <ShlObj.h>
#include <stdio.h>

DWORD IATloc = 0;

wchar_t NewDocumentsFolder[MAX_PATH];

HRESULT WINAPI FakeSHGetFolderPathW(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPWSTR pszPath)
{
	if (csidl == 0x05)
	{
		wcscpy(pszPath, NewDocumentsFolder);
		return S_OK;
	}
	return SHGetFolderPathW(hwnd, csidl, hToken, dwFlags, pszPath);
}

DWORD WINAPI Start(LPVOID lpParam)
{
	char targetfolder[MAX_PATH];
	FILE* RedirectorCfgFile;
	fopen_s(&RedirectorCfgFile, "ASI\\ME3DocumentsRedirector.txt", "r");
	if (!RedirectorCfgFile)
	{
		fopen_s(&RedirectorCfgFile, "ME3DocumentsRedirector.txt", "r");
		if (!RedirectorCfgFile)
		{
			return 0;
		}
	}
	fscanf(RedirectorCfgFile, "%s", targetfolder);
	fclose(RedirectorCfgFile);
	if (strlen(targetfolder) == 0)
		return 0;
	mbstowcs(NewDocumentsFolder, targetfolder, MAX_PATH - 1);
	DWORD membase = (DWORD)GetModuleHandle(NULL);
	PIMAGE_DOS_HEADER doshdr = (PIMAGE_DOS_HEADER)membase;
	PIMAGE_NT_HEADERS nthdr = (PIMAGE_NT_HEADERS)(membase + doshdr->e_lfanew);
	WORD sectioncount = nthdr->FileHeader.NumberOfSections;
	PIMAGE_SECTION_HEADER sechdr = IMAGE_FIRST_SECTION(nthdr);
	char* sectionname;
	for (int i = 0; i < sectioncount; i++)
	{
		sectionname = (char*)sechdr[i].Name;
		if (strcmp(sectionname, ".rdata") != 0)
			continue;
		IATloc = membase + sechdr[i].VirtualAddress;
		break;
	}
	if (!IATloc)
		return 0;
	HMODULE hShell32 = GetModuleHandle(L"Shell32.dll");
	int OriginalSHGetFolderPathW = (int)GetProcAddress(hShell32, "SHGetFolderPathW");
	int* p = (int*)IATloc;
	for (int i = 0; i < 0x200000; i++)
	{
		if (p[i] != OriginalSHGetFolderPathW)
			continue;
		DWORD originalProtection;
		VirtualProtect(&p[i], 4, PAGE_EXECUTE_READWRITE, &originalProtection);
		p[i] = (int)FakeSHGetFolderPathW;
		VirtualProtect(&p[i], 4, originalProtection, NULL);
		break;
	}
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		DWORD dwThreadId, dwThrdParam = 1;
		HANDLE hThread;
		hThread = CreateThread(NULL, 0, Start, &dwThrdParam, 0, &dwThreadId);
	}
	return 1;
}

