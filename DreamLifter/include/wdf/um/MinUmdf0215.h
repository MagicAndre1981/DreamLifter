#pragma once

#ifndef __MIN_UMDF_0215_H__
#define __MIN_UMDF_0215_H__

// Leave any future UMDF specific bits here
PVOID DlFxLdrGetWdfUmdf0215Functions();

NTSTATUS DlUmBindVersionLib(
    _In_ PVOID Context,
    _In_ PWDF_BIND_INFO BindInfo,
    _In_ PWDF_DRIVER_GLOBALS* DriverGlobals
);

NTSTATUS DlUmBindExtensionClass(
    _In_ PVOID Context,
    _In_ PWDF_BIND_INFO BindInfo,
    _In_ PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_ PWDF_CLASS_EXTENSION ClassExtensionInfo
);

NTSTATUS DlWdfUmFunctionImplStub();

void DlWdfRequestComplete(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFREQUEST Request,
    _In_
    NTSTATUS Status
);

NTSTATUS DlWdfDriverOpenParametersRegistryKey(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFDRIVER Driver,
    _In_
    ACCESS_MASK DesiredAccess,
    _In_opt_
    PWDF_OBJECT_ATTRIBUTES KeyAttributes,
    _Out_
    WDFKEY* Key
);

VOID DlWdfRegistryClose(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFKEY Key
);

NTSTATUS DlWdfRegistryQueryULong(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFKEY Key,
    _In_
    PCUNICODE_STRING ValueName,
    _Out_
    PULONG Value
);

#endif
