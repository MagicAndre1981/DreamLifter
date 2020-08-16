#pragma once

DL_FUNCTION_DESCRIPTOR m_ntRoutines[] = {
	{
		FUNCTION_IMPL_DESCRIPTOR_V1,
		"RtlCopyUnicodeString",
		(PVOID) DlkRtlCopyUnicodeString
	},
	{
		FUNCTION_IMPL_DESCRIPTOR_V1,
		"RtlInitUnicodeString",
		(PVOID) DlKRtlInitUnicodeString
	},
	{
		FUNCTION_IMPL_DESCRIPTOR_V1,
		"RtlCompareMemory",
		(PVOID) DlkRtlCompareMemory
	},
	{
		FUNCTION_IMPL_DESCRIPTOR_V1,
		"MmGetSystemRoutineAddress",
		(PVOID) DlkMmGetSystemRoutineAddress
	},
	{
		FUNCTION_IMPL_DESCRIPTOR_V1,
		"PsGetVersion",
		(PVOID) DlkPsGetVersion
	},
	{
		FUNCTION_IMPL_DESCRIPTOR_V1,
		"WmiTraceMessage",
		(PVOID) DlkWmiTraceMessage
	},
	{
		FUNCTION_IMPL_DESCRIPTOR_V1,
		"WmiQueryTraceInformation",
		(PVOID) DlkWmiQueryTraceInformation
	},
	{
		FUNCTION_IMPL_DESCRIPTOR_V1,
		"EtwRegisterClassicProvider",
		(PVOID) DlkEtwRegisterClassicProvider
	},
	{
		FUNCTION_IMPL_DESCRIPTOR_V1,
		"EtwUnregister",
		(PVOID) DlkEtwUnregister
	},
	{
		FUNCTION_IMPL_DESCRIPTOR_V1,
		"IoWMIRegistrationControl",
		(PVOID) DlkIoWMIRegistrationControl
	},
	{
		FUNCTION_IMPL_DESCRIPTOR_V1,
		"DbgPrint",
		(PVOID) DlkDbgPrint
	},
	{
		0
	},
};

DL_FUNCTION_DESCRIPTOR m_wdfldrRoutines[] = {
	{
		FUNCTION_IMPL_DESCRIPTOR_V1,
		"WdfVersionBind",
		(PVOID) DlKWdfVersionBind
	},
	{
		FUNCTION_IMPL_DESCRIPTOR_V1,
		"WdfVersionBindClass",
		(PVOID) DlKmWdfVersionBindClass
	},
	{
		FUNCTION_IMPL_DESCRIPTOR_V1,
		"WdfVersionUnbind",
		(PVOID) DlWdfVersionUnbind
	},
	{
		FUNCTION_IMPL_DESCRIPTOR_V1,
		"WdfVersionUnbindClass",
		(PVOID) DlWdfVersionUnbindClass
	},
	{
		0
	},
};

DL_FUNCTION_DESCRIPTOR m_wppRoutines[] = {
	{
		FUNCTION_IMPL_DESCRIPTOR_V1,
		"WppAutoLogTrace",
		(PVOID) DlKmImplementationStubNoOp
	},
	{
		FUNCTION_IMPL_DESCRIPTOR_V1,
		"WppAutoLogStart",
		(PVOID)DlKmImplementationStubNoOp
	},
	{
		0
	},
};


DL_MODULE_IMPLEMENTATION g_DlKmModules[] = {
	{
		MODULE_IMPL_DESCRIPTOR_V1,
		"ntoskrnl.exe",
		(PDL_FUNCTION_DESCRIPTOR) m_ntRoutines
	},
	{
		MODULE_IMPL_DESCRIPTOR_V1,
		"WDFLDR.sys",
		(PDL_FUNCTION_DESCRIPTOR) m_wdfldrRoutines
	},
	{
		// We don't want WPP tracing yet...
		MODULE_IMPL_DESCRIPTOR_V1,
		"WppRecorder.sys",
		(PDL_FUNCTION_DESCRIPTOR)m_wppRoutines
	},
	{
		0
	},
};
