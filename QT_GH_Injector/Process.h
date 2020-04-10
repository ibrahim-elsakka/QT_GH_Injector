#pragma once

#ifndef NT_FAIL
#define NT_FAIL(status) (status < 0)
#endif

#include <Windows.h>

enum ARCH
{
    NONE,
    X64,
    X86
};

struct Process_Struct
{
    unsigned long   pid;
    char            name[100];
    char            fullName[MAX_PATH];
    int             arch;
    int             session;
};

enum _PROCESSINFOCLASS
{
	ProcessSessionInformation	= 24
};
typedef _PROCESSINFOCLASS PROCESSINFOCLASS;

struct PROCESS_SESSION_INFORMATION
{
    ULONG SessionId;
};

using f_NtQueryInformationProcess = NTSTATUS(__stdcall *)
(
    HANDLE					hTargetProc,
    PROCESSINFOCLASS		PIC,
    void * pBuffer,
    ULONG					BufferSize,
    ULONG * SizeOut
);

enum ARCH getFileArch(const char* szDllFile);
enum ARCH getProcArch(const int pid);
int getProcSession(const int pid);
bool getProcFullPath(char* fullPath, int strSize, int pid);
Process_Struct getProcessByName(const char* name);
Process_Struct getProcessByPID(const int pid);
bool getProcessList(std::vector<Process_Struct>& pl);

bool SetDebugPrivilege(bool Enable);
bool isCorrectPlatform();
BOOL StartProcess(const char* szExeFile);