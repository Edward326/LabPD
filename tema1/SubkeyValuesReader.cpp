#include <windows.h>
#include <iostream>
#include <tchar.h>

#define maxValueNameLength 64
#define maxValueDataLength 256

void ListRegistryValues(LPCTSTR subKeyPath) {
    HKEY hKey;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, subKeyPath, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        fwprintf(stderr, L"Error: Cannot open the registry key: %ls\n", subKeyPath);
        return;
    }

    DWORD valuesCount = 0;
    DWORD maxValueNameLen = 0;

    if (RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL, &valuesCount, &maxValueNameLen, NULL, NULL, NULL) != ERROR_SUCCESS) {
        fwprintf(stderr, L"Error querying key information.\n");
        RegCloseKey(hKey);
        return;
    }

    if (valuesCount == 0) {
        _tprintf(L"The key '%ls' has no values.\n", subKeyPath);
    }
    else {
        WCHAR valueName[maxValueNameLength];
        BYTE valueData[maxValueDataLength];

        for (DWORD i = 0; i < valuesCount; i++) {
            DWORD nameSize = maxValueNameLength;
            DWORD dataSize = maxValueDataLength;
            DWORD type = 0;

            if (RegEnumValue(hKey, i, valueName, &nameSize, NULL, &type, valueData, &dataSize) == ERROR_SUCCESS) {
                _tprintf(L"\n\n%lu. %ls\n", i + 1, (nameSize == 0) ? L"(Default)" : valueName);

                switch (type) {
                case REG_SZ:
                    _tprintf(L"\tType: REG_SZ\n\tValue: %ls", (TCHAR*)valueData);
                    break;
                case REG_EXPAND_SZ:
                    _tprintf(L"\tType: REG_EXPAND_SZ\n\tValue: %ls", (TCHAR*)valueData);
                    break;
                case REG_DWORD:
                    _tprintf(L"\tType: REG_DWORD\n\tValue: %lu\n", *(DWORD*)valueData);
                    break;
                case REG_MULTI_SZ:
                    _tprintf(L"\tType: REG_MULTI_SZ");
                    break;
                case REG_BINARY:
                    _tprintf(L"\tType: REG_BINARY");
                    break;
                default:
                    _tprintf(L"\tUnknown Type");
                    break;
                }
            }
            else
                _tprintf(L"Error getting info about Value no %lu", i);
        }
    }

    RegCloseKey(hKey);
}

int main() {
    LPCTSTR pathWithValues = L"SYSTEM\\CurrentControlSet\\Control\\Session Manager";
    ListRegistryValues(pathWithValues);
    return 0;
}