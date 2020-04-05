#include "CmdArg.hpp"


#include <algorithm>
#include <string>
#include <vector>
#include "Injection.h"
#include "Process.h"




int CmdArg(int argc, char* argv[])
{
	std::vector<std::string> arguments;
	arguments.reserve(10);

	

	if (argc < 2)
		return  err::none;
	
	if(argc <= 3)
		return err::wrong_usage;

	HINSTANCE hInjectionMod = LoadLibrary(GH_INJ_MOD_NAME);
	if (hInjectionMod == nullptr)
		return err::no_lib;

	
	auto InjectA = (f_InjectA)GetProcAddress(hInjectionMod, "InjectA");
	if (InjectA == nullptr)
		return err::no_func;

	INJECTIONDATAA data{ 0 };

	
	bool bDebug = SetDebugPrivilege(true);
	
	bool bPlatform = isCorrectPlatform();

	
	// Dll
	const ARCH fileArch = getFileArch(argv[args::dll]);
	if(fileArch == ARCH::NONE)
		return no_file;

	const int fileLength = GetFullPathNameA(argv[1], MAX_PATH * 2, data.szDllPath, nullptr);
	if (fileLength == 0)
		return err::file_path;

	
	// process
	Process_Struct procStruct{ 0 };
	if (is_number(argv[args::process]))
		procStruct = getProcessByPID(std::atoi(argv[args::process]));
	else
		procStruct = getProcessByName(argv[args::process]);

	
	if (procStruct.arch == NONE)
		return err::no_process;
		

	if (procStruct.arch != fileArch)
		return err::different_arch;
	
	data.ProcessID = procStruct.pid;

	
	// inject
	if (argc > 3)
		data.Mode = getInjMode(argv[args::inject]);

	
	// launch
	if (argc > 4)
		data.Method = getLaunchMethod(argv[args::launch]);

	// flags
	if (argc > 5)
		data.Flags = std::atoi(argv[args::flags]);
	else
		if (data.Mode == INJECTION_MODE::IM_ManualMap)
			data.Flags = MM_DEFAULT;


	data.GenerateErrorLog = true;

	int iInject = InjectA(&data);
	if (iInject != 0)
		return err::inject_fail;
	
	int i = 42;
	return  ok;
}


bool is_number(const std::string& s)
{
	std::string::const_iterator it = s.begin();
	while (it != s.end() && std::isdigit(*it)) ++it;
	return !s.empty() && it == s.end();
}

INJECTION_MODE getInjMode(std::string str)
{
	for (auto c : str)
		std::transform(str.begin(), str.end(), str.begin(), 
			[](unsigned char c) {return std::tolower(c); });

	
	if (str == "loadlib")
		return  INJECTION_MODE::IM_LoadLibraryExW;

	
	if (str == "ldr")
		return INJECTION_MODE::IM_LdrLoadDll;

	
	if (str == "ldrp")
		return INJECTION_MODE::IM_LdrpLoadDll;

	
	if (str == "manuel")
		return  INJECTION_MODE::IM_LdrLoadDll;

	
	return INJECTION_MODE::IM_LoadLibraryExW;	
}


LAUNCH_METHOD getLaunchMethod(std::string str)
{
	for (auto c : str)
		std::transform(str.begin(), str.end(), str.begin(),
			[](unsigned char c) {return std::tolower(c); });


	if (str == "create")
		return  LAUNCH_METHOD::LM_NtCreateThreadEx;


	if (str == "hijack")
		return LAUNCH_METHOD::LM_HijackThread;


	if (str == "hook")
		return LAUNCH_METHOD::LM_SetWindowsHookEx;


	if (str == "apc")
		return  LAUNCH_METHOD::LM_QueueUserAPC;


	return LAUNCH_METHOD::LM_NtCreateThreadEx;
}
