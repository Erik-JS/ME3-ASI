#include <windows.h>

BYTE pattern[] = { 0x3D, 0xB7, 0x00, 0x00, 0x00, 0x74, 0x27 };

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
	DWORD target = FindPattern(0x400000, 0x800000, pattern, "xxxxxxx", 0);
	if (!target)
		return 0;
	target += 5;
	DWORD originalProtection;
	VirtualProtect((void*)target, 2, PAGE_EXECUTE_READWRITE, &originalProtection);
	*(WORD*)target = 0x9090;
	VirtualProtect((void*)target, 2, originalProtection, NULL);
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