#pragma once
#ifndef _DREAMLIFTER_H_
#define _DREAMLIFTER_H_

#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

#include <MinUmdfLoaderInterface.h>
#include <wdf/um/UmdfDriverEnums0215.h>
#include <wdf/um/UcmFuncEnums0100.h>

#define GUEST_DRIVER_NAME "DreamLifterGuest"

#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)    // ntsubauth
#define STATUS_INSUFFICIENT_RESOURCES    ((NTSTATUS)0xC000009AL)
#define STATUS_NOT_SUPPORTED             ((NTSTATUS)0xC00000BBL)

NTSTATUS DlUmLoadVersionLib(
    _In_ PVOID Context,
    _In_ PWDF_BIND_INFO BindInfo,
    _In_ PWDF_DRIVER_GLOBALS* DriverGlobals
);

NTSTATUS DlUmLoadExtensionClass(
    _In_ PVOID Context,
    _In_ PWDF_BIND_INFO BindInfo,
    _In_ PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_ PWDF_CLASS_EXTENSION ClassExtensionInfo
);

PVOID g_WdfFunctions0215[WdfFunctionTableNumEntries];

PVOID g_UcmFunctions0100[UcmFunctionTableNumEntries];

#endif
