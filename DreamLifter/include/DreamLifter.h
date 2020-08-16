#pragma once

#ifndef _DREAMLIFTER_H_
#define _DREAMLIFTER_H_

#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <winternl.h>
#include <wintrust.h>

#include <MinWdfLoaderInterface.h>

#include <nt/NtRoutine.h>
#include <wdf/FxShared.h>
#include <wdf/km/MinKmdf0115.h>
#include <wdf/um/MinUmdf0215.h>
#include <wdf/UcmCx/MinUcmCx0100.h>

#ifdef _MSC_VER 
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif

// TODO: separate this to a proper header file
typedef struct _DREAMLIFTER_DEVICE {
    DREAMLIFTER_WDF_OBJECT_HEADER Header;
    // Anything with potential context will have these two at front
    PCWDF_OBJECT_CONTEXT_TYPE_INFO DeviceContextInfo;
    PVOID DeviceContext;
    // This can be further extended
    PFN_WDF_DEVICE_PREPARE_HARDWARE EvtDevicePrepareHardware;
    HANDLE SerializationMutex;
    PDREAMLIFTER_UCM_DEVICE UcmManagerInfo;
} DREAMLIFTER_DEVICE, * PDREAMLIFTER_DEVICE;

#define GUEST_DRIVER_NAME "DreamLifterGuest"
#define GUEST_DRIVER_NAME_KM "DreamLifterGuestKm"
#define DL_KM_LOADER "--km"

int DlStartUmHost();
int DlStartKmHost();

inline void CpuDeadLoop()
{
    while (TRUE) { ; }
}

inline void TrapDebugger(char* Message)
{
    printf(Message);
    OutputDebugStringA(Message);

    if (IsDebuggerPresent())
    {
        DebugBreak();
    }
}

#endif
