#pragma once

DL_FUNCTION_DESCRIPTOR m_ntRoutines[] = {
	// RTL
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
	// MM
	{
		FUNCTION_IMPL_DESCRIPTOR_V1,
		"MmGetSystemRoutineAddress",
		(PVOID) DlkMmGetSystemRoutineAddress
	},
	// PS
	{
		FUNCTION_IMPL_DESCRIPTOR_V1,
		"PsGetVersion",
		(PVOID) DlkPsGetVersion
	},
	// WMI
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
	// ETW
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
	// IO
	{
		FUNCTION_IMPL_DESCRIPTOR_V1,
		"IoWMIRegistrationControl",
		(PVOID) DlkIoWMIRegistrationControl
	},
	// Dbg
	{
		FUNCTION_IMPL_DESCRIPTOR_V1,
		"DbgPrint",
		(PVOID) DlkDbgPrint
	},
	// CRT
	{
		FUNCTION_IMPL_DESCRIPTOR_V1,
		"wcslen",
		(PVOID) Dlkwcslen
	},
	{
		FUNCTION_IMPL_DESCRIPTOR_V1,
		"strlen",
		(PVOID) Dlkstrlen
	},
	{
		FUNCTION_IMPL_DESCRIPTOR_V1,
		"strcpy",
		(PVOID) Dlkstrcpy
	},
	{
		FUNCTION_IMPL_DESCRIPTOR_V1,
		"sprintf_s",
		(PVOID) Dlksprintf_s
	},
	{
		FUNCTION_IMPL_DESCRIPTOR_V1,
		"sprintf",
		(PVOID) Dlksprintf
	},
	{
		FUNCTION_IMPL_DESCRIPTOR_V1,
		"_vsnwprintf",
		(PVOID) Dlk_vsnwprintf
	},
	{
		FUNCTION_IMPL_DESCRIPTOR_V1,
		"_vsnprintf",
		(PVOID) Dlk_vsnprintf
	},
	// Po
	{
		FUNCTION_IMPL_DESCRIPTOR_V1,
		"PoRegisterPowerSettingCallback",
		(PVOID) DlPoRegisterPowerSettingCallback
	},
	// Zw
	{
		FUNCTION_IMPL_DESCRIPTOR_V1,
		"ZwCreateFile",
		(PVOID) DlZwCreateFile
	},
	{
		FUNCTION_IMPL_DESCRIPTOR_V1,
		"ZwClose",
		(PVOID) DlZwClose
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

DL_FUNCTION_DESCRIPTOR m_halRoutines[] = {
	{
		FUNCTION_IMPL_DESCRIPTOR_V1,
		"KeGetCurrentIrql",
		(PVOID) DlKeGetCurrentIrql
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
		MODULE_IMPL_DESCRIPTOR_V1,
		"HAL.dll",
		(PDL_FUNCTION_DESCRIPTOR) m_halRoutines
	},
	{
		// We don't want WPP tracing yet...
		MODULE_IMPL_DESCRIPTOR_V1,
		"WppRecorder.sys",
		(PDL_FUNCTION_DESCRIPTOR) m_wppRoutines
	},
	{
		0
	},
};
