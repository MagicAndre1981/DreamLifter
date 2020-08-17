// Wdf.c: commonly-shared code between KMDF and UMDF

#include <DreamLifter.h>

extern PDRIVER_INSTANCE g_pDriverInstance;
extern PDREAMLIFTER_DEVICE g_pDevice;
extern wchar_t* g_hdmiDpiI2cConnectionStrings[6];

NTSTATUS DlWdfSpinLockCreate(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_opt_
    PWDF_OBJECT_ATTRIBUTES SpinLockAttributes,
    _Out_
    WDFSPINLOCK* SpinLock
)
{
    PDREAMLIFTER_SPINLOCK pSpinLock;

    UNREFERENCED_PARAMETER(DriverGlobals);
    UNREFERENCED_PARAMETER(SpinLockAttributes);

    pSpinLock = malloc(sizeof(DREAMLIFTER_SPINLOCK));
    if (pSpinLock == NULL) {
        OutputDebugString(L"[ERROR] Failed to allocate spin lock\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    pSpinLock->Header.Magic = DREAMLIFTER_OBJECT_HEADER_MAGIC;
    pSpinLock->Header.Type = DlObjectTypeSpinLock;
    pSpinLock->Exclusion = 1;
    *SpinLock = (WDFSPINLOCK)pSpinLock;
    return STATUS_SUCCESS;
}

void DlWdfSpinLockAcquire(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    _Requires_lock_not_held_(_Curr_)
    _Acquires_lock_(_Curr_)
    _IRQL_saves_
    WDFSPINLOCK SpinLock
)
{
    PDREAMLIFTER_SPINLOCK pSpinLock;

    UNREFERENCED_PARAMETER(DriverGlobals);

    pSpinLock = (PDREAMLIFTER_SPINLOCK)SpinLock;
    while (InterlockedCompareExchange(&pSpinLock->Exclusion, 0, 1) != 1)
    {
        ;
    }
}

void DlWdfSpinLockRelease(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    _Requires_lock_held_(_Curr_)
    _Releases_lock_(_Curr_)
    _IRQL_restores_
    WDFSPINLOCK SpinLock
)
{
    PDREAMLIFTER_SPINLOCK pSpinLock;

    UNREFERENCED_PARAMETER(DriverGlobals);

    pSpinLock = (PDREAMLIFTER_SPINLOCK)SpinLock;
    pSpinLock->Exclusion = 1;
}

NTSTATUS DlWdfTimerCreate(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    PWDF_TIMER_CONFIG Config,
    _In_
    PWDF_OBJECT_ATTRIBUTES Attributes,
    _Out_
    WDFTIMER* Timer
)
{
    PDREAMLIFTER_TIMER pTimer = NULL;

    UNREFERENCED_PARAMETER(DriverGlobals);

    if (Config == NULL || Attributes == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    if (Config->UseHighResolutionTimer != FALSE || Config->Period != 0) {
        return STATUS_NOT_SUPPORTED;
    }

    pTimer = malloc(sizeof(DREAMLIFTER_TIMER));
    if (pTimer == NULL) {
        OutputDebugString(L"[ERROR] Failed to allocate timer\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(pTimer, sizeof(DREAMLIFTER_TIMER));
    pTimer->Header.Magic = DREAMLIFTER_OBJECT_HEADER_MAGIC;
    pTimer->Header.Type = DlObjectTypeTimer;

    if (Config->AutomaticSerialization == TRUE
        && Attributes->SynchronizationScope != WdfSynchronizationScopeInheritFromParent
        && Attributes->SynchronizationScope != WdfSynchronizationScopeDevice) {
        OutputDebugString(L"[ERROR] The timer serialization mode is not yet supported\n");
        free(pTimer);
        return STATUS_NOT_SUPPORTED;
    }

    pTimer->AutomaticSerialization = Config->AutomaticSerialization;
    pTimer->EvtTimerFunc = Config->EvtTimerFunc;
    pTimer->ParentObject = Attributes->ParentObject;

    *Timer = (WDFTIMER)pTimer;
    return STATUS_SUCCESS;
}

WDFOBJECT DlWdfTimerGetParentObject(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFTIMER Timer
)
{
    UNREFERENCED_PARAMETER(DriverGlobals);

    return (WDFOBJECT)((PDREAMLIFTER_TIMER)Timer)->ParentObject;
}

BOOLEAN DlWdfTimerStart(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFTIMER Timer,
    _In_
    LONGLONG DueTime
)
{
    PDREAMLIFTER_TIMER pTimer = (PDREAMLIFTER_TIMER)Timer;
    DWORD dueTime;

    UNREFERENCED_PARAMETER(DriverGlobals);

    if (pTimer == NULL) {
        return FALSE;
    }

    // Check the time and convert to Win32 thing
    if (DueTime < 0) {
        dueTime = (DWORD)((-DueTime * 100) / 1000000);
    }
    else {
        // Not yet seeing the use case here
        TrapDebugger("[WARN] Attempt to use absoulte time for WdfTimer, which is not yet supported\n");
        return FALSE;
    }

    pTimer->Cancelled = FALSE;
    return (BOOLEAN)CreateTimerQueueTimer(
        &pTimer->TimerHandle,
        NULL,
        DlTimerCallbackThreadWorker,
        pTimer,
        dueTime,
        0,
        WT_EXECUTEONLYONCE
    );
}

VOID CALLBACK DlTimerCallbackThreadWorker(
    _In_ PVOID   lpParameter,
    _In_ BOOLEAN TimerOrWaitFired
)
{
    PDREAMLIFTER_TIMER pTimer = (PDREAMLIFTER_TIMER)lpParameter;
    PDREAMLIFTER_DEVICE pDevice = (PDREAMLIFTER_DEVICE)pTimer->ParentObject;

    UNREFERENCED_PARAMETER(TimerOrWaitFired);

    if (pTimer->AutomaticSerialization && pDevice != NULL) {
        // Take device mutex if needed
        WaitForSingleObject(pDevice->SerializationMutex, INFINITE);
    }

    pTimer->EvtTimerFunc((WDFTIMER)pTimer);

    if (pTimer->AutomaticSerialization && pDevice != NULL) {
        ReleaseMutex(pDevice->SerializationMutex);
    }
}

BOOLEAN DlWdfTimerStop(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFTIMER Timer,
    _In_
    BOOLEAN Wait
)
{
    PDREAMLIFTER_TIMER pTimer = (PDREAMLIFTER_TIMER)Timer;

    UNREFERENCED_PARAMETER(DriverGlobals);
    // Technically we need to wait, but not in this case
    UNREFERENCED_PARAMETER(Wait);

    if (pTimer != NULL && pTimer->TimerHandle != INVALID_HANDLE_VALUE) {
        pTimer->Cancelled = TRUE;
        return (BOOLEAN)DeleteTimerQueueTimer(
            NULL,
            pTimer->TimerHandle,
            NULL
        );
    }

    return FALSE;
}

NTSTATUS DlWdfWorkItemCreate(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    PWDF_WORKITEM_CONFIG Config,
    _In_
    PWDF_OBJECT_ATTRIBUTES Attributes,
    _Out_
    WDFWORKITEM* WorkItem
)
{
    PDREAMLIFTER_WORKITEM pWorkItem = NULL;
    size_t contextSize = 0;

    UNREFERENCED_PARAMETER(DriverGlobals);

    if (Config == NULL || Attributes == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    pWorkItem = malloc(sizeof(DREAMLIFTER_WORKITEM));
    if (pWorkItem == NULL) {
        OutputDebugString(L"[ERROR] Failed to allocate workitem\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(pWorkItem, sizeof(DREAMLIFTER_WORKITEM));
    pWorkItem->Header.Magic = DREAMLIFTER_OBJECT_HEADER_MAGIC;
    pWorkItem->Header.Type = DlObjectTypeWorkItem;

    if (Config->AutomaticSerialization == TRUE && Attributes->ParentObject != NULL) {
        if (((PDREAMLIFTER_WDF_OBJECT_HEADER)Attributes->ParentObject)->Type != DlObjectTypeDeviceInstance) {
            OutputDebugString(L"[ERROR] The workitem serialization mode is not yet supported\n");
            free(pWorkItem);
            return STATUS_NOT_SUPPORTED;
        }
    }

    if (Attributes->ContextTypeInfo != NULL) {
        pWorkItem->WorkItemContextInfo = Attributes->ContextTypeInfo;
        contextSize = (Attributes->ContextSizeOverride > Attributes->ContextTypeInfo->ContextSize) ?
            Attributes->ContextSizeOverride : Attributes->ContextTypeInfo->ContextSize;
        if (contextSize > 0) {
            pWorkItem->WorkItemContext = malloc(contextSize);
            if (pWorkItem->WorkItemContext == NULL) {
                OutputDebugString(L"[ERROR] Failed to allocate workitem context struct\n");
                free(pWorkItem);
                return STATUS_INSUFFICIENT_RESOURCES;
            }
            RtlZeroMemory(pWorkItem->WorkItemContext, contextSize);
        }
        else {
            OutputDebugString(L"[WARN] Unexpected device context size 0\n");
            free(pWorkItem);
            return STATUS_INVALID_PARAMETER;
        }
    }

    pWorkItem->AutomaticSerialization = Config->AutomaticSerialization;
    pWorkItem->EvtWorkItemFunc = Config->EvtWorkItemFunc;
    pWorkItem->ParentObject = Attributes->ParentObject;

    *WorkItem = (WDFWORKITEM)pWorkItem;

    return STATUS_SUCCESS;
}

WDFOBJECT DlWdfWorkItemGetParentObject(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFWORKITEM WorkItem
)
{
    UNREFERENCED_PARAMETER(DriverGlobals);

    return (WDFOBJECT)((PDREAMLIFTER_WORKITEM)WorkItem)->ParentObject;
}

VOID DlWdfWorkItemEnqueue(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFWORKITEM WorkItem
)
{
    PDREAMLIFTER_WORKITEM pWorkItem = (PDREAMLIFTER_WORKITEM)WorkItem;
    DWORD AsyncThreadId = 0;

    UNREFERENCED_PARAMETER(DriverGlobals);

    if (pWorkItem != NULL) {
        // Push things in a new thread
        CreateThread(
            NULL,
            0,
            DlWdfWorkItemThreadWorker,
            pWorkItem,
            0,
            &AsyncThreadId
        );
    }
}

DWORD WINAPI DlWdfWorkItemThreadWorker(
    LPVOID lpParam
)
{
    PDREAMLIFTER_WORKITEM pWorkItem = (PDREAMLIFTER_WORKITEM) lpParam;

    if (pWorkItem->AutomaticSerialization && pWorkItem->ParentObject != NULL) {
        WaitForSingleObject(((PDREAMLIFTER_DEVICE)pWorkItem->ParentObject)->SerializationMutex, INFINITE);
    }

    pWorkItem->EvtWorkItemFunc((WDFWORKITEM)pWorkItem);

    if (pWorkItem->AutomaticSerialization && pWorkItem->ParentObject != NULL) {
        ReleaseMutex(((PDREAMLIFTER_DEVICE)pWorkItem->ParentObject)->SerializationMutex);
    }
    return 0;
}

NTSTATUS DlWdfCreateDriver(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    PDRIVER_OBJECT DriverObject,
    _In_
    PCUNICODE_STRING RegistryPath,
    _In_opt_
    PWDF_OBJECT_ATTRIBUTES DriverAttributes,
    _In_
    PWDF_DRIVER_CONFIG DriverConfig,
    _Out_opt_
    WDFDRIVER* Driver
)
{
    PDRIVER_INSTANCE pDriverInstance = NULL;

    UNREFERENCED_PARAMETER(DriverGlobals);
    UNREFERENCED_PARAMETER(DriverObject);
    UNREFERENCED_PARAMETER(RegistryPath);

    if (DriverConfig == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    // Access DriverConfig to get callbacks such as device add
    UNREFERENCED_PARAMETER(DriverConfig);

    // Initialize driver struct
    pDriverInstance = malloc(sizeof(DRIVER_INSTANCE));
    if (pDriverInstance == NULL) {
        OutputDebugString(L"[ERROR] Failed to allocate DRIVER_INSTANCE struct\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(pDriverInstance, sizeof(DRIVER_INSTANCE));
    pDriverInstance->Header.Magic = DREAMLIFTER_OBJECT_HEADER_MAGIC;
    pDriverInstance->Header.Type = DlObjectTypeDriverInstance;

    Driver = (WDFDRIVER*)pDriverInstance;
    // Currently this is a singleton host
    g_pDriverInstance = pDriverInstance;

    if (DriverAttributes != NULL) {
        pDriverInstance->DriverCleanupCallback = DriverAttributes->EvtCleanupCallback;
        pDriverInstance->DriverDestroyCallback = DriverAttributes->EvtCleanupCallback;
    }

    // This will be called when entry successfully exits
    pDriverInstance->DriverDeviceAdd = DriverConfig->EvtDriverDeviceAdd;
    pDriverInstance->DriverUnload = DriverConfig->EvtDriverUnload;

    return STATUS_SUCCESS;
}

void DlWdfDeviceInitSetPnpPowerEventCallbacks(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    PVOID DeviceInit,
    _In_
    PWDF_PNPPOWER_EVENT_CALLBACKS PnpPowerEventCallbacks
)
{
    UNREFERENCED_PARAMETER(DriverGlobals);

    if (DeviceInit != NULL && PnpPowerEventCallbacks != NULL) {
        ((PDREAMLIFTER_DEVICE_INIT)DeviceInit)->EvtDevicePrepareHardware = PnpPowerEventCallbacks->EvtDevicePrepareHardware;
        ((PDREAMLIFTER_DEVICE_INIT)DeviceInit)->EvtDeviceReleaseHardware = PnpPowerEventCallbacks->EvtDeviceReleaseHardware;
        ((PDREAMLIFTER_DEVICE_INIT)DeviceInit)->EvtDeviceD0Entry = PnpPowerEventCallbacks->EvtDeviceD0Entry;
        ((PDREAMLIFTER_DEVICE_INIT)DeviceInit)->EvtDeviceD0Exit = PnpPowerEventCallbacks->EvtDeviceD0Exit;
    }
}

void DlWdfDeviceInitSetFileObjectConfig(
    PWDFDEVICE_INIT        DeviceInit,
    PWDF_FILEOBJECT_CONFIG FileObjectConfig,
    PWDF_OBJECT_ATTRIBUTES FileObjectAttributes
)
{
    UNREFERENCED_PARAMETER(FileObjectAttributes);

    if (DeviceInit != NULL && FileObjectConfig != NULL) {
        ((PDREAMLIFTER_DEVICE_INIT)DeviceInit)->AutoForwardCleanupClose = FileObjectConfig->AutoForwardCleanupClose;
        ((PDREAMLIFTER_DEVICE_INIT)DeviceInit)->EvtDeviceFileCreate = FileObjectConfig->EvtDeviceFileCreate;
        ((PDREAMLIFTER_DEVICE_INIT)DeviceInit)->EvtFileCleanup = FileObjectConfig->EvtFileCleanup;
        ((PDREAMLIFTER_DEVICE_INIT)DeviceInit)->EvtFileClose = FileObjectConfig->EvtFileClose;
        ((PDREAMLIFTER_DEVICE_INIT)DeviceInit)->FileObjectClass = FileObjectConfig->FileObjectClass;
    }
}

NTSTATUS DlWdfDeviceCreate(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _Inout_
    PWDFDEVICE_INIT* DeviceInit,
    _In_opt_
    PWDF_OBJECT_ATTRIBUTES DeviceAttributes,
    _Out_
    WDFDEVICE* Device
)
{
    PDREAMLIFTER_DEVICE pDevice;
    size_t contextSize = 0;

    UNREFERENCED_PARAMETER(DriverGlobals);

    if (DeviceInit == NULL || DeviceAttributes == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    pDevice = malloc(sizeof(DREAMLIFTER_DEVICE));
    if (pDevice == NULL) {
        OutputDebugString(L"[ERROR] Failed to allocate DREAMLIFTER_DEVICE struct\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(pDevice, sizeof(DREAMLIFTER_DEVICE));
    pDevice->Header.Magic = DREAMLIFTER_OBJECT_HEADER_MAGIC;
    pDevice->Header.Type = DlObjectTypeDeviceInstance;

    pDevice->EvtDevicePrepareHardware = ((PDREAMLIFTER_DEVICE_INIT)*DeviceInit)->EvtDevicePrepareHardware;
    pDevice->EvtDeviceReleaseHardware = ((PDREAMLIFTER_DEVICE_INIT)*DeviceInit)->EvtDeviceReleaseHardware;
    pDevice->EvtDeviceD0Entry = ((PDREAMLIFTER_DEVICE_INIT)*DeviceInit)->EvtDeviceD0Entry;
    pDevice->EvtDeviceD0Exit = ((PDREAMLIFTER_DEVICE_INIT)*DeviceInit)->EvtDeviceD0Exit;
    pDevice->EvtDeviceFileCreate = ((PDREAMLIFTER_DEVICE_INIT)*DeviceInit)->EvtDeviceFileCreate;
    pDevice->EvtFileClose = ((PDREAMLIFTER_DEVICE_INIT)*DeviceInit)->EvtFileClose;
    pDevice->EvtFileCleanup = ((PDREAMLIFTER_DEVICE_INIT)*DeviceInit)->EvtFileCleanup;

    if (DeviceAttributes->ContextTypeInfo != NULL) {
        pDevice->DeviceContextInfo = DeviceAttributes->ContextTypeInfo;
        contextSize = (DeviceAttributes->ContextSizeOverride > DeviceAttributes->ContextTypeInfo->ContextSize) ?
            DeviceAttributes->ContextSizeOverride : DeviceAttributes->ContextTypeInfo->ContextSize;
        if (contextSize > 0) {
            pDevice->DeviceContext = malloc(contextSize);
            if (pDevice->DeviceContext == NULL) {
                OutputDebugString(L"[ERROR] Failed to allocate device context struct\n");
                return STATUS_INSUFFICIENT_RESOURCES;
            }
            RtlZeroMemory(pDevice->DeviceContext, contextSize);
        }
        else {
            OutputDebugString(L"[WARN] Unexpected device context size 0\n");
            return STATUS_INVALID_PARAMETER;
        }
    }

    pDevice->SerializationMutex = CreateMutex(
        NULL,
        FALSE,
        NULL
    );
    if (pDevice->SerializationMutex == NULL)
    {
        printf("[ERROR] CreateMutex error: %d\n", GetLastError());
        free(pDevice);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    *Device = (WDFDEVICE)pDevice;
    g_pDevice = pDevice;
    return STATUS_SUCCESS;
}

PVOID DlWdfObjectGetTypedContextWorker(
    PWDF_DRIVER_GLOBALS            DriverGlobals,
    WDFOBJECT                      Handle,
    PCWDF_OBJECT_CONTEXT_TYPE_INFO TypeInfo
)
{
    PDREAMLIFTER_WDF_OBJECT_HEADER pObjectHeader = (PDREAMLIFTER_WDF_OBJECT_HEADER) Handle;
    if (pObjectHeader->Magic != DREAMLIFTER_OBJECT_HEADER_MAGIC) {
        TrapDebugger("[ERROR] Attempt to access invalid WDF object\n");
        return NULL;
    }

    // Skip the first object header
    PDREAMLIFTER_CONTEXT_HEADER pContextHeader = (PDREAMLIFTER_CONTEXT_HEADER) ((PUCHAR) Handle + sizeof(DREAMLIFTER_WDF_OBJECT_HEADER));
    UNREFERENCED_PARAMETER(DriverGlobals);

    if (pContextHeader != NULL) {
        if (strcmp(pContextHeader->ContextInfo->ContextName, TypeInfo->ContextName) == 0 &&
            pContextHeader->ContextInfo->ContextSize == TypeInfo->ContextSize) {
            return pContextHeader->Context;
        }
    }

    TrapDebugger("[ERROR] Attempt to access invalid WDF context\n");
    return NULL;
}

NTSTATUS DlWdfCreateDeviceInterface(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFDEVICE Device,
    _In_
    CONST GUID* InterfaceClassGUID,
    _In_opt_
    PCUNICODE_STRING ReferenceString
)
{
    UNREFERENCED_PARAMETER(DriverGlobals);
    UNREFERENCED_PARAMETER(ReferenceString);

    if (Device == NULL || InterfaceClassGUID == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    printf("[INFO] Creating device interface {%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}. This is currently a no-op\n",
        InterfaceClassGUID->Data1, InterfaceClassGUID->Data2, InterfaceClassGUID->Data3,
        InterfaceClassGUID->Data4[0], InterfaceClassGUID->Data4[1],
        InterfaceClassGUID->Data4[2], InterfaceClassGUID->Data4[3],
        InterfaceClassGUID->Data4[4], InterfaceClassGUID->Data4[5],
        InterfaceClassGUID->Data4[6], InterfaceClassGUID->Data4[7]
    );

    return STATUS_SUCCESS;
}

NTSTATUS DlWdfDeviceCreateSymbolicLink(
    PWDF_DRIVER_GLOBALS DriverGlobals,
    WDFDEVICE        Device,
    PCUNICODE_STRING SymbolicLinkName
)
{
    UNREFERENCED_PARAMETER(DriverGlobals);
    UNREFERENCED_PARAMETER(Device);

    wprintf(L"[INFO] Creating device symbolic link %s. This is currently a no-op\n",
        SymbolicLinkName->Buffer
    );

    return STATUS_SUCCESS;
}

NTSTATUS DlWdfInterruptCreate(
    PWDF_DRIVER_GLOBALS DriverGlobals,
    WDFDEVICE              Device,
    PWDF_INTERRUPT_CONFIG  Configuration,
    PWDF_OBJECT_ATTRIBUTES Attributes,
    WDFINTERRUPT* Interrupt
)
{
    PDREAMLIFTER_DEVICE pDevice = (PDREAMLIFTER_DEVICE) Device;

    UNREFERENCED_PARAMETER(DriverGlobals);
    UNREFERENCED_PARAMETER(Attributes);

    if (Configuration == NULL || pDevice == NULL || Interrupt == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    if (pDevice->Interrupt != NULL) {
        printf("[ERROR] DreamLifter only supports a single interrupt yet\n");
        return STATUS_NOT_SUPPORTED;
    }

    if (!Configuration->PassiveHandling) {
        printf("[ERROR] DreamLifter only supports passive level interrupt\n");
        return STATUS_INVALID_PARAMETER;
    }

    pDevice->Interrupt = malloc(sizeof(DL_WDF_INTERRUPT));
    if (pDevice->Interrupt == NULL) {
        printf("[ERROR] Failed to allocate resources for interrupt\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(pDevice->Interrupt, sizeof(DL_WDF_INTERRUPT));
    pDevice->Interrupt->Header.Magic = DREAMLIFTER_OBJECT_HEADER_MAGIC;
    pDevice->Interrupt->Header.Type = DlObjectTypeInterrupt;

    pDevice->Interrupt->AssociatedDevice = Device;
    pDevice->Interrupt->EvtInterruptIsr = Configuration->EvtInterruptIsr;
    pDevice->Interrupt->EvtInterruptDisable = Configuration->EvtInterruptDisable;
    pDevice->Interrupt->EvtInterruptEnable = Configuration->EvtInterruptEnable;
    
    *Interrupt = (WDFINTERRUPT) pDevice->Interrupt;
    return STATUS_SUCCESS;
}

void DlWdfInterruptEnable(
    PWDF_DRIVER_GLOBALS DriverGlobals,
    WDFINTERRUPT Interrupt
)
{
    PDL_WDF_INTERRUPT pInterrupt = (PDL_WDF_INTERRUPT) Interrupt;
    UNREFERENCED_PARAMETER(DriverGlobals);

    if (pInterrupt->EvtInterruptEnable != NULL) {
        pInterrupt->EvtInterruptEnable((WDFINTERRUPT) pInterrupt, (WDFDEVICE) pInterrupt->AssociatedDevice);
    }

    // TODO: notify km partner to enable interrupt
}

void DlWdfInterruptDisable(
    PWDF_DRIVER_GLOBALS DriverGlobals,
    WDFINTERRUPT Interrupt
)
{
    PDL_WDF_INTERRUPT pInterrupt = (PDL_WDF_INTERRUPT)Interrupt;
    UNREFERENCED_PARAMETER(DriverGlobals);

    if (pInterrupt->EvtInterruptDisable != NULL) {
        pInterrupt->EvtInterruptDisable((WDFINTERRUPT) pInterrupt, (WDFDEVICE) pInterrupt->AssociatedDevice);
    }

    // TODO: notify km partner to disable interrupt
}

NTSTATUS DlWdfIoQueueCreate(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFDEVICE Device,
    _In_
    PWDF_IO_QUEUE_CONFIG Config,
    _In_opt_
    PWDF_OBJECT_ATTRIBUTES QueueAttributes,
    _Out_opt_
    WDFQUEUE* Queue
)
{
    PDREAMLIFTER_DEVICE pDevice = (PDREAMLIFTER_DEVICE) Device;

    UNREFERENCED_PARAMETER(DriverGlobals);
    UNREFERENCED_PARAMETER(QueueAttributes);

    if (Config == NULL || pDevice == NULL || Queue == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    if (pDevice->DefaultIoQueue != NULL || !Config->DefaultQueue) {
        printf("[ERROR] DreamLifter only supports a single default queue\n");
        return STATUS_NOT_SUPPORTED;
    }

    pDevice->DefaultIoQueue = malloc(sizeof(DL_WDF_QUEUE));
    if (pDevice->DefaultIoQueue == NULL) {
        printf("[ERROR] Failed to allocate resources for IO queue\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(pDevice->DefaultIoQueue, sizeof(DL_WDF_QUEUE));
    pDevice->DefaultIoQueue->Header.Magic = DREAMLIFTER_OBJECT_HEADER_MAGIC;
    pDevice->DefaultIoQueue->Header.Type = DlObjectTypeIoQueue;

    pDevice->DefaultIoQueue->AssociatedDevice = Device;
    pDevice->DefaultIoQueue->EvtIoDeviceControl = Config->EvtIoDeviceControl;
    pDevice->DefaultIoQueue->EvtIoCanceledOnQueue = Config->EvtIoCanceledOnQueue;

    *Queue = (WDFQUEUE) pDevice->DefaultIoQueue;
    return STATUS_SUCCESS;
}

ULONG DlWdfCmResourceListGetCount(
    PWDF_DRIVER_GLOBALS DriverGlobals,
    WDFCMRESLIST List
)
{
    PDL_WDF_CM_RES_LIST pList = (PDL_WDF_CM_RES_LIST) List;

    UNREFERENCED_PARAMETER(DriverGlobals);

    if (pList == NULL || pList->Header.Magic != DREAMLIFTER_OBJECT_HEADER_MAGIC ||
        pList->Header.Type != DlObjectTypeCmListTranslated) {
        TrapDebugger("[ERROR] Sending a non CmList to WdfCmResourceListGetCount\n");
        return 0;
    }

    return pList->ResourceCount;
}

PCM_PARTIAL_RESOURCE_DESCRIPTOR DlWdfCmResourceListGetDescriptor(
    PWDF_DRIVER_GLOBALS DriverGlobals,
    WDFCMRESLIST List,
    ULONG        Index
)
{
    PDL_WDF_CM_RES_LIST pList = (PDL_WDF_CM_RES_LIST) List;

    UNREFERENCED_PARAMETER(DriverGlobals);

    if (pList == NULL || pList->Header.Magic != DREAMLIFTER_OBJECT_HEADER_MAGIC ||
        pList->Header.Type != DlObjectTypeCmListTranslated) {
        TrapDebugger("[ERROR] Sending a non CmList to WdfCmResourceListGetDescriptor\n");
        return NULL;
    }

    if (Index >= pList->ResourceCount) {
        TrapDebugger("[ERROR] Index out of boundary\n");
        return NULL;
    }

    return &pList->Resources[Index];
}

NTSTATUS DlWdfIoTargetCreate(
    PWDF_DRIVER_GLOBALS DriverGlobals,
    WDFDEVICE Device,
    PWDF_OBJECT_ATTRIBUTES IoTargetAttributes,
    WDFIOTARGET* IoTarget
)
{
    PDL_WDF_IOTARGET pIoTarget = NULL;

    UNREFERENCED_PARAMETER(DriverGlobals);
    UNREFERENCED_PARAMETER(IoTargetAttributes);

    if (IoTarget == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    pIoTarget = malloc(sizeof(DL_WDF_IOTARGET));
    if (pIoTarget == NULL) {
        printf("[ERROR] Failed to allocate resources for IO target\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(pIoTarget, sizeof(DL_WDF_IOTARGET));
    pIoTarget->Header.Magic = DREAMLIFTER_OBJECT_HEADER_MAGIC;
    pIoTarget->Header.Type = DlObjectTypeIoTarget;
    pIoTarget->ParentDevice = Device;
    pIoTarget->TargetInternalId = 0xFFFFFFFF;
    *IoTarget = (WDFIOTARGET)pIoTarget;

    return STATUS_SUCCESS;
}

void DlWdfObjectDelete(
    PWDF_DRIVER_GLOBALS DriverGlobals,
    WDFOBJECT Object
)
{
    PDREAMLIFTER_WDF_OBJECT_HEADER pObjectHeader = (PDREAMLIFTER_WDF_OBJECT_HEADER) Object;

    UNREFERENCED_PARAMETER(DriverGlobals);

    if (pObjectHeader != NULL) {
        if (pObjectHeader->Magic != DREAMLIFTER_OBJECT_HEADER_MAGIC) {
            TrapDebugger("[ERROR] Attempt to access invalid WDF object\n");
            return;
        }

        // TODO: implement delete for memory objects later
        if (pObjectHeader->Type == DlObjectTypeIoTarget) {
            // Can be freed
            free((PVOID)Object);
        }
    }
}

NTSTATUS DlWdfIoTargetOpen(
    PWDF_DRIVER_GLOBALS        DriverGlobals,
    WDFIOTARGET                IoTarget,
    PWDF_IO_TARGET_OPEN_PARAMS OpenParams
)
{
    PDL_WDF_IOTARGET pIoTarget = (PDL_WDF_IOTARGET) IoTarget;

    UNREFERENCED_PARAMETER(DriverGlobals);

    if (IoTarget == NULL || OpenParams == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    if (pIoTarget->Opened) {
        return STATUS_NOT_SUPPORTED;
    }

    // Kernel proxy should have it open, we just pick and use
    // But we need to identify the component from path string
    pIoTarget->TargetInternalId = 0xFFFFFFFF;

    for (ULONG i = 0; i < 5; i++) {
        if (wcscmp(OpenParams->TargetDeviceName.Buffer, g_hdmiDpiI2cConnectionStrings[i]) == 0) {
            pIoTarget->TargetInternalId = i;
            break;
        }
    }

    if (pIoTarget->TargetInternalId == 0xFFFFFFFF) {
        return STATUS_OBJECT_NAME_NOT_FOUND;
    }

    pIoTarget->Opened = TRUE;
    return STATUS_SUCCESS;
}

void DlWdfIoTargetClose(
    PWDF_DRIVER_GLOBALS        DriverGlobals,
    WDFIOTARGET IoTarget
)
{
    PDL_WDF_IOTARGET pIoTarget = (PDL_WDF_IOTARGET)IoTarget;

    UNREFERENCED_PARAMETER(DriverGlobals);

    if (pIoTarget != NULL) {
        pIoTarget->Opened = FALSE;
        pIoTarget->TargetInternalId = 0xFFFFFFFF;
    }
}
