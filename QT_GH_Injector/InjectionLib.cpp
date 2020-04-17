#include "InjectionLib.hpp"


InjectionLib::InjectionLib()
    : hInjectionMod(nullptr), InjectA(nullptr), InjectW(nullptr),ValidateFunc(nullptr),RestoreFunc(nullptr)
{
    //Init();
}


InjectionLib::~InjectionLib()
{
    BOOL fFreeResult = FreeLibrary(hInjectionMod);
}

bool InjectionLib::Init()
{
    hInjectionMod = LoadLibrary(GH_INJ_MOD_NAME);
    if (hInjectionMod == nullptr)
        return false;

    InjectA = reinterpret_cast<f_InjectA>(GetProcAddress(hInjectionMod, "InjectA"));
    InjectW = reinterpret_cast<f_InjectW>(GetProcAddress(hInjectionMod, "InjectW"));
    ValidateFunc = reinterpret_cast<f_ValidateInjectionFunctions>(GetProcAddress(hInjectionMod, "ValidateInjectionFunctions"));
    RestoreFunc = reinterpret_cast<f_RestoreInjectionFunctions>(GetProcAddress(hInjectionMod, "RestoreInjectionFunctions"));

    if (InjectA == nullptr || InjectW == nullptr || ValidateFunc == nullptr || RestoreFunc == nullptr)
        return false;

    return true;
}

bool InjectionLib::LoadingStatus()
{
    if (hInjectionMod == nullptr)
        return false;
	
    if (InjectA == nullptr || InjectW == nullptr || ValidateFunc == nullptr || RestoreFunc == nullptr)
        return false;

    return true;
}

DWORD InjectionLib::InjectFuncA(INJECTIONDATAA* pData)
{
    if (!LoadingStatus())
        return 42;
	
	return InjectFuncA(pData);	
}

DWORD InjectionLib::InjectFuncW(INJECTIONDATAW* pData)
{
    if (!LoadingStatus())
        return 42;
	
    return InjectFuncW(pData);
}

void InjectionLib::ScanHook()
{
    std::vector<HookInfo> info;
    info.push_back({});
	
    DWORD err1, err2;
    ValidateFunc((DWORD)4912, err1, err2, info);

    printf("InjectionLib functions validated\n");

    UINT Changed = 0;
    for (auto i : info)
    {
        if (i.ChangeCount && !i.ErrorCode)
        {
            printf("Hook detected: %s.%s\n", i.ModulePath.c_str(), i.FunctionName.c_str());
            ++Changed;
        }
    }

    if (Changed)
    {
        printf("Restoring hooks...\n");
        RestoreFunc(GetCurrentProcessId(), err1, err2, info);
        printf("Hooks restored\n");
    }
    else
    {
        printf("No hooks found\n");
    }
}
