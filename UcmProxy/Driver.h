/*++

Module Name:

    driver.h

Abstract:

    This file contains the driver definitions.

Environment:

    User-mode Driver Framework 2

--*/

#include <windows.h>
#include <wdf.h>
#include <initguid.h>
#include <ucm/1.0/UcmCx.h>

#include "device.h"
#include "queue.h"
#include "trace.h"

EXTERN_C_START

//
// WDFDRIVER Events
//

DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD UcmProxyEvtDeviceAdd;
EVT_WDF_OBJECT_CONTEXT_CLEANUP UcmProxyEvtDriverContextCleanup;

EXTERN_C_END
