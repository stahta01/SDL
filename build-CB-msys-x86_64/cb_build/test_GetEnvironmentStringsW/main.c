#include <windows.h>
#include <wchar.h>
#include <stdio.h>

int main() {
LPWCH envStrings = GetEnvironmentStringsW();
if (envStrings == NULL) {
printf("%s\n", "Failed to retrieve environment strings.");
return 1;
}

// Iterate through the environment block
for (LPWCH current = envStrings; *current; current += wcslen(current) + 1) {
  wprintf(L"Wide string: %ls\n", current);
}

// Free the allocated memory
FreeEnvironmentStringsW(envStrings);
return 0;
}
