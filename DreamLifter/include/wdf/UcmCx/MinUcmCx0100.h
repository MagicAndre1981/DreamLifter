#pragma once

#ifndef __MIN_UCMCX_0100_H__
#define __MIN_UCMCX_0100_H__

#include "UcmGlobals.h"
#include "UcmTypes.h"

DECLARE_HANDLE(UCMCONNECTOR);

typedef
NTSTATUS
EVT_UCM_CONNECTOR_SET_DATA_ROLE(
    _In_
    UCMCONNECTOR Connector,
    _In_
    UCM_DATA_ROLE DataRole
);

typedef EVT_UCM_CONNECTOR_SET_DATA_ROLE* PFN_UCM_CONNECTOR_SET_DATA_ROLE;

typedef
NTSTATUS
EVT_UCM_CONNECTOR_SET_POWER_ROLE(
    _In_
    UCMCONNECTOR Connector,
    _In_
    UCM_POWER_ROLE PowerRole
);

typedef EVT_UCM_CONNECTOR_SET_POWER_ROLE* PFN_UCM_CONNECTOR_SET_POWER_ROLE;

typedef struct _UCM_MANAGER_CONFIG
{
    ULONG               Size;
} UCM_MANAGER_CONFIG, * PUCM_MANAGER_CONFIG;


VOID
FORCEINLINE
UCM_MANAGER_CONFIG_INIT(
    _Out_ PUCM_MANAGER_CONFIG Config
)
{
    RtlZeroMemory(Config, sizeof(*Config));
    Config->Size = sizeof(*Config);
}

typedef struct _UCM_CONNECTOR_TYPEC_CONFIG
{
    ULONG                           Size;
    BOOLEAN                         IsSupported;
    ULONG                           SupportedOperatingModes;
    ULONG                           SupportedPowerSourcingCapabilities;
    BOOLEAN                         AudioAccessoryCapable;
    PFN_UCM_CONNECTOR_SET_DATA_ROLE EvtSetDataRole;
} UCM_CONNECTOR_TYPEC_CONFIG, * PUCM_CONNECTOR_TYPEC_CONFIG;

VOID
FORCEINLINE
UCM_CONNECTOR_TYPEC_CONFIG_INIT(
    _Out_ PUCM_CONNECTOR_TYPEC_CONFIG Config,
    _In_ ULONG SupportedOperatingModes, // Bitwise-OR of UCM_TYPEC_OPERATING_MODE
    _In_ ULONG SupportedPowerSourcingCapabilities   // Bitwise-OR of UCM_TYPEC_CURRENT
)
{
    RtlZeroMemory(Config, sizeof(*Config));
    Config->Size = sizeof(*Config);
    Config->IsSupported = TRUE;
    Config->SupportedOperatingModes = SupportedOperatingModes;
    Config->SupportedPowerSourcingCapabilities = SupportedPowerSourcingCapabilities;
}


typedef struct _UCM_CONNECTOR_PD_CONFIG
{
    ULONG       Size;
    BOOLEAN     IsSupported;
    ULONG       SupportedPowerRoles;    // Bitwise-OR of UCM_POWER_ROLE
    PFN_UCM_CONNECTOR_SET_POWER_ROLE    EvtSetPowerRole;
} UCM_CONNECTOR_PD_CONFIG, * PUCM_CONNECTOR_PD_CONFIG;


VOID
FORCEINLINE
UCM_CONNECTOR_PD_CONFIG_INIT(
    _Out_ PUCM_CONNECTOR_PD_CONFIG Config,
    _In_ ULONG SupportedPowerRoles
)
{
    RtlZeroMemory(Config, sizeof(*Config));
    Config->Size = sizeof(*Config);
    Config->IsSupported = TRUE;
    Config->SupportedPowerRoles = SupportedPowerRoles;
}

typedef struct _UCM_CONNECTOR_CONFIG
{
    ULONG                       Size;
    ULONGLONG                   ConnectorId;
    PUCM_CONNECTOR_TYPEC_CONFIG TypeCConfig;
    PUCM_CONNECTOR_PD_CONFIG    PdConfig;
} UCM_CONNECTOR_CONFIG, * PUCM_CONNECTOR_CONFIG;


VOID
FORCEINLINE
UCM_CONNECTOR_CONFIG_INIT(
    _Out_ PUCM_CONNECTOR_CONFIG Config,
    _In_ ULONGLONG ConnectorId
)
{
    RtlZeroMemory(Config, sizeof(*Config));
    Config->Size = sizeof(*Config);
    Config->ConnectorId = ConnectorId;
}


typedef struct _UCM_CONNECTOR_TYPEC_ATTACH_PARAMS
{
    ULONG               Size;
    UCM_TYPEC_PARTNER   Partner;
    UCM_TYPEC_CURRENT   CurrentAdvertisement;
    UCM_CHARGING_STATE  ChargingState;
} UCM_CONNECTOR_TYPEC_ATTACH_PARAMS, *PUCM_CONNECTOR_TYPEC_ATTACH_PARAMS;


VOID
FORCEINLINE
UCM_CONNECTOR_TYPEC_ATTACH_PARAMS_INIT(
    _Out_ PUCM_CONNECTOR_TYPEC_ATTACH_PARAMS Params,
    _In_ UCM_TYPEC_PARTNER Partner
)
{
    RtlZeroMemory(Params, sizeof(*Params));
    Params->Size = sizeof(*Params);
    Params->Partner = Partner;
}


typedef struct _UCM_CONNECTOR_PD_CONN_STATE_CHANGED_PARAMS
{
    ULONG                       Size;
    UCM_PD_CONN_STATE           PdConnState;
    UCM_PD_REQUEST_DATA_OBJECT  Rdo;
    UCM_CHARGING_STATE          ChargingState;
} UCM_CONNECTOR_PD_CONN_STATE_CHANGED_PARAMS, * PUCM_CONNECTOR_PD_CONN_STATE_CHANGED_PARAMS;


VOID
FORCEINLINE
UCM_CONNECTOR_PD_CONN_STATE_CHANGED_PARAMS_INIT(
    _Out_ PUCM_CONNECTOR_PD_CONN_STATE_CHANGED_PARAMS Params,
    _In_ UCM_PD_CONN_STATE PdConnState
)
{
    RtlZeroMemory(Params, sizeof(*Params));
    Params->Size = sizeof(*Params);
    Params->PdConnState = PdConnState;
}

//
// UCM Function: UcmInitializeDevice
//
typedef
_Must_inspect_result_
_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS
(*PFN_UCMINITIALIZEDEVICE)(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFDEVICE WdfDevice,
    _In_
    PUCM_MANAGER_CONFIG Config
    );

//
// UCM Function: UcmConnectorCreate
//
typedef
_Must_inspect_result_
_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS
(*PFN_UCMCONNECTORCREATE)(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFDEVICE WdfDevice,
    _In_
    PUCM_CONNECTOR_CONFIG Config,
    _In_opt_
    PWDF_OBJECT_ATTRIBUTES Attributes,
    _Out_
    UCMCONNECTOR* Connector
    );

//
// UCM Function: UcmConnectorTypeCAttach
//
typedef
_Must_inspect_result_
_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS
(*PFN_UCMCONNECTORTYPECATTACH)(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    UCMCONNECTOR Connector,
    _In_
    PUCM_CONNECTOR_TYPEC_ATTACH_PARAMS Params
    );

//
// UCM Function: UcmConnectorTypeCDetach
//
typedef
_Must_inspect_result_
_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS
(*PFN_UCMCONNECTORTYPECDETACH)(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    UCMCONNECTOR Connector
    );

//
// UCM Function: UcmConnectorTypeCCurrentAdChanged
//
typedef
_Must_inspect_result_
_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS
(*PFN_UCMCONNECTORTYPECCURRENTADCHANGED)(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    UCMCONNECTOR Connector,
    _In_
    UCM_TYPEC_CURRENT CurrentAdvertisement
    );

//
// UCM Function: UcmConnectorPdSourceCaps
//
typedef
_Must_inspect_result_
_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS
(*PFN_UCMCONNECTORPDSOURCECAPS)(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    UCMCONNECTOR Connector,
    _In_
    UCM_PD_POWER_DATA_OBJECT Pdos[],
    _In_
    UCHAR PdoCount
    );

//
// UCM Function: UcmConnectorPdPartnerSourceCaps
//
typedef
_Must_inspect_result_
_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS
(*PFN_UCMCONNECTORPDPARTNERSOURCECAPS)(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    UCMCONNECTOR Connector,
    _In_
    UCM_PD_POWER_DATA_OBJECT Pdos[],
    _In_
    UCHAR PdoCount
    );

//
// UCM Function: UcmConnectorPdConnectionStateChanged
//
typedef
_Must_inspect_result_
_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS
(*PFN_UCMCONNECTORPDCONNECTIONSTATECHANGED)(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    UCMCONNECTOR Connector,
    _In_
    PUCM_CONNECTOR_PD_CONN_STATE_CHANGED_PARAMS Params
    );

//
// UCM Function: UcmConnectorChargingStateChanged
//
typedef
_Must_inspect_result_
_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS
(*PFN_UCMCONNECTORCHARGINGSTATECHANGED)(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    UCMCONNECTOR Connector,
    _In_
    UCM_CHARGING_STATE ChargingState
    );

//
// UCM Function: UcmConnectorDataDirectionChanged
//
typedef
_IRQL_requires_max_(PASSIVE_LEVEL)
VOID
(*PFN_UCMCONNECTORDATADIRECTIONCHANGED)(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    UCMCONNECTOR Connector,
    _In_
    BOOLEAN Success,
    _In_
    UCM_DATA_ROLE CurrentDataRole
    );

//
// UCM Function: UcmConnectorPowerDirectionChanged
//
typedef
_IRQL_requires_max_(PASSIVE_LEVEL)
VOID
(*PFN_UCMCONNECTORPOWERDIRECTIONCHANGED)(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    UCMCONNECTOR Connector,
    _In_
    BOOLEAN Success,
    _In_
    UCM_POWER_ROLE CurrentPowerRole
    );

PVOID DlFxLdrGetUcmCx0100Functions();
NTSTATUS DlWdfCxUcmFunctionImplStub();

typedef struct _DREAMLIFTER_UCM_MANAGER {
    HANDLE ProxyDriverHandle;
    ULONGLONG ConnectorId;
    BOOL Connected;

    UCM_CONNECTOR_TYPEC_CONFIG TypeCConfig;
    UCM_CONNECTOR_PD_CONFIG PdConfig;

    UCM_CHARGING_STATE ChargingState;
    UCM_PD_CONN_STATE PdConnState;
    UCM_TYPEC_CURRENT PowerCurrent;

    UCM_TYPEC_PARTNER Partner;
    UCM_DATA_ROLE DataRole;
    UCM_POWER_ROLE PowerRole;

    UCM_PD_REQUEST_DATA_OBJECT PdRdo;
    UCM_PD_POWER_DATA_OBJECT SourcePdos[256];
    UCHAR SourcePdoCount;
    UCM_PD_POWER_DATA_OBJECT PartnerPdos[256];
    UCHAR PartnerPdoCount;
} DREAMLIFTER_UCM_DEVICE, * PDREAMLIFTER_UCM_DEVICE;

NTSTATUS DlUcmInitializeDevice(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFDEVICE WdfDevice,
    _In_
    PUCM_MANAGER_CONFIG Config
);

NTSTATUS DlUcmCreateConnector(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFDEVICE WdfDevice,
    _In_
    PUCM_CONNECTOR_CONFIG Config,
    _In_opt_
    PWDF_OBJECT_ATTRIBUTES Attributes,
    _Out_
    UCMCONNECTOR* Connector
);

NTSTATUS DlUcmConnectorTypeCAttach(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    UCMCONNECTOR Connector,
    _In_
    PUCM_CONNECTOR_TYPEC_ATTACH_PARAMS Params
);

NTSTATUS DlUcmConnectorTypeCDetach(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    UCMCONNECTOR Connector
);

NTSTATUS DlUcmConnectorTypeCCurrentAdChanged(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    UCMCONNECTOR Connector,
    _In_
    UCM_TYPEC_CURRENT CurrentAdvertisement
);

NTSTATUS DlUcmConnectorPdSourceCaps(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    UCMCONNECTOR Connector,
    _In_
    UCM_PD_POWER_DATA_OBJECT Pdos[],
    _In_
    UCHAR PdoCount
);

NTSTATUS DlUcmConnectorPdPartnerSourceCaps(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    UCMCONNECTOR Connector,
    _In_
    UCM_PD_POWER_DATA_OBJECT Pdos[],
    _In_
    UCHAR PdoCount
);

NTSTATUS DlUcmConnectorPdConnectionStateChanged(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    UCMCONNECTOR Connector,
    _In_
    PUCM_CONNECTOR_PD_CONN_STATE_CHANGED_PARAMS Params
);

NTSTATUS DlUcmConnectorChargingStateChanged(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    UCMCONNECTOR Connector,
    _In_
    UCM_CHARGING_STATE ChargingState
);

NTSTATUS DlUcmConnectorDataDirectionChanged(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    UCMCONNECTOR Connector,
    _In_
    BOOLEAN Success,
    _In_
    UCM_DATA_ROLE CurrentDataRole
);

NTSTATUS DlUcmConnectorPowerDirectionChanged(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    UCMCONNECTOR Connector,
    _In_
    BOOLEAN Success,
    _In_
    UCM_POWER_ROLE CurrentPowerRole
);

// Debug utilities
char* DbgUcmGetOperatingMode(UCM_TYPEC_OPERATING_MODE mode);
char* DbgUcmGetPartner(UCM_TYPEC_PARTNER partner);
char* DbgUcmGetCurrent(UCM_TYPEC_CURRENT current);
char* DbgUcmGetPowerRole(UCM_POWER_ROLE role);
char* DbgUcmGetDataRole(UCM_DATA_ROLE role);
char* DbgUcmGetPdConnState(UCM_PD_CONN_STATE state);
char* DbgUcmGetChargingState(UCM_CHARGING_STATE state);
char* DbgUcmGetPdoType(UCM_PD_POWER_DATA_OBJECT_TYPE type);
void DbgUcmDumpPdo(UCM_PD_POWER_DATA_OBJECT Pdo);

//
// Macro definition for defining IOCTL and FSCTL function control codes.  Note
// that function codes 0-2047 are reserved for Microsoft Corporation, and
// 2048-4095 are reserved for customers.
//

#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)

#define FILE_DEVICE_CONTROLLER          0x00000004

//
// Define the method codes for how buffers are passed for I/O and FS controls
//

#define METHOD_BUFFERED                 0
#define METHOD_IN_DIRECT                1
#define METHOD_OUT_DIRECT               2
#define METHOD_NEITHER                  3

//
// Proxy device IOCTLs and structs
// Starting from here, platform bitness is also needed to be taken care of
//

#define FILE_ANY_ACCESS                 0
#define FILE_SPECIAL_ACCESS    (FILE_ANY_ACCESS)
#define FILE_READ_ACCESS          ( 0x0001 )    // file & pipe
#define FILE_WRITE_ACCESS         ( 0x0002 )    // file & pipe

// UcmConnectorTypeCAttach
#define IOCTL_UCMPROXY_TYPEC_ATTACH CTL_CODE(FILE_DEVICE_CONTROLLER, 0x900, METHOD_BUFFERED, FILE_ANY_ACCESS)

// UcmConnectorTypeCDetach
#define IOCTL_UCMPROXY_TYPEC_DETACH CTL_CODE(FILE_DEVICE_CONTROLLER, 0x901, METHOD_BUFFERED, FILE_ANY_ACCESS)

// UcmConnectorTypeCCurrentAdChanged
#define IOCTL_UCMPROXY_TYPEC_CURRENT_CHANGE CTL_CODE(FILE_DEVICE_CONTROLLER, 0x902, METHOD_BUFFERED, FILE_ANY_ACCESS)

// UcmConnectorPdSourceCaps
#define IOCTL_UCMPROXY_TYPEC_PD_SOURCE_CAPS CTL_CODE(FILE_DEVICE_CONTROLLER, 0x903, METHOD_BUFFERED, FILE_ANY_ACCESS)

// UcmConnectorPdPartnerSourceCaps
#define IOCTL_UCMPROXY_TYPEC_PD_PARTNER_CAPS CTL_CODE(FILE_DEVICE_CONTROLLER, 0x904, METHOD_BUFFERED, FILE_ANY_ACCESS)

// UcmConnectorPdConnectionStateChanged
#define IOCTL_UCMPROXY_TYPEC_PD_CONN_STATE_CHANGE CTL_CODE(FILE_DEVICE_CONTROLLER, 0x905, METHOD_BUFFERED, FILE_ANY_ACCESS)

// UcmConnectorChargingStateChanged
#define IOCTL_UCMPROXY_TYPEC_CHARGING_STATE_CHANGE CTL_CODE(FILE_DEVICE_CONTROLLER, 0x906, METHOD_BUFFERED, FILE_ANY_ACCESS)

// UcmConnectorDataDirectionChanged
#define IOCTL_UCMPROXY_TYPEC_DATA_DIRECTION_CHANGE CTL_CODE(FILE_DEVICE_CONTROLLER, 0x907, METHOD_BUFFERED, FILE_ANY_ACCESS)

// UcmConnectorPowerDirectionChanged
#define IOCTL_UCMPROXY_TYPEC_POWER_DIRECTION_CHANGE CTL_CODE(FILE_DEVICE_CONTROLLER, 0x908, METHOD_BUFFERED, FILE_ANY_ACCESS)

// EVT_UCM_CONNECTOR_SET_DATA_ROLE
#define IOCTL_UCMPROXY_WAIT_SET_DATA_ROLE_CALLBACK CTL_CODE(FILE_DEVICE_CONTROLLER, 0x909, METHOD_BUFFERED, FILE_ANY_ACCESS)

// EVT_UCM_CONNECTOR_SET_POWER_ROLE
#define IOCTL_UCMPROXY_WAIT_SET_POWER_ROLE_CALLBACK CTL_CODE(FILE_DEVICE_CONTROLLER, 0x910, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct _PROXY_POWER_CAPS_EXCHANGE_REQUEST {
    ULONG Size;
    UCHAR PdoCount;
    UCM_PD_POWER_DATA_OBJECT Pdos[256];
} PROXY_POWER_CAPS_EXCHANGE_REQUEST, * PPROXY_POWER_CAPS_EXCHANGE_REQUEST;

typedef struct _PROXY_ROLE_CHANGE_REQUEST {
    BOOLEAN Success;
    UCHAR   Role;
} PROXY_ROLE_CHANGE_REQUEST, * PPROXY_ROLE_CHANGE_REQUEST;

DWORD WINAPI DlUcmPowerRoleEventWorker(
    LPVOID lpParam
);

DWORD WINAPI DlUcmDataRoleEventWorker(
    LPVOID lpParam
);

#endif
