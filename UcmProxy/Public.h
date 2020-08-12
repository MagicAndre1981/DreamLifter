/*++

Module Name:

    public.h

Abstract:

    This module contains the common declarations shared by driver
    and user applications.

Environment:

    driver and application

--*/

//
// Define an Interface Guid so that apps can find the device and talk to it.
//

DEFINE_GUID (GUID_DEVINTERFACE_UcmProxy,
    0xd1f8023f,0xf151,0x4747,0x8e,0x8d,0x19,0x4b,0x08,0xd5,0xe0,0xee);
// {d1f8023f-f151-4747-8e8d-194b08d5e0ee}
