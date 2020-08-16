#pragma once

#ifndef _DREAMLIFTER_H_
#define _DREAMLIFTER_H_

#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <winternl.h>
#include <wintrust.h>

#include <MinUmdfLoaderInterface.h>

#include <nt/NtRoutine.h>
#include <wdf/FxShared.h>
#include <wdf/km/MinKmdf0115.h>
#include <wdf/um/MinUmdf0215.h>
#include <wdf/UcmCx/MinUcmCx0100.h>

// TODO: separate this to a proper header file
typedef struct _DREAMLIFTER_DEVICE {
    // This can be further extended
    PFN_WDF_DEVICE_PREPARE_HARDWARE EvtDevicePrepareHardware;
    PCWDF_OBJECT_CONTEXT_TYPE_INFO DeviceContextInfo;
    PVOID DeviceContext;
    HANDLE SerializationMutex;
    PDREAMLIFTER_UCM_DEVICE UcmManagerInfo;
} DREAMLIFTER_DEVICE, * PDREAMLIFTER_DEVICE;

#define GUEST_DRIVER_NAME "DreamLifterGuest"
#define DL_KM_LOADER "--km"

int DlStartDriverHost();
inline void CpuDeadLoop()
{
    while (TRUE) { ; }
}

#endif
