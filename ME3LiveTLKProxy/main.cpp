#define _CRT_SECURE_NO_WARNINGS 1
#include <windows.h>
#include <stdio.h>

BYTE pattern[] = { 0xE8, 0xFE, 0x2F, 0x8E, 0xFF };

BYTE *tlkcontent = NULL;

struct TlkStruct
{
	BYTE *tlkData;
	DWORD tlkSize1;
	DWORD tlkSize2;
} *tlkstruct;

DWORD newval1, newval2, newval3;
DWORD backup1, backup2, backup3;

DWORD locStart = 0x00BBD44D;
DWORD locReturn = 0x00BBD452;

__declspec(naked) void UseNewTalkTable()
{
	__asm
	{
		pushad
		sub eax, 0x10
		mov tlkstruct, eax
	}

	backup1 = (DWORD)tlkstruct->tlkData;
	backup2 = tlkstruct->tlkSize1;
	backup3 = tlkstruct->tlkSize2;
	tlkstruct->tlkData = (BYTE*)newval1;
	tlkstruct->tlkSize1 = newval2;
	tlkstruct->tlkSize2 = newval3;

	__asm
	{
		popad
		mov eax, 0x4A0450
		call eax
		pushad
	}

	tlkstruct->tlkData = (BYTE*)backup1;
	tlkstruct->tlkSize1 = backup2;
	tlkstruct->tlkSize2 = backup3;

	__asm
	{
		popad
		jmp locReturn
	}
}


bool DataCompare(const BYTE* OpCodes, const BYTE* Mask, const char* StrMask)
{
	while (*StrMask)
	{
		if (*StrMask == 'x' && *OpCodes != *Mask)
			return false;
		++StrMask;
		++OpCodes;
		++Mask;
	}
	return true;
}

DWORD FindPattern(DWORD StartAddress, DWORD CodeLen, BYTE* Mask, char* StrMask, unsigned short ignore)
{
	unsigned short Ign = 0;
	DWORD i = 0;
	while (Ign <= ignore)
	{
		if (DataCompare((BYTE*)(StartAddress + i++), Mask, StrMask))
			++Ign;
		else if (i >= CodeLen)
			return 0;
	}
	return StartAddress + i - 1;
}

DWORD WINAPI Start(LPVOID lpParam)
{
	DWORD patternfound = FindPattern(0x400000, 0x800000, pattern, "xxxxx", 0);
	if (!patternfound)
	{
		return 0;
	}
	FILE *tlkfile = fopen("ServerTLK.tlk", "rb");
	if (!tlkfile)
	{
		tlkfile = fopen("ASI\\ServerTLK.tlk", "rb");
		if (!tlkfile)
		{
			return 0;
		}
	}

	fseek(tlkfile, 0, SEEK_END);
	int tlksize = ftell(tlkfile);
	tlkcontent = (BYTE*)malloc(tlksize);
	rewind(tlkfile);
	fread(tlkcontent, sizeof(BYTE), tlksize, tlkfile);
	fclose(tlkfile);

	newval1 = (DWORD)tlkcontent;
	newval2 = tlksize;
	newval3 = tlksize + 0x800;

	DWORD dwProtect;
	VirtualProtect((void*)locStart, 5, PAGE_READWRITE, &dwProtect);
	*(BYTE*)locStart = 0xE9;
	*(DWORD*)(locStart + 1) = (DWORD)&UseNewTalkTable - (locStart + 5);
	VirtualProtect((void*)locStart, 5, dwProtect, &dwProtect);
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		DWORD dwThreadId, dwThrdParam = 1;
		HANDLE hThread;
		hThread = CreateThread(NULL, 0, Start, &dwThrdParam, 0, &dwThreadId);
	}
	return 1;
}