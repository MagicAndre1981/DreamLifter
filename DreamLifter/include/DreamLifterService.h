#pragma once

#define DL_SERVICE_NAME  L"DreamLifter USB Type-C Controller Service"
#define DL_STANDALONE_SWITCH "--standalone"

VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv);
VOID WINAPI ServiceCtrlHandler(DWORD);
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);
