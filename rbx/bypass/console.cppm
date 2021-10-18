module;

#include <Windows.h>
#include <cstdint>
#include <cstdio>

export module rbx.bypass.console;

export namespace rbx::bypass {
    void console(LPCWSTR title) {
        DWORD old_protection;
        VirtualProtect(static_cast<void*>(FreeConsole), 1, PAGE_READWRITE, &old_protection);
        *reinterpret_cast<std::uint8_t*>(FreeConsole) = 0xC3;
        VirtualProtect(static_cast<void*>(FreeConsole), 1, old_protection, &old_protection);
        AllocConsole();
        FILE* safe_handle_stream;
        SetConsoleTitle(title);
        freopen_s(&safe_handle_stream, "CONIN$", "r", stdin);
        freopen_s(&safe_handle_stream, "CONOUT$", "w", stdout);
        freopen_s(&safe_handle_stream, "CONOUT$", "w", stderr);
    }
}
