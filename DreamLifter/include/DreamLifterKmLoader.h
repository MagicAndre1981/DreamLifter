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

PDRIVER_MODULE DlKmLoadModule();
