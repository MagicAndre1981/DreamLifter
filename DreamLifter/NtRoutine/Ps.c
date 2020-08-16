// Ps.c: process service?

#include <DreamLifter.h>

UNICODE_STRING g_csdVersion;

BOOLEAN DlkPsGetVersion(
	PULONG          MajorVersion,
	PULONG          MinorVersion,
	PULONG          BuildNumber,
	PUNICODE_STRING CSDVersion
)
{
	if (g_csdVersion.Length < 1) {
		RtlInitUnicodeString(&g_csdVersion, L"");
	}

	if (MajorVersion) {
		*MajorVersion = 10;
	}
	else {
		return FALSE;
	}

	if (MinorVersion) {
		*MinorVersion = 0;
	}

	if (BuildNumber) {
		*BuildNumber = 18363;
	}

	if (CSDVersion) {
		CSDVersion = &g_csdVersion;
	}

	return TRUE;
}
