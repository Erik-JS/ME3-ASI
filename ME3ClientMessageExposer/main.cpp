#include "header.h"
#include <windows.h>
#include <stdio.h>

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
		printf("%ls\n", message->str);
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

void PatchGameMemory()
{
	unsigned long hold = NULL;

	VirtualProtect((void*)LOC_START, 8, PAGE_EXECUTE_READWRITE, &hold);
	*(BYTE*)(LOC_START) = 0xE9;
	*(DWORD*)(LOC_START+1) = (unsigned long)&ExposeMessageFunc - (LOC_START + 5);
	*(BYTE*)(LOC_START+5) = 0x90;
	*(BYTE*)(LOC_START+6) = 0x90;
	*(BYTE*)(LOC_START+7) = 0x90;
	VirtualProtect((void*)LOC_START, 8, hold, NULL);

	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	freopen ( "CON", "w", stdout ) ;
	printf("ME3 ClientMessage Exposer by Erik JS\n------------------------------------\n");
}

bool __stdcall DllMain(HANDLE process, DWORD reason, LPVOID lpReserved){
	if(reason == DLL_PROCESS_ATTACH){
		PatchGameMemory();
		return 1;
	}
	else
		return 0;
}
