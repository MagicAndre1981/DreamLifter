// MinUmdf0215.c: UMDF 2.15 min required implementation

#include <DreamLifter.h>

extern PDRIVER_INSTANCE g_pDriverInstance;
extern PDREAMLIFTER_DEVICE g_pDevice;

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
    RtlZeroMemory(pDriverInstance, sizeof(DRIVER_INSTANCE));
    if (pDriverInstance == NULL) {
        OutputDebugString(L"[ERROR] Failed to allocate DRIVER_INSTANCE struct\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Driver = (WDFDRIVER*) pDriverInstance;
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
        ((PDREAMLIFTER_DEVICE_INIT) DeviceInit)->EvtDevicePrepareHardware = PnpPowerEventCallbacks->EvtDevicePrepareHardware;
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
    pDevice->EvtDevicePrepareHardware = ((PDREAMLIFTER_DEVICE_INIT) *DeviceInit)->EvtDevicePrepareHardware;
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

    *Device = (WDFDEVICE) pDevice;
    g_pDevice = pDevice;
    return STATUS_SUCCESS;
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

    pSpinLock->Exclusion = 1;
    *SpinLock = (WDFSPINLOCK) pSpinLock;
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

    pSpinLock = (PDREAMLIFTER_SPINLOCK) SpinLock;
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

void DlWdfRequestComplete(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFREQUEST Request,
    _In_
    NTSTATUS Status
)
{
    UNREFERENCED_PARAMETER(DriverGlobals);
    
    printf("[INFO] Completing request %p with status 0x%x. This is currently a no-op\n", Request, Status);
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
    UNREFERENCED_PARAMETER(DriverGlobals);
    UNREFERENCED_PARAMETER(Device);
    UNREFERENCED_PARAMETER(Config);
    UNREFERENCED_PARAMETER(QueueAttributes);
    UNREFERENCED_PARAMETER(Queue);

    printf("[INFO] Creating queue is currently a no-op\n");
    return STATUS_SUCCESS;
}

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
)
{
    UNREFERENCED_PARAMETER(DriverGlobals);
    UNREFERENCED_PARAMETER(Driver);
    UNREFERENCED_PARAMETER(DesiredAccess);
    UNREFERENCED_PARAMETER(KeyAttributes);

    // This is almost a no-op now
    *Key = (WDFKEY) malloc(1);
    if (*Key == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    return STATUS_SUCCESS;
}

VOID DlWdfRegistryClose(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFKEY Key
)
{
    UNREFERENCED_PARAMETER(DriverGlobals);

    // This is almost a no-op now
    if (Key != NULL) {
        free(Key);
    }
}

NTSTATUS DlWdfRegistryQueryULong(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFKEY Key,
    _In_
    PCUNICODE_STRING ValueName,
    _Out_
    PULONG Value
)
{
#if !defined(_X86_)
    UNICODE_STRING KeyEnableDisplay;
    UNICODE_STRING KeyBootDelay;

    UNREFERENCED_PARAMETER(DriverGlobals);

    if (Key == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    RtlInitUnicodeString(&KeyEnableDisplay, L"EnableDisplay");
    RtlInitUnicodeString(&KeyBootDelay, L"BootDelay");


    if (RtlEqualUnicodeString(&KeyEnableDisplay, ValueName, FALSE)) {
        printf("[INFO] WdfRegistryQueryUlong requesting EnableDisplay key. Returns 0 to disable HDMI control\n");
        OutputDebugString(L"[INFO] WdfRegistryQueryUlong requesting EnableDisplay key. Returns 0 to disable HDMI control\n");
        *Value = 0;
        return STATUS_SUCCESS;
    }

    if (RtlEqualUnicodeString(&KeyBootDelay, ValueName, FALSE)) {
        printf("[INFO] WdfRegistryQueryUlong requesting BootDelay key. Returns 50 for immediately start\n");
        OutputDebugString(L"[INFO] WdfRegistryQueryUlong requesting BootDelay key. Returns 50 for immediately start\n");
        *Value = 50;
        return STATUS_SUCCESS;
    }
#else
    UNREFERENCED_PARAMETER(DriverGlobals);
    UNREFERENCED_PARAMETER(Key);
    UNREFERENCED_PARAMETER(ValueName);
    UNREFERENCED_PARAMETER(Value);
#endif

    return STATUS_OBJECT_NAME_NOT_FOUND;
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

    return (WDFOBJECT) ((PDREAMLIFTER_TIMER)Timer)->ParentObject;
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
        dueTime = (DWORD) ((-DueTime * 100) / 1000000);
    }
    else {
        // Not yet seeing the use case here
        OutputDebugString(L"[WARN] Attempt to use absoulte time for WdfTimer, which is not yet supported\n");
        if (IsDebuggerPresent())
        {
            DebugBreak();
        }
        return FALSE;
    }

    pTimer->Cancelled = FALSE;
    return (BOOLEAN) CreateTimerQueueTimer(
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
    UNREFERENCED_PARAMETER(TimerOrWaitFired);

    if (pTimer->AutomaticSerialization) {
        // Take device mutex if needed
        WaitForSingleObject(g_pDevice->SerializationMutex, INFINITE);
    }

    pTimer->EvtTimerFunc((WDFTIMER)pTimer);
    
    if (pTimer->AutomaticSerialization) {
        ReleaseMutex(g_pDevice->SerializationMutex);
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
    PDREAMLIFTER_TIMER pTimer = (PDREAMLIFTER_TIMER) Timer;

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
    if (Config->AutomaticSerialization == TRUE
        && Attributes->SynchronizationScope != WdfSynchronizationScopeInheritFromParent
        && Attributes->SynchronizationScope != WdfSynchronizationScopeDevice) {
        OutputDebugString(L"[ERROR] The timer serialization mode is not yet supported\n");
        free(pWorkItem);
        return STATUS_NOT_SUPPORTED;
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
    PDREAMLIFTER_WORKITEM pWorkItem = (PDREAMLIFTER_WORKITEM) WorkItem;
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

    if (pWorkItem->AutomaticSerialization) {
        // Take device mutex if needed
        WaitForSingleObject(g_pDevice->SerializationMutex, INFINITE);
    }

    pWorkItem->EvtWorkItemFunc((WDFWORKITEM) pWorkItem);

    if (pWorkItem->AutomaticSerialization) {
        ReleaseMutex(g_pDevice->SerializationMutex);
    }

    return 0;
}
