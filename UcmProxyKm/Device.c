#include "driver.h"
#include "device.tmh"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, UcmProxyKmCreateDevice)
#endif

NTSTATUS
UcmProxyKmCreateDevice(
    _Inout_ PWDFDEVICE_INIT DeviceInit
    )
{
    WDF_OBJECT_ATTRIBUTES deviceAttributes;
    PDEVICE_CONTEXT pDeviceContext;
    WDF_PNPPOWER_EVENT_CALLBACKS pnpCallbacks;
    WDFDEVICE device;
    NTSTATUS status;
    UCM_MANAGER_CONFIG ucmMangerConfig;

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&deviceAttributes, DEVICE_CONTEXT);

    WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnpCallbacks);
    pnpCallbacks.EvtDevicePrepareHardware = UcmProxyPrepareHardware;
    pnpCallbacks.EvtDeviceD0Entry = UcmProxyD0Entry;
    pnpCallbacks.EvtDeviceD0Exit = UcmProxyD0Exit;

    WdfDeviceInitSetPnpPowerEventCallbacks(DeviceInit, &pnpCallbacks);
    status = WdfDeviceCreate(&DeviceInit, &deviceAttributes, &device);

    if (NT_SUCCESS(status)) {
        pDeviceContext = DeviceGetContext(device);

        //
        // Initialize UCM device
        //
        UCM_MANAGER_CONFIG_INIT(&ucmMangerConfig);
        status = UcmInitializeDevice(device, &ucmMangerConfig);
        if (NT_SUCCESS(status)) {
            //
            // Create a device interface so that applications can find and talk
            // to us.
            //
            status = WdfDeviceCreateDeviceInterface(
                device,
                &GUID_DEVINTERFACE_UcmProxyKm,
                NULL // ReferenceString
            );

            if (NT_SUCCESS(status)) {
                //
                // Initialize the I/O Package and any Queues
                //
                status = UcmProxyKmQueueInitialize(device);

                if (NT_SUCCESS(status)) {
                    DECLARE_CONST_UNICODE_STRING(UcmProxyDosDeviceName, DOS_DEVICE_NAME);
                    status = WdfDeviceCreateSymbolicLink(
                        device,
                        &UcmProxyDosDeviceName
                    );
                }
            }
        }
    }

    return status;
}

NTSTATUS
UcmProxyPrepareHardware
(
    _In_
    WDFDEVICE Device,
    _In_
    WDFCMRESLIST ResourcesRaw,
    _In_
    WDFCMRESLIST ResourcesTranslated
)
{
    NTSTATUS status;
    PDEVICE_CONTEXT pDeviceContext;
    PCONNECTOR_CONTEXT pConnectorContext;
    UCM_CONNECTOR_CONFIG ucmConnectorConfig;
    UCM_CONNECTOR_TYPEC_CONFIG typeCConfig;
    UCM_CONNECTOR_PD_CONFIG pdConfig;
    WDF_OBJECT_ATTRIBUTES attrib;

    UNREFERENCED_PARAMETER(ResourcesRaw);
    UNREFERENCED_PARAMETER(ResourcesTranslated);

    pDeviceContext = DeviceGetContext(Device);

    // Keep these consistent with stock driver
    UCM_CONNECTOR_CONFIG_INIT(&ucmConnectorConfig, 0);
    UCM_CONNECTOR_TYPEC_CONFIG_INIT(&typeCConfig, UcmTypeCOperatingModeDrp, 0x7);
    UCM_CONNECTOR_PD_CONFIG_INIT(&pdConfig, UcmPowerRoleSink | UcmPowerRoleSource);

    // Event handler will forward things to userland
    pdConfig.EvtSetPowerRole = UcmProxyEvtSetPowerRole;
    typeCConfig.EvtSetDataRole = UcmProxyEvtSetDataRole;

    ucmConnectorConfig.PdConfig = &pdConfig;
    ucmConnectorConfig.TypeCConfig = &typeCConfig;

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attrib, CONNECTOR_CONTEXT);
    status = UcmConnectorCreate(Device, &ucmConnectorConfig, &attrib, &pDeviceContext->Connector);
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "UcmConnectorCreate failed %!STATUS!", status);
    }

    // Put device reference in
    pConnectorContext = ConnectorGetContext(pDeviceContext->Connector);
    pConnectorContext->ParentDevice = Device;

    return status;
}

NTSTATUS
UcmProxyEvtSetPowerRole(
    _In_
    UCMCONNECTOR Connector,
    _In_
    UCM_POWER_ROLE PowerRole
)
{
    NTSTATUS status;
    WDFREQUEST request;
    PUCHAR Buf;

    PCONNECTOR_CONTEXT pConnectorContext = ConnectorGetContext(Connector);
    PDEVICE_CONTEXT pDeviceContext = DeviceGetContext(pConnectorContext->ParentDevice);

    status = WdfIoQueueRetrieveNextRequest(pDeviceContext->SetPowerRoleEventRequestQueue, &request);
    if (NT_SUCCESS(status))
    {
        status = WdfRequestRetrieveOutputBuffer(request, sizeof(UCHAR), &Buf, NULL);
        if (NT_SUCCESS(status))
        {
            *Buf = (UCHAR)PowerRole;
            WdfRequestCompleteWithInformation(request, status, sizeof(UCHAR));
        }
        else
        {
            WdfRequestComplete(request, status);
        }
    }

    return status;
}

NTSTATUS
UcmProxyEvtSetDataRole(
    _In_
    UCMCONNECTOR Connector,
    _In_
    UCM_DATA_ROLE DataRole
)
{
    NTSTATUS status;
    WDFREQUEST request;
    PUCHAR Buf;

    PCONNECTOR_CONTEXT pConnectorContext = ConnectorGetContext(Connector);
    PDEVICE_CONTEXT pDeviceContext = DeviceGetContext(pConnectorContext->ParentDevice);

    status = WdfIoQueueRetrieveNextRequest(pDeviceContext->SetDataRoleEventRequestQueue, &request);
    if (NT_SUCCESS(status))
    {
        status = WdfRequestRetrieveOutputBuffer(request, sizeof(UCHAR), &Buf, NULL);
        if (NT_SUCCESS(status))
        {
            *Buf = (UCHAR)DataRole;
            WdfRequestCompleteWithInformation(request, status, sizeof(UCHAR));
        }
        else
        {
            WdfRequestComplete(request, status);
        }
    }

    return status;
}

NTSTATUS
UcmProxyD0Entry(
    _In_
    WDFDEVICE Device,
    _In_
    WDF_POWER_DEVICE_STATE PreviousState
)
{
    UNREFERENCED_PARAMETER(Device);
    UNREFERENCED_PARAMETER(PreviousState);

    return STATUS_SUCCESS;
}

NTSTATUS
UcmProxyD0Exit(
    _In_
    WDFDEVICE Device,
    _In_
    WDF_POWER_DEVICE_STATE TargetState
)
{
    PDEVICE_CONTEXT pDeviceContext = DeviceGetContext(Device);
    NTSTATUS status;
    WDFREQUEST request;

    UNREFERENCED_PARAMETER(TargetState);

    do
    {
        status = WdfIoQueueRetrieveNextRequest(pDeviceContext->SetDataRoleEventRequestQueue, &request);
        if (NT_SUCCESS(status)) {
            WdfRequestComplete(request, STATUS_CANCELLED);
        }
    } while (NT_SUCCESS(status));

    do
    {
        status = WdfIoQueueRetrieveNextRequest(pDeviceContext->SetPowerRoleEventRequestQueue, &request);
        if (NT_SUCCESS(status)) {
            WdfRequestComplete(request, STATUS_CANCELLED);
        }
    } while (NT_SUCCESS(status));

    return STATUS_SUCCESS;
}
