#include <windows.h>
#include <iostream>
int main() {
   DWORD creation_flags;
   STARTUPINFO si;
   PROCESS_INFORMATION pi;
   DWORD exitCode;
   ZeroMemory(&si, sizeof(si));
   si.cb = sizeof(si);
   ZeroMemory(&pi, sizeof(pi));
   //LPCWSTR greeting = L"Hello, Win32!";
   creation_flags = CREATE_UNICODE_ENVIRONMENT;
   wchar_t envBlock[] =
        L"MY_APP_MODE=production\0"
        L"MY_DATA_DIR=C:\\ProgramData\\MyApp\0"
        L"SystemRoot=C:\\Windows\0" // Often needed by Windows DLLs/cmd.exe
        L"\0";                      // Extra null terminator at the end
#if defined(SDL_PLATFORM_MSYS)
   creation_flags |= CREATE_NO_WINDOW;
#endif
//   // Switch stdout to UTF-16 mode so Unicode characters render properly
//   setmode(fileno(stdout), O_U16TEXT);
//   wprintf(L"%s\n", greeting);
   // Command to execute (e.g., "notepad.exe")
   LPCSTR command = "pwd.exe";
   // Create the process
   // CreateProcessW(NULL, createprocess_cmdline, NULL, NULL, TRUE, creation_flags, createprocess_env, createprocess_cwd, &startup_info, &data->process_information)
   if (!CreateProcess(
           NULL, // Application name
           (LPSTR)command, // Command line
           NULL, // Process security attributes
           NULL, // Thread security attributes
           TRUE, // Inherit handles
           creation_flags, // Creation flags
           envBlock, // Environment variables
           NULL, // Current directory
           &si, // Startup info
           &pi)) { // Process info
       std::cerr << "CreateProcess failed (" << GetLastError() << ")." << std::endl;
       return 1;
   }
   // Wait for the process to complete
   WaitForSingleObject(pi.hProcess, INFINITE);
   // Get the exit code of the process
   if (GetExitCodeProcess(pi.hProcess, &exitCode)) {
       std::cout << "Process exited with code: " << exitCode << std::endl;
   } else {
       std::cerr << "Failed to get exit code (" << GetLastError() << ")." << std::endl;
   }
   // Close handles
   CloseHandle(pi.hProcess);
   CloseHandle(pi.hThread);
   return 0;
}
