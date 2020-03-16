#include <windows.h>
#include <tchar.h>
#include <string>

#define SERVICE_NAME TEXT("OnShutdown")

SERVICE_STATUS service_status = {};
SERVICE_STATUS_HANDLE status_handle = NULL;
HANDLE stop_event = CreateEvent(NULL, TRUE, FALSE, NULL);
TCHAR* shell_command;
DWORD wait_hint;

VOID WINAPI ServiceMain(DWORD, LPTSTR*);
DWORD WINAPI ServiceCtrlHandlerEx(DWORD, DWORD, LPVOID, LPVOID);

void _tmain(int argc, TCHAR* argv[])
{
    if (argc < 3)
        _tprintf(TEXT("Usage: %s <wait hint in ms> <command>"), SERVICE_NAME);
    else
    {
        wait_hint = std::stoi(argv[1]);
        shell_command = argv[2];
        SERVICE_TABLE_ENTRY DispatchTable[] = { {SERVICE_NAME, ServiceMain}, {NULL, NULL} };
        StartServiceCtrlDispatcher(DispatchTable);
    }
}

VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv)
{
    status_handle = RegisterServiceCtrlHandlerEx(SERVICE_NAME, ServiceCtrlHandlerEx, NULL);

    service_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    service_status.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PRESHUTDOWN;
    service_status.dwCurrentState = SERVICE_RUNNING;
    service_status.dwWin32ExitCode = NO_ERROR;
    SetServiceStatus(status_handle, &service_status);

    WaitForSingleObject(stop_event, INFINITE);
    
    service_status.dwCurrentState = SERVICE_STOPPED;
    service_status.dwCheckPoint++;
    SetServiceStatus(status_handle, &service_status);

    CloseHandle(stop_event);
}

DWORD WINAPI ServiceCtrlHandlerEx(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
{
    switch (dwControl) // intentional fall through..
    {
        case SERVICE_CONTROL_PRESHUTDOWN:
        {
            service_status.dwCurrentState = SERVICE_STOP_PENDING;
            service_status.dwCheckPoint++;
            service_status.dwWaitHint = wait_hint;
            SetServiceStatus(status_handle, &service_status);

            PROCESS_INFORMATION pi = {}; STARTUPINFO si = { sizeof(si) };
            if (CreateProcess(NULL, shell_command, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
            {
                WaitForSingleObject(pi.hProcess, INFINITE);
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
            }
        }
        case SERVICE_CONTROL_STOP: SetEvent(stop_event);
        default: return NO_ERROR;
    }
}
