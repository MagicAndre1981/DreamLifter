// Mm.c: memory-management related routines.

#include <DreamLifter.h>
#include <DreamLifterKmLoader.h>

extern DL_FUNCTION_DESCRIPTOR m_ntRoutines[];

PVOID DlkMmGetSystemRoutineAddress(
	PUNICODE_STRING SystemRoutineName
)
{
	PDL_FUNCTION_DESCRIPTOR m = m_ntRoutines;
	ANSI_STRING s;

	if (NT_SUCCESS(RtlUnicodeStringToAnsiString(&s, SystemRoutineName, TRUE))) {
		while (m->Version != 0) {
			if (strcmp(m->FunctionName, s.Buffer) == 0)
			{
				return (PVOID) m->Entry;
			}
			m++;
		}

		wprintf(L"[INFO] Requesting system routine %s. Becacuse it is not implemented, returning stub trap\n", SystemRoutineName->Buffer);
	}
	else {
		TrapDebugger("[ERROR] Failed to call RtlUnicodeStringToAnsiString\n");
	}
	
	return (PVOID)DlKmImplementationStub;
}
