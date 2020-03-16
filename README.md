## About
This is a reliable way to run code at shutdown on Windows.
A service that accepts `SERVICE_CONTROL_PRESHUTDOWN` is allowed to delay the system shutdown up to 125 seconds (provided it asks for it).
The 125 second limit can be extended by editing `HKLM:\SYSTEM\CurrentControlSet\Control\WaitToKillServiceTimeout`.
See [Service Control Handler Function](https://docs.microsoft.com/en-us/windows/win32/services/service-control-handler-function) for more details.

This is an alternative to these unreliable methods of running code at shutdown:
- [Group Policy Shutdown](https://docs.microsoft.com/en-us/previous-versions/windows/it-pro/windows-server-2012-r2-and-2012/dn789190(v%3Dws.11)) can be terminated early, doesn't fire when fastboot is enabled (default in Win10+) and isn't supported on Server Core 1909
- `Register-WmiEvent -Class Win32_ComputerShutdownEvent` can be terminated early, doesn't fire when fastboot is enabled (default in Win10+) and fails on Server Core 1909
- Task event trigger on event id 1074 (`<Select Path="System">*[System[Provider[@Name='User32'] and EventID=1074]]</Select>`) depends on EventLog and TaskScheduler and doesn't fire reliably (or at all on Server Core 1909)

The code is basically this [reference implementation](https://docs.microsoft.com/en-us/windows/win32/services/the-complete-service-sample) with minor changes.

## How to use
- Download [OnShutdown](https://github.com/gfody/OnShutdown/releases/download/v1.0/OnShutdown.exe) to some location in your PATH
- Run `sc create OnShutdown binpath= "OnShutdown 30000 \"powershell -c { ... }\"" start= auto` ([the escaping can be tricky](https://stackoverflow.com/a/11084834/99691))
- The first parameter is the [dwWaitHint](https://docs.microsoft.com/en-us/windows/win32/api/winsvc/ns-winsvc-service_status) in milliseconds
- The second parameter is the command to pass through to [CreateProcess](https://github.com/gfody/OnShutdown/blob/master/ServiceMain.cpp#L60)

