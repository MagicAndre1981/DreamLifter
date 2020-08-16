#pragma once

#pragma warning(disable:4214)
typedef struct _RELOCATION
{
    WORD    Offset : 12;
    WORD    Type : 4;
} RELOCATION, *PRELOCATION;
#pragma warning(default:4214)

typedef struct _DRIVER_MODULE {
    PFN_WDM_DRIVER_ENTRY Entry;
    HMODULE Module;
    DWORD ModuleSize;
} DRIVER_MODULE, *PDRIVER_MODULE;

typedef struct _DL_FUNCTION_DESCRIPTOR {
    DWORD Version;
    CHAR FunctionName[_MAX_PATH];
    PVOID Entry;
} DL_FUNCTION_DESCRIPTOR, *PDL_FUNCTION_DESCRIPTOR;

typedef struct _DL_MODULE_IMPLEMENTATION {
    DWORD Version;
    CHAR ModuleName[_MAX_PATH];
    PDL_FUNCTION_DESCRIPTOR Functions;
} DL_MODULE_IMPLEMENTATION, *PDL_MODULE_IMPLEMENTATION;

#define MODULE_IMPL_DESCRIPTOR_V1 1
#define FUNCTION_IMPL_DESCRIPTOR_V1 1

PDRIVER_MODULE DlKmLoadModule();
