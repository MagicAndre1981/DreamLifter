// Po.c: Power Management wrapper

#include <DreamLifter.h>
#include <powrprof.h>
#include <powersetting.h>

NTSTATUS DlPoRegisterPowerSettingCallback(
	PVOID					DeviceObject,
	LPCGUID                 SettingGuid,
	PPOWER_SETTING_CALLBACK Callback,
	PVOID                   Context,
	PVOID* Handle
)
{
	DWORD ret;

	HPOWERNOTIFY m_notifyHandle;
	DEVICE_NOTIFY_SUBSCRIBE_PARAMETERS m_notifyParams;
	PDL_POWERSETTINGS_CALLBACK_CONTEXT m_win32CallbackContext;

	UNREFERENCED_PARAMETER(DeviceObject);

	if (SettingGuid == NULL || Callback == NULL) {
		return STATUS_INVALID_PARAMETER;
	}

	// Redirect this to PowerSettingRegisterNotification
	m_win32CallbackContext = malloc(sizeof(DL_POWERSETTINGS_CALLBACK_CONTEXT));
	if (m_win32CallbackContext == NULL) {
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	RtlZeroMemory(m_win32CallbackContext, sizeof(DL_POWERSETTINGS_CALLBACK_CONTEXT));
	m_win32CallbackContext->DriverCallback = Callback;
	m_win32CallbackContext->DriverContext = Context;
	RtlCopyMemory(&m_win32CallbackContext->PowerSettingsGuid, SettingGuid, sizeof(GUID));

	m_notifyParams.Callback = DlPoSettingsCallback;
	m_notifyParams.Context = m_win32CallbackContext;
	ret = PowerSettingRegisterNotification(
		SettingGuid,
		DEVICE_NOTIFY_CALLBACK,
		&m_notifyParams,
		&m_notifyHandle
	);

	if (ret) {
		TrapDebugger("PowerSettingRegisterNotification failed");
		return STATUS_UNSUCESSFUL;
	}

	if (Handle != NULL) {
		*Handle = m_notifyHandle;
	}

	return STATUS_SUCCESS;
}

DWORD WINAPI DlPoSettingsCallback(PVOID Context, ULONG Type, PVOID Setting)
{
	PDL_POWERSETTINGS_CALLBACK_CONTEXT pWin32Context = (PDL_POWERSETTINGS_CALLBACK_CONTEXT) Context;
	UNREFERENCED_PARAMETER(Type);

	if (pWin32Context != NULL) {
		// Note: we hardcoded DWORD size because we know it is
		// 6FE69556-704A-47A0-8F24-C28D936FDA47,
		// GUID_CONSOLE_DISPLAY_STATE
		pWin32Context->DriverCallback(&pWin32Context->PowerSettingsGuid,
			Setting, sizeof(DWORD), pWin32Context->DriverContext);
	}
	return 0;
}
