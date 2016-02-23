#include <windows.h>

BYTE pattern1[] = { 0x8B, 0x81, 0x8C, 0x01, 0x00, 0x00, 0xC1, 0xE8, 0x0A, 0x83, 0xE0, 0x01, 0xC3 };
BYTE pattern2[] = { 0x55, 0x8B, 0xEC, 0x56, 0x68, 0x78, 0x53, 0x5A, 0x01, 0x68, 0x00, 0x00, 0x00, 0x03 };

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

bool RemoveOriginLink()
{
		DWORD pLoc1, pLoc2, dwProtect;
		pLoc1 = FindPattern(0x401000, 0x1500000, pattern1, "xxxxxxxxxxxxx", 0);
		pLoc2 = FindPattern(0x401000, 0x1500000, pattern2, "xxxxx????xxxxx", 0);
		if(!pLoc1 || !pLoc2)
			return false;
		VirtualProtect( (void*)pLoc1, 13, PAGE_READWRITE, &dwProtect );
		BYTE* p = (BYTE*)pLoc1;
		*p = 0xC7;
		p += 6;
		*p++ = 0x08;
		*p++ = 0x08;
		*p++ = 0;
		*p++ = 0;
		*p++ = 0x31;
		*p = 0xC0;
		VirtualProtect( (void*)pLoc1, 13, dwProtect, &dwProtect );
		VirtualProtect( (void*)pLoc2, 14, PAGE_READWRITE, &dwProtect );
		p = (BYTE*)pLoc2;
		*p++ = 0xB8;
		*p++ = 0;
		*p++ = 0;
		*p++ = 0;
		*p++ = 0;
		*p = 0xC3;
		VirtualProtect( (void*)pLoc2, 14, dwProtect, &dwProtect );
		return true;
}

BOOL WINAPI DllMain(HINSTANCE hInst,DWORD reason,LPVOID)
{
	if (reason == DLL_PROCESS_ATTACH)
	{			
		if(!RemoveOriginLink())
			return 0;
	}
	return 1;
}