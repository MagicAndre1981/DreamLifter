#include "driver.h"
#include "queue.tmh"

NTSTATUS
UcmProxyQueueInitialize(
    _In_ WDFDEVICE Device
    )
{
    WDFQUEUE queue;
    NTSTATUS status;
    WDF_IO_QUEUE_CONFIG queueConfig;
    PDEVICE_CONTEXT pDeviceContext;

    pDeviceContext = DeviceGetContext(Device);

    //
    // Configure a default queue so that requests that are not
    // configure-fowarded using WdfDeviceConfigureRequestDispatching to goto
    // other queues get dispatched here.
    //
    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(
        &queueConfig,
        WdfIoQueueDispatchParallel
        );

    queueConfig.EvtIoDeviceControl = UcmProxyEvtIoDeviceControl;

    status = WdfIoQueueCreate(
                 Device,
                 &queueConfig,
                 WDF_NO_OBJECT_ATTRIBUTES,
                 &queue
                 );

    if(!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "WdfIoQueueCreate (Default) failed %!STATUS!", status);
        return status;
    }

    WDF_IO_QUEUE_CONFIG_INIT(
        &queueConfig,
        WdfIoQueueDispatchManual
    );

    status = WdfIoQueueCreate(
        Device,
        &queueConfig,
        WDF_NO_OBJECT_ATTRIBUTES,
        &pDeviceContext->SetDataRoleEventRequestQueue
    );

    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "WdfIoQueueCreate (DataRole) failed %!STATUS!", status);
        return status;
    }

    WDF_IO_QUEUE_CONFIG_INIT(
        &queueConfig,
        WdfIoQueueDispatchManual
    );

    status = WdfIoQueueCreate(
        Device,
        &queueConfig,
        WDF_NO_OBJECT_ATTRIBUTES,
        &pDeviceContext->SetPowerRoleEventRequestQueue
    );

    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "WdfIoQueueCreate (PowerRole) failed %!STATUS!", status);
        return status;
    }

    return status;
}

VOID
UcmProxyEvtIoDeviceControl(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ size_t OutputBufferLength,
    _In_ size_t InputBufferLength,
    _In_ ULONG IoControlCode
    )
{
    WDFDEVICE device;
    PDEVICE_CONTEXT pDeviceContext;
    BOOL bMarkComplete = TRUE;
    NTSTATUS status = STATUS_SUCCESS;
    PVOID Buffer;

    TraceEvents(TRACE_LEVEL_INFORMATION, 
                TRACE_QUEUE, 
                "%!FUNC! Queue 0x%p, Request 0x%p OutputBufferLength %d InputBufferLength %d IoControlCode %d", 
                Queue, Request, (int) OutputBufferLength, (int) InputBufferLength, IoControlCode);

    device = WdfIoQueueGetDevice(Queue);
    pDeviceContext = DeviceGetContext(device);

    if (IoControlCode == IOCTL_UCMPROXY_WAIT_SET_DATA_ROLE_CALLBACK) {
        // This has single bit output field.
        if (OutputBufferLength < sizeof(UCHAR)) {
            status = STATUS_INVALID_PARAMETER;
            goto exit;
        }
        status = WdfRequestForwardToIoQueue(Request, pDeviceContext->SetDataRoleEventRequestQueue);
        bMarkComplete = !NT_SUCCESS(status);
    }
    else if (IoControlCode == IOCTL_UCMPROXY_WAIT_SET_POWER_ROLE_CALLBACK) {
        // This has single bit output field.
        if (OutputBufferLength < sizeof(UCHAR)) {
            status = STATUS_INVALID_PARAMETER;
            goto exit;
        }
        status = WdfRequestForwardToIoQueue(Request, pDeviceContext->SetPowerRoleEventRequestQueue);
        bMarkComplete = !NT_SUCCESS(status);
    }
    else {
        // Other have input fields. Not require output fields
        if (InputBufferLength < 1) {
            status = STATUS_INVALID_PARAMETER;
            goto exit;
        }

        status = WdfRequestRetrieveInputBuffer(Request, sizeof(UCHAR), &Buffer, NULL);
        if (!NT_SUCCESS(status)) {
            TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "%!FUNC! WdfRequestRetrieveInputBuffer failed %!STATUS!", status);
            goto exit;
        }

        switch (IoControlCode) {
        case IOCTL_UCMPROXY_TYPEC_ATTACH:
            status = UcmProxyConnectorTypeCAttach(device, (PUCM_CONNECTOR_TYPEC_ATTACH_PARAMS) Buffer);
            break;
        case IOCTL_UCMPROXY_TYPEC_DETACH:
            status = UcmProxyConnectorTypeCDetach(device);
            break;
        case IOCTL_UCMPROXY_TYPEC_CURRENT_CHANGE:
            status = ProxyConnectorTypeCCurrentAdChanged(device, (UCM_TYPEC_CURRENT) *((PUCHAR)Buffer));
            break;
        case IOCTL_UCMPROXY_TYPEC_PD_SOURCE_CAPS:
            status = ProxyUcmConnectorPdSourceCaps(device, (PPROXY_POWER_CAPS_EXCHANGE_REQUEST) Buffer);
            break;
        case IOCTL_UCMPROXY_TYPEC_PD_PARTNER_CAPS:
            status = ProxyUcmConnectorPdPartnerSourceCaps(device, (PPROXY_POWER_CAPS_EXCHANGE_REQUEST)Buffer);
            break;
        case IOCTL_UCMPROXY_TYPEC_PD_CONN_STATE_CHANGE:
            status = ProxyConnectorPdConnectionStateChanged(device, (PUCM_CONNECTOR_PD_CONN_STATE_CHANGED_PARAMS) Buffer);
            break;
        case IOCTL_UCMPROXY_TYPEC_CHARGING_STATE_CHANGE:
            status = ProxyConnectorChargingStateChanged(device, (UCM_CHARGING_STATE) *((PUCHAR)Buffer));
            break;
        case IOCTL_UCMPROXY_TYPEC_DATA_DIRECTION_CHANGE:
            ProxyConnectorDataDirectionChanged(device, (PPROXY_ROLE_CHANGE_REQUEST) Buffer);
            status = STATUS_SUCCESS;
            break;
        case IOCTL_UCMPROXY_TYPEC_POWER_DIRECTION_CHANGE:
            ProxyConnectorPowerDirectionChanged(device, (PPROXY_ROLE_CHANGE_REQUEST)Buffer);
            status = STATUS_SUCCESS;
            break;
        default:
            status = STATUS_NOT_SUPPORTED;
            break;
        }
    }

exit:
    if (bMarkComplete) {
        WdfRequestComplete(Request, status);
    }
    return;
}

NTSTATUS UcmProxyConnectorTypeCAttach(
    _In_
    WDFDEVICE Device,
    _In_
    PUCM_CONNECTOR_TYPEC_ATTACH_PARAMS Params
)
{
    PDEVICE_CONTEXT pContext = DeviceGetContext(Device);

    if (Params->Size != sizeof(PUCM_CONNECTOR_TYPEC_ATTACH_PARAMS)) {
        return STATUS_INVALID_PARAMETER;
    }

    return UcmConnectorTypeCAttach(pContext->Connector, Params);
}

NTSTATUS UcmProxyConnectorTypeCDetach(
    _In_
    WDFDEVICE Device
)
{
    PDEVICE_CONTEXT pContext = DeviceGetContext(Device);
    return UcmConnectorTypeCDetach(pContext->Connector);
}

NTSTATUS ProxyConnectorTypeCCurrentAdChanged(
    _In_
    WDFDEVICE Device,
    _In_
    UCM_TYPEC_CURRENT CurrentAdvertisement
)
{
    PDEVICE_CONTEXT pContext = DeviceGetContext(Device);
    return UcmConnectorTypeCCurrentAdChanged(pContext->Connector, CurrentAdvertisement);
}

NTSTATUS ProxyConnectorPdConnectionStateChanged(
    _In_
    WDFDEVICE Device,
    _In_
    PUCM_CONNECTOR_PD_CONN_STATE_CHANGED_PARAMS Params
)
{
    PDEVICE_CONTEXT pContext = DeviceGetContext(Device);
    return UcmConnectorPdConnectionStateChanged(pContext->Connector, Params);
}

NTSTATUS ProxyConnectorChargingStateChanged(
    _In_
    WDFDEVICE Device,
    _In_
    UCM_CHARGING_STATE ChargingState
)
{
    PDEVICE_CONTEXT pContext = DeviceGetContext(Device);
    return UcmConnectorChargingStateChanged(pContext->Connector, ChargingState);
}

void ProxyConnectorDataDirectionChanged(
    _In_
    WDFDEVICE Device,
    _In_
    PPROXY_ROLE_CHANGE_REQUEST Params
)
{
    PDEVICE_CONTEXT pContext = DeviceGetContext(Device);
    UcmConnectorDataDirectionChanged(pContext->Connector, Params->Success, (UCM_DATA_ROLE) Params->Role);
}

void ProxyConnectorPowerDirectionChanged(
    _In_
    WDFDEVICE Device,
    _In_
    PPROXY_ROLE_CHANGE_REQUEST Params
)
{
    PDEVICE_CONTEXT pContext = DeviceGetContext(Device);
    UcmConnectorDataDirectionChanged(pContext->Connector, Params->Success, (UCM_POWER_ROLE)Params->Role);
}

NTSTATUS ProxyUcmConnectorPdSourceCaps(
    _In_
    WDFDEVICE Device,
    _In_
    PPROXY_POWER_CAPS_EXCHANGE_REQUEST Params
)
{
    PDEVICE_CONTEXT pContext = DeviceGetContext(Device);
    if (Params->Size != sizeof(PROXY_POWER_CAPS_EXCHANGE_REQUEST)) {
        return STATUS_INVALID_PARAMETER;
    }

    return UcmConnectorPdSourceCaps(pContext->Connector, Params->Pdos, Params->PdoCount);
}

NTSTATUS ProxyUcmConnectorPdPartnerSourceCaps(
    _In_
    WDFDEVICE Device,
    _In_
    PPROXY_POWER_CAPS_EXCHANGE_REQUEST Params
)
{
    PDEVICE_CONTEXT pContext = DeviceGetContext(Device);
    if (Params->Size != sizeof(PROXY_POWER_CAPS_EXCHANGE_REQUEST)) {
        return STATUS_INVALID_PARAMETER;
    }

    return UcmConnectorPdPartnerSourceCaps(pContext->Connector, Params->Pdos, Params->PdoCount);
}
