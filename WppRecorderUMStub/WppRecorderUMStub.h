// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the WPPRECORDERUMSTUB_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// WPPRECORDERUMSTUB_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef WPPRECORDERUMSTUB_EXPORTS
#define WPPRECORDERUMSTUB_API __declspec(dllexport)
#else
#define WPPRECORDERUMSTUB_API __declspec(dllimport)
#endif

WPPRECORDERUMSTUB_API void WppAutoLogStop();
WPPRECORDERUMSTUB_API void WppAutoLogStart();
WPPRECORDERUMSTUB_API void WppAutoLogTrace();
