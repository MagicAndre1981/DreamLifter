// KmCodeLoader.c: the land of DreamLifterK magic
// What it does:
// 1. Load certain arm32 kernel-mode driver code in userland (customized PE loader).
// 2. Fix up IAT and reloc table (we do not care about .pdata yet, km drivers are unlikely 
// to do exception unwind, and even the worst case in um is process crash)
// 3. Kick driver in, connect to the km IO proxy driver
// 4. **Popcorn**
// 
// Generally speaking, the km code needs to be compliant with a few constraints:
// - Standard MS ARMNT calling convention
// - No direct memory access or MMIO operations (technically we can map memory twice though)
// - No non-passive level interrupt (same as UMDF)
// - No interface query to other drivers, or providing interface to drivers
// - No exception unwind (yet)
//
// As investigated, HDMIDPI.sys is compliant with these constraints, and therefore it is chosen
// to be an experiment subject.

#include <DreamLifter.h>
#include <DreamLifterKmLoader.h>
#include <KmModuleDescriptor.h>

static DWORD DlKLdrGetDOSHeader(LPVOID pvData, DWORD cbSize, IMAGE_DOS_HEADER** pDOSHeader)
{
	DWORD dwStatus = ERROR_INVALID_PARAMETER;

	if (pDOSHeader)
	{
		if (pvData &&
			cbSize > sizeof(IMAGE_DOS_HEADER))
		{
			*pDOSHeader = (IMAGE_DOS_HEADER*)pvData;

			if ((*pDOSHeader)->e_magic == IMAGE_DOS_SIGNATURE)
			{
				dwStatus = ERROR_SUCCESS;
			}
			else
			{
				dwStatus = ERROR_INVALID_MODULETYPE;
			}

		}
		else
		{
			dwStatus = ERROR_INVALID_DATA;
		}
	}

	return dwStatus;
}

static DWORD DlKLdrGetNTHeaders(LPVOID pvData, DWORD cbSize, IMAGE_NT_HEADERS** pNTHeader)
{
	DWORD dwStatus = ERROR_INVALID_PARAMETER;

	if (pNTHeader)
	{
		PIMAGE_DOS_HEADER pDOSHeader = NULL;
		dwStatus = DlKLdrGetDOSHeader(pvData, cbSize, &pDOSHeader);

		if (ERROR_SUCCESS == dwStatus && pDOSHeader != NULL)
		{
			CONST ULONGLONG REMAIN = cbSize - sizeof(IMAGE_DOS_HEADER) - sizeof(IMAGE_OPTIONAL_HEADER);

			if (pDOSHeader->e_lfanew < REMAIN)
			{
				*pNTHeader = (IMAGE_NT_HEADERS*)(((UINT_PTR)pDOSHeader) + pDOSHeader->e_lfanew);
				if ((*pNTHeader)->Signature == IMAGE_NT_SIGNATURE)
				{
					dwStatus = ERROR_SUCCESS;
				}
				else
				{
					dwStatus = ERROR_INVALID_MODULETYPE;
				}
			}
			else
			{
				dwStatus = ERROR_INVALID_DATA;
			}
		}
	}

	return dwStatus;
}

IMAGE_BASE_RELOCATION* WINAPI LdrProcessRelocationBlock(void* page, UINT count,
	USHORT* relocs, INT_PTR delta)
{
	while (count--)
	{
		USHORT offset = *relocs & 0xfff;
		int type = *relocs >> 12;
		switch (type)
		{
		case IMAGE_REL_BASED_ABSOLUTE:
			break;
		case IMAGE_REL_BASED_HIGH:
			*(short*)((char*)page + offset) += HIWORD(delta);
			break;
		case IMAGE_REL_BASED_LOW:
			*(short*)((char*)page + offset) += LOWORD(delta);
			break;
		case IMAGE_REL_BASED_HIGHLOW:
			*(int*)((char*)page + offset) += delta;
			break;
		case IMAGE_REL_BASED_DIR64:
			*(INT_PTR*)((char*)page + offset) += delta;
			break;
		case IMAGE_REL_BASED_THUMB_MOV32:
		{
			DWORD inst = *(INT_PTR*)((char*)page + offset);
			DWORD imm16 = ((inst << 1) & 0x0800) + ((inst << 12) & 0xf000) +
				((inst >> 20) & 0x0700) + ((inst >> 16) & 0x00ff);
			DWORD hi_delta;

			if ((inst & 0x8000fbf0) != 0x0000f240)
				printf("[ERROR] Wrong Thumb2 instruction %08x, expected MOVW\n", inst);

			imm16 += LOWORD(delta);
			hi_delta = HIWORD(delta) + HIWORD(imm16);
			*(INT_PTR*)((char*)page + offset) = (inst & 0x8f00fbf0) + ((imm16 >> 1) & 0x0400) +
				((imm16 >> 12) & 0x000f) +
				((imm16 << 20) & 0x70000000) +
				((imm16 << 16) & 0xff0000);

			if (hi_delta != 0)
			{
				inst = *(INT_PTR*)((char*)page + offset + 4);
				imm16 = ((inst << 1) & 0x0800) + ((inst << 12) & 0xf000) +
					((inst >> 20) & 0x0700) + ((inst >> 16) & 0x00ff);

				if ((inst & 0x8000fbf0) != 0x0000f2c0)
					printf("[ERROR] Wrong Thumb2 instruction %08x, expected MOVT\n", inst);

				imm16 += hi_delta;
				if (imm16 > 0xffff)
					printf("[ERROR] Resulting immediate value won't fit: %08x\n", imm16);
				*(INT_PTR*)((char*)page + offset + 4) = (inst & 0x8f00fbf0) +
					((imm16 >> 1) & 0x0400) +
					((imm16 >> 12) & 0x000f) +
					((imm16 << 20) & 0x70000000) +
					((imm16 << 16) & 0xff0000);
			}
		}
		break;
		default:
			printf("[WARN] Unknown/unsupported fixup type %x.\n", type);
			return NULL;
		}
		relocs++;
	}
	return (IMAGE_BASE_RELOCATION*) relocs;  /* return address of next block */
}

PDRIVER_MODULE DlKmLoadModule()
{
	HANDLE hFile = NULL;
	DWORD fileSize = 0;
	DWORD fileSizeHigh = 0;
	PUCHAR pFileBuffer = NULL;
	PIMAGE_NT_HEADERS pImageNtHeader = NULL;

	DWORD ret = 0;
	HMODULE hModule = NULL;
	DWORD oldProtect = 0;

	hFile = CreateFileW(
		L"C:\\Windows\\DreamLifter\\hdmidpi.sys",
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (hFile == INVALID_HANDLE_VALUE) {
		printf("[ERROR] CreateFileW failed, error %d\n", GetLastError());
		goto exit;
	}

	fileSize = GetFileSize(hFile, &fileSizeHigh);
	// Sanity check: the file should be small enough (we don't care about higher bits)
	if (fileSize == 0 || fileSizeHigh != 0) {
		goto exit;
	}

	// TODO: check file signature here for security :D

	pFileBuffer = malloc((size_t) fileSize);
	if (pFileBuffer == NULL) {
		printf("[ERROR] Failed to allocate memory\n");
		goto exit;
	}

	RtlZeroMemory(pFileBuffer, (size_t)fileSize);
	if (!ReadFile(hFile, (LPVOID)pFileBuffer, fileSize, NULL, NULL)) {
		printf("[ERROR] ReadFileEx failed, error %d\n", GetLastError());
		goto exit;
	}

	ret = DlKLdrGetNTHeaders(pFileBuffer, fileSize, &pImageNtHeader);
	if (ret != 0 || pImageNtHeader == NULL) {
		printf("[ERROR] Image doesn't present valid NT image header\n");
		goto exit;
	}

	// Architecture: only host arm32 drivers yet
	if (pImageNtHeader->FileHeader.Machine != IMAGE_FILE_MACHINE_ARMNT) {
		printf("[ERROR] Present image is not valid arm32 NT image\n");
		goto exit;
	}

	// Constraint: only native image
	if (pImageNtHeader->OptionalHeader.Subsystem != IMAGE_SUBSYSTEM_NATIVE) {
		printf("[ERROR] Present image is not valid native subsystem image\n");
		goto exit;
	}

	// Prepare memory region, once memory is copied, mark it as read and executable
	hModule = (HMODULE) VirtualAlloc(NULL,
		pImageNtHeader->OptionalHeader.SizeOfImage,
		MEM_RESERVE | MEM_COMMIT,
		PAGE_READWRITE
	);

	printf("[INFO] VirtualAlloc @ 0x%x\n", (UINT_PTR) hModule);

	if (hModule == 0) {
		printf("[ERROR] Failed to allocate memory for execution, error %d\n", GetLastError());
		goto exit;
	}

	// Copy header and sections
	RtlCopyMemory(hModule, pFileBuffer, pImageNtHeader->OptionalHeader.SizeOfHeaders);
	CONST IMAGE_SECTION_HEADER* pFirst = IMAGE_FIRST_SECTION(pImageNtHeader);
	if (pFirst)
	{
		CONST DWORD dwNumSection = pImageNtHeader->FileHeader.NumberOfSections;
		if (dwNumSection <= 0)
		{
			printf("[ERROR] Failed to locate sections in the image file\n");
			goto exit;
		}

		for (DWORD dwCurrent = 0; dwCurrent < dwNumSection; ++dwCurrent)
		{
			CONST IMAGE_SECTION_HEADER* pCurrent = &pFirst[dwCurrent];

			LPVOID pDest = (LPVOID)((UINT_PTR) hModule + pCurrent->VirtualAddress);
			LPCVOID pSrc = (LPCVOID)((UINT_PTR) pFileBuffer + pCurrent->PointerToRawData);
			DWORD SectionSize = pCurrent->SizeOfRawData;

			if (SectionSize == 0)
			{
				if (pCurrent->Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA)
				{
					SectionSize = pImageNtHeader->OptionalHeader.SizeOfInitializedData;
				}
				else if (pCurrent->Characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA)
				{
					SectionSize = pImageNtHeader->OptionalHeader.SizeOfUninitializedData;
				}
				else
				{
					continue;
				}
			}

			printf("[INFO] Section %s, VA 0x%x, PointerToRawData 0x%x\n", pCurrent->Name, pCurrent->VirtualAddress, pCurrent->PointerToRawData);
			RtlCopyMemory(pDest, pSrc, SectionSize);
		}
	}
	else
	{
		printf("[ERROR] Failed to locate sections in the image file\n");
		goto exit;
	}

	// Process imports
	CONST IMAGE_DATA_DIRECTORY ImageDir = pImageNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	PIMAGE_IMPORT_DESCRIPTOR pImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR) ((UINT_PTR) hModule + (ImageDir.VirtualAddress));
	if (pImportDescriptor == NULL) {
		printf("[ERROR] Failed to locate import section\n");
		goto exit;
	}

	while (pImportDescriptor->Name != 0 && ret == ERROR_SUCCESS) {
		LPCSTR szLibraryName = (LPCSTR)((UINT_PTR) hModule + pImportDescriptor->Name);

		// Log only. Later we will connect them to our actual implementation,
		// Now just leave everything as stub.

		PIMAGE_THUNK_DATA pThunk = NULL;
		PIMAGE_THUNK_DATA pAddrThunk = NULL;

		if (pImportDescriptor->OriginalFirstThunk)
		{
			pThunk = (PIMAGE_THUNK_DATA)((UINT_PTR) hModule + pImportDescriptor->OriginalFirstThunk);
		}
		else
		{
			pThunk = (PIMAGE_THUNK_DATA)((UINT_PTR) hModule + pImportDescriptor->FirstThunk);
		}

		pAddrThunk = (PIMAGE_THUNK_DATA)((UINT_PTR) hModule + pImportDescriptor->FirstThunk);

		while (pAddrThunk && pThunk && pThunk->u1.AddressOfData && ret == ERROR_SUCCESS)
		{
			PDL_MODULE_IMPLEMENTATION m = g_DlKmModules;
			PDL_FUNCTION_DESCRIPTOR f = NULL;
			LPCSTR functionName = NULL;
			BOOLEAN bBuiltIn = FALSE;
			BOOLEAN bFound = FALSE;

			while (m->Version != 0) {
				if (strcasecmp(m->ModuleName, szLibraryName) == 0) {
					bBuiltIn = TRUE;
					break;
				}
				m++;
			}

			if (IMAGE_SNAP_BY_ORDINAL(pThunk->u1.Ordinal))
			{
				functionName = (LPCSTR) IMAGE_ORDINAL(pAddrThunk->u1.Ordinal);
			}
			else
			{
				PIMAGE_IMPORT_BY_NAME pImport = (PIMAGE_IMPORT_BY_NAME)((UINT_PTR) hModule + pThunk->u1.AddressOfData);
				functionName = (LPCSTR) pImport->Name;
			}

			if (bBuiltIn) {
				f = m->Functions;
				while (f->Version != 0) {
					if (strcmp(f->FunctionName, functionName) == 0) {
						bFound = TRUE;
						pAddrThunk->u1.Function = (UINT_PTR) f->Entry;
						printf("[INFO] Import %s from %s, link it to internal implementation\n", functionName, szLibraryName);
						break;
					}
					f++;
				}
			}

			if (!bFound) {
				printf("[INFO] Import %s from %s, link it to DlKmImplementationStub\n", functionName, szLibraryName);
				pAddrThunk->u1.Function = (UINT_PTR)(PVOID)DlKmImplementationStub;
			}

			++pThunk;
			++pAddrThunk;
		}

		pImportDescriptor++;
	}

	// Process relocation
	CONST IMAGE_DATA_DIRECTORY RelocDir = pImageNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
	CONST DWORD RelocDirSize = RelocDir.Size;
	if (RelocDirSize > 0)
	{
		PIMAGE_BASE_RELOCATION pRelocTable = (PIMAGE_BASE_RELOCATION)((UINT_PTR)hModule + (RelocDir.VirtualAddress));
		PIMAGE_BASE_RELOCATION pEnd = (PIMAGE_BASE_RELOCATION)((UINT_PTR) pRelocTable + (RelocDir.Size));
		if (pRelocTable == NULL) {
			printf("[ERROR] Failed to locate base relocation section\n");
			goto exit;
		}

		CONST UINT_PTR Difference = ((UINT_PTR)hModule - pImageNtHeader->OptionalHeader.ImageBase);
		while (pRelocTable < pEnd && pRelocTable->SizeOfBlock)
		{
			void* page = (char*)hModule + pRelocTable->VirtualAddress;
			pRelocTable = LdrProcessRelocationBlock(
				page,
				(pRelocTable->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(USHORT),
				(USHORT*)(pRelocTable + 1),
				Difference
			);
		}
	}

	// Lock pages down
	// Give it a security cookie
	// The second data at .data section
	pFirst = IMAGE_FIRST_SECTION(pImageNtHeader);
	if (pFirst)
	{
		CONST DWORD dwNumSection = pImageNtHeader->FileHeader.NumberOfSections;
		if (dwNumSection <= 0)
		{
			printf("[ERROR] Failed to locate sections in the image file\n");
			goto exit;
		}

		for (DWORD dwCurrent = 0; dwCurrent < dwNumSection; ++dwCurrent)
		{
			CONST IMAGE_SECTION_HEADER* pCurrent = &pFirst[dwCurrent];
			LPVOID pDest = (LPVOID)((UINT_PTR)hModule + pCurrent->VirtualAddress);
			DWORD SectionSize = pCurrent->SizeOfRawData;
			DWORD SectionProtectionFlags = 0;

			// Security cookie is the second DWORD at .data section
			if (strcmp(".data", (const char*) pCurrent->Name) == 0) {
				// TODO: Generate a crypto safe cookie canary here
				// For debugging purpose, I filled a hard-coded value,
				// taken from the first few bytes of katsuragishoko.
				*((DWORD*)((UINT_PTR)pDest + 4)) = 0x6B617473;
			}

			if (pCurrent->Characteristics & IMAGE_SCN_MEM_READ) {
				SectionProtectionFlags = PAGE_READONLY;
			}

			if (pCurrent->Characteristics & IMAGE_SCN_MEM_READ &&
				(pCurrent->Characteristics & IMAGE_SCN_MEM_WRITE)) {
				SectionProtectionFlags = PAGE_READWRITE;
			}

			if ((pCurrent->Characteristics & IMAGE_SCN_MEM_EXECUTE) &&
				(pCurrent->Characteristics & IMAGE_SCN_MEM_READ)) {
				SectionProtectionFlags = PAGE_EXECUTE_READ;
			}

			if ((pCurrent->Characteristics & IMAGE_SCN_MEM_EXECUTE) &&
				(pCurrent->Characteristics & IMAGE_SCN_MEM_READ) &&
				(pCurrent->Characteristics & IMAGE_SCN_MEM_WRITE)) {
				printf("[WARN] Section %s is not W^X enforced\n", pCurrent->Name);
				SectionProtectionFlags = PAGE_EXECUTE_READWRITE;
			}

			if (SectionProtectionFlags != 0) {
				if (!VirtualProtect(pDest,
					SectionSize,
					SectionProtectionFlags,
					&oldProtect)
					) {
					printf("[ERROR] Failed to set protection type for R^X, error %d\n", GetLastError());
					goto exit;
				}
			}
		}
	}

	// Provide entry point information
	PDRIVER_MODULE pModuleInformation = malloc(sizeof(DRIVER_MODULE));
	if (pModuleInformation == NULL) {
		printf("[ERROR] Failed to allocate memory for module information\n");
		goto exit;
	}

	pModuleInformation->Module = hModule;
	pModuleInformation->ModuleSize = pImageNtHeader->OptionalHeader.SizeOfImage;
	pModuleInformation->Entry = (PFN_WDM_DRIVER_ENTRY) ((UINT_PTR) hModule + pImageNtHeader->OptionalHeader.AddressOfEntryPoint);
	free(pFileBuffer);
	if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
	return pModuleInformation;

exit:
	if (hModule != NULL && pImageNtHeader->OptionalHeader.SizeOfImage > 0) {
#pragma warning(disable:6333)
		VirtualFree(hModule, pImageNtHeader->OptionalHeader.SizeOfImage, MEM_RELEASE);
#pragma warning(default:6333)
	}
	free(pFileBuffer);
	if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
	return NULL;
}

void DlKmImplementationStub()
{
	// Calling a routine that is not yet implemented.
	printf("[ERROR] Calling a unimplemented kernel routine\n");

	if (IsDebuggerPresent())
	{
		DebugBreak();
	}

	CpuDeadLoop();
}

void DlKmImplementationStubNoOp()
{
	// No-op
}

void DlKmLoaderStart()
{
	PDRIVER_MODULE driverModule = DlKmLoadModule();
	UNICODE_STRING FakeRegString;
	PVOID pDriverObjectStub = (PVOID) driverModule;
	NTSTATUS status = STATUS_SUCCESS;

	DlFxLdrGetWdfKmdf0115Functions();
	RtlInitUnicodeString(&FakeRegString, L"HKLM\\Something");
	if (driverModule != NULL && driverModule->Entry != NULL) {
		status = driverModule->Entry(pDriverObjectStub, &FakeRegString);
	}
}
