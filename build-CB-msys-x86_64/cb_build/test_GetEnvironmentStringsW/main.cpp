#include <windows.h>
#include <iostream>

int main() {
LPWCH envStrings = GetEnvironmentStringsW();
if (envStrings == NULL) {
std::cerr << "Failed to retrieve environment strings." << std::endl;
return 1;
}

// Iterate through the environment block
LPWCH current = envStrings;
while (*current) {
std::wcout << current << std::endl;
current += wcslen(current) + 1; // Move to the next variable
}

// Free the allocated memory
FreeEnvironmentStringsW(envStrings);
return 0;
}
