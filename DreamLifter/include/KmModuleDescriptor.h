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
		0
	},
};
