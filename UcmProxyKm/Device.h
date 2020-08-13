#include "public.h"

EXTERN_C_START

//
// The device context performs the same job as
// a WDM device extension in the driver frameworks
//
typedef struct _DEVICE_CONTEXT
{
    UCMCONNECTOR Connector;
    WDFQUEUE SetPowerRoleEventRequestQueue;
    WDFQUEUE SetDataRoleEventRequestQueue;
} DEVICE_CONTEXT, * PDEVICE_CONTEXT;

typedef struct _CONNECTOR_CONTEXT {
    WDFDEVICE ParentDevice;
} CONNECTOR_CONTEXT, * PCONNECTOR_CONTEXT;

#define DOS_DEVICE_NAME  L"\\DosDevices\\DlUcmProxyDevice"

//
// This macro will generate an inline function called DeviceGetContext
// which will be used to get a pointer to the device context memory
// in a type safe manner.
//
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT, DeviceGetContext)
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(CONNECTOR_CONTEXT, ConnectorGetContext)

//
// Function to initialize the device and its callbacks
//
NTSTATUS
UcmProxyKmCreateDevice(
    _Inout_ PWDFDEVICE_INIT DeviceInit
    );

NTSTATUS
UcmProxyEvtSetPowerRole(
    _In_
    UCMCONNECTOR Connector,
    _In_
    UCM_POWER_ROLE PowerRole
);

NTSTATUS
UcmProxyEvtSetDataRole(
    _In_
    UCMCONNECTOR Connector,
    _In_
    UCM_DATA_ROLE DataRole
);

NTSTATUS
UcmProxyD0Entry(
    _In_
    WDFDEVICE Device,
    _In_
    WDF_POWER_DEVICE_STATE PreviousState
);

NTSTATUS
UcmProxyD0Exit(
    _In_
    WDFDEVICE Device,
    _In_
    WDF_POWER_DEVICE_STATE TargetState
);

//
// Event handlers
//
EVT_WDF_DEVICE_PREPARE_HARDWARE UcmProxyPrepareHardware;

//
// Proxy device IOCTLs and structs
// Starting from here, platform bitness is also needed to be taken care of
//

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

//
// Proxy implementations
//
NTSTATUS UcmProxyConnectorTypeCAttach(
    _In_
    WDFDEVICE Device,
    _In_
    PUCM_CONNECTOR_TYPEC_ATTACH_PARAMS Params
);

NTSTATUS UcmProxyConnectorTypeCDetach(
    _In_
    WDFDEVICE Device
);

NTSTATUS ProxyConnectorTypeCCurrentAdChanged(
    _In_
    WDFDEVICE Device,
    _In_
    UCM_TYPEC_CURRENT CurrentAdvertisement
);

NTSTATUS ProxyConnectorPdConnectionStateChanged(
    _In_
    WDFDEVICE Device,
    _In_
    PUCM_CONNECTOR_PD_CONN_STATE_CHANGED_PARAMS Params
);

NTSTATUS ProxyConnectorChargingStateChanged(
    _In_
    WDFDEVICE Device,
    _In_
    UCM_CHARGING_STATE ChargingState
);

typedef struct _PROXY_ROLE_CHANGE_REQUEST {
    BOOLEAN Success;
    UCHAR   Role;
} PROXY_ROLE_CHANGE_REQUEST, * PPROXY_ROLE_CHANGE_REQUEST;

void ProxyConnectorDataDirectionChanged(
    _In_
    WDFDEVICE Device,
    _In_
    PPROXY_ROLE_CHANGE_REQUEST Params
);

void ProxyConnectorPowerDirectionChanged(
    _In_
    WDFDEVICE Device,
    _In_
    PPROXY_ROLE_CHANGE_REQUEST Params
);

typedef struct _PROXY_POWER_CAPS_EXCHANGE_REQUEST {
    ULONG Size;
    UCHAR PdoCount;
    UCM_PD_POWER_DATA_OBJECT Pdos[256];
} PROXY_POWER_CAPS_EXCHANGE_REQUEST, * PPROXY_POWER_CAPS_EXCHANGE_REQUEST;

NTSTATUS ProxyUcmConnectorPdSourceCaps(
    _In_
    WDFDEVICE Device,
    _In_
    PPROXY_POWER_CAPS_EXCHANGE_REQUEST Params
);

NTSTATUS ProxyUcmConnectorPdPartnerSourceCaps(
    _In_
    WDFDEVICE Device,
    _In_
    PPROXY_POWER_CAPS_EXCHANGE_REQUEST Params
);

EXTERN_C_END
