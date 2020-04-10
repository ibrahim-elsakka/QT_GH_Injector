#pragma once
#include <string>
#include <vector>

#include "Injection.h"

enum err
{
	none,
	ok,
	wrong_usage,
	help,
	no_lib,
	no_lib_arg,
	no_func,
	no_file,
	file_path,
	no_process,
	no_process_arg,
	different_arch,
	inject_fail
};

enum args
{
	dll = 1,
	process,
	inject,
	launch,
	flags
};


int CmdArg(int argc, char* argv[]);


bool is_number(const std::string& s);
INJECTION_MODE getInjMode(std::string str);
LAUNCH_METHOD getLaunchMethod(std::string str);
