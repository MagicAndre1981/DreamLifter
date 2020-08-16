// Wdf.c: commonly-shared code between KMDF and UMDF

#include <DreamLifter.h>

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
        OutputDebugString(L"[WARN] Attempt to use absoulte time for WdfTimer, which is not yet supported\n");
        if (IsDebuggerPresent())
        {
            DebugBreak();
        }
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
    if (Config->AutomaticSerialization == TRUE) {
        OutputDebugString(L"[ERROR] The workitem serialization mode is not yet supported\n");
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
    PDREAMLIFTER_WORKITEM pWorkItem = (PDREAMLIFTER_WORKITEM)lpParam;
    pWorkItem->EvtWorkItemFunc((WDFWORKITEM)pWorkItem);
    return 0;
}
