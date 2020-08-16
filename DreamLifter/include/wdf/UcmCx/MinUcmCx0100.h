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

#endif
