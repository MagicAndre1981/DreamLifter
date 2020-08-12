// UcmEnumDebugUtility.c: prints human-friendly enum strings.

#include <DreamLifter.h>

char* DbgUcmGetOperatingMode(UCM_TYPEC_OPERATING_MODE mode) {
	switch (mode) {
	case UcmTypeCOperatingModeInvalid:
		return "UcmTypeCOperatingModeInvalid";
	case UcmTypeCOperatingModeDfp:
		return "UcmTypeCOperatingModeDfp";
	case UcmTypeCOperatingModeUfp:
		return "UcmTypeCOperatingModeUfp";
	case UcmTypeCOperatingModeDrp:
		return "UcmTypeCOperatingModeDrp";
	default:
		return "Unknown";
	}
}

char* DbgUcmGetPartner(UCM_TYPEC_PARTNER partner) {
	switch (partner) {
	case UcmTypeCPartnerInvalid:
		return "UcmTypeCPartnerInvalid";
	case UcmTypeCPartnerUfp:
		return "UcmTypeCPartnerUfp";
	case UcmTypeCPartnerDfp:
		return "UcmTypeCPartnerDfp";
	case UcmTypeCPartnerPoweredCableNoUfp:
		return "UcmTypeCPartnerPoweredCableNoUfp";
	case UcmTypeCPartnerPoweredCableWithUfp:
		return "UcmTypeCPartnerPoweredCableWithUfp";
	case UcmTypeCPartnerAudioAccessory:
		return "UcmTypeCPartnerAudioAccessory";
	case UcmTypeCPartnerDebugAccessory:
		return "UcmTypeCPartnerDebugAccessory";
	default:
		return "Unknown";
	}
}

char* DbgUcmGetCurrent(UCM_TYPEC_CURRENT current) {
	switch (current) {
	case UcmTypeCCurrentInvalid:
		return "UcmTypeCCurrentInvalid";
	case UcmTypeCCurrentDefaultUsb:
		return "UcmTypeCCurrentDefaultUsb";
	case UcmTypeCCurrent1500mA:
		return "UcmTypeCCurrent1500mA";
	case UcmTypeCCurrent3000mA:
		return "UcmTypeCCurrent3000mA";
	default:
		return "Unknown";
	}
}

char* DbgUcmGetPowerRole(UCM_POWER_ROLE role) {
	switch (role) {
	case UcmPowerRoleInvalid:
		return "UcmPowerRoleInvalid";
	case UcmPowerRoleSink:
		return "UcmPowerRoleSink";
	case UcmPowerRoleSource:
		return "UcmPowerRoleSource";
	default:
		return "Unknown";
	}
}

char* DbgUcmGetDataRole(UCM_DATA_ROLE role) {
	switch (role) {
	case UcmDataRoleInvalid:
		return "UcmDataRoleInvalid";
	case UcmDataRoleDfp:
		return "UcmDataRoleDfp";
	case UcmDataRoleUfp:
		return "UcmDataRoleUfp";
	default:
		return "Unknown";
	}
}

char* DbgUcmGetPdConnState(UCM_PD_CONN_STATE state) {
	switch (state) {
	case UcmPdConnStateInvalid:
		return "UcmPdConnStateInvalid";
	case UcmPdConnStateNotSupported:
		return "UcmPdConnStateNotSupported";
	case UcmPdConnStateNegotiationFailed:
		return "UcmPdConnStateNegotiationFailed";
	case UcmPdConnStateNegotiationSucceeded:
		return "UcmPdConnStateNegotiationSucceeded";
	default:
		return "Unknown";
	}
}

char* DbgUcmGetChargingState(UCM_CHARGING_STATE state) {
	switch (state) {
	case UcmChargingStateInvalid:
		return "UcmChargingStateInvalid";
	case UcmChargingStateNotCharging:
		return "UcmChargingStateNotCharging";
	case UcmChargingStateNominalCharging:
		return "UcmChargingStateNominalCharging";
	case UcmChargingStateSlowCharging:
		return "UcmChargingStateSlowCharging";
	case UcmChargingStateTrickleCharging:
		return "UcmChargingStateTrickleCharging";
	default:
		return "Unknown";
	}
}

char* DbgUcmGetPdoType(UCM_PD_POWER_DATA_OBJECT_TYPE type) {
	switch (type) {
	case UcmPdPdoTypeFixedSupply:
		return "UcmPdPdoTypeFixedSupply";
	case UcmPdPdoTypeBatterySupply:
		return "UcmPdPdoTypeBatterySupply";
	case UcmPdPdoTypeVariableSupplyNonBattery:
		return "UcmPdPdoTypeVariableSupplyNonBattery";
	default:
		return "Unknown";
	}
}