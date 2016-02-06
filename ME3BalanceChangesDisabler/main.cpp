#include <windows.h>

BYTE pattern[] = { 0x4D, 0x45, 0x33, 0x5F, 0x42, 0x49, 0x4E, 0x49, 0x5F, 0x56, 0x45, 0x52, 0x53, 0x49, 0x4F, 0x4E };

bool DataCompare(const BYTE* OpCodes, const BYTE* Mask, const char* StrMask)  
{  
	while (*StrMask)  
	{  
		if(*StrMask == 'x' && *OpCodes != *Mask )  
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
		if(DataCompare((BYTE*)(StartAddress + i++), Mask, StrMask)) 
			++Ign;  
		else if (i>=CodeLen)  
			return 0;  
	} 
	return StartAddress + i - 1;  
}

DWORD WINAPI Start(LPVOID lpParam)
{
		DWORD ME3biniversionstringloc, dwProtect;
		ME3biniversionstringloc = FindPattern(0x401000, 0x1500000, pattern, "xxxxxxxxxxxxxxxx", 0);
		if(ME3biniversionstringloc)
		{
			VirtualProtect( (void*)ME3biniversionstringloc, 0x3, PAGE_READWRITE, &dwProtect );
			BYTE* p = (BYTE*)ME3biniversionstringloc;
			// turns it into "XXX_BINI_VERSION"
			*p++ = 'X';
			*p++ = 'X';
			*p = 'X';
			VirtualProtect( (void*)ME3biniversionstringloc, 0x3, dwProtect, &dwProtect );
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