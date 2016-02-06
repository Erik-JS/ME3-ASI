#include <windows.h>
#include <stdio.h>

#pragma pack(1)

FILE* Log;

struct ErrorClass
{
	DWORD *vtable;
	DWORD unknA;
	DWORD unknB;
};

// Mimicked function declaration to make the compiler generate the correct var_arg asm for us:
void __cdecl LogPrintf(ErrorClass* error, wchar_t* pFormat, ...)
{
	// Prepend status of the unknown error class:
	//fprintf(Log, "[%s, %s] ", error->unknA ? "true" : "false", error->unknB ? "true" : "false");

	va_list args;
	va_start(args, pFormat);
	vfwprintf(Log, pFormat, args);
	va_end(args);

	fprintf(Log, "\n");
	fflush(Log);
}

int return_addr_one = 0;
int return_addr_two = 0;
int return_addr_three = 0;

// inline asm version, needed to get stack frame:
void __declspec(naked) LogPrintf_ASM(void)
{
	__asm
	{
		push ebp
		mov ebp, esp			// fetch stack frame
		push eax				// backup eax

		mov eax, [ebp + 0x4]		// fetch old stackframe's return pointers for easy printing below
		mov return_addr_one, eax
		mov eax, [ebp]
		mov eax, [eax + 0x4]
		mov return_addr_two, eax
		mov eax, [ebp]
		mov eax, [eax]
		mov eax, [eax + 0x4]
		mov return_addr_three, eax

		lea eax, [ebp + 0x10]	// forward the va_list
		push eax
		mov eax, [ebp + 0xC]	// forward the format  string
		push eax
		mov eax, Log			// set print target to our log
		push eax

		call vfwprintf
		add esp, 0xC			// clean up stack

	}

	fprintf(Log, " | Stacktrace: ME3Logger <- [%#x] <- [%#x] <- [%#x] \n", 
		return_addr_one, return_addr_two, return_addr_three);			// print and flush
	fflush(Log);

	_asm {
		pop eax					// restore
		pop ebp
		retn
	}
}

void DetourPrintFunction()
{
	BYTE *BioDiagnosticPrintf = (BYTE *)0x00467920;

	DWORD OriginalProtection;
	VirtualProtect(BioDiagnosticPrintf, 0x5, PAGE_READWRITE, &OriginalProtection);

	// calculate relative jump offset
	DWORD JmpOffset = (DWORD)((DWORD)LogPrintf_ASM - (DWORD)BioDiagnosticPrintf) - 5;

	*BioDiagnosticPrintf = 0xE9;					// write the long relative jmp
	memcpy(BioDiagnosticPrintf + 1, &JmpOffset, 4); // copy the destination adress

	VirtualProtect(BioDiagnosticPrintf, 0x5, OriginalProtection, &OriginalProtection);
}

DWORD WINAPI Start(LPVOID lpParam)
{

	// Start the debug logging.
	fopen_s(&Log, "ME3Log.txt", "w");
	fprintf(Log, "ME3Log - Logging started.\n");
	fflush(Log);

	DetourPrintFunction();

	return 0;
}

void Cleanup()
{
	fprintf(Log, "Shutting down, clean exit.\n");
	fclose(Log);
}

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		DWORD dwThreadId, dwThrdParam = 1;
		HANDLE hThread;
		hThread = CreateThread(NULL, 0, Start, &dwThrdParam, 0, &dwThreadId);
	}
	if (reason == DLL_PROCESS_DETACH)
	{
		Cleanup();
	}

	return 1;
}
