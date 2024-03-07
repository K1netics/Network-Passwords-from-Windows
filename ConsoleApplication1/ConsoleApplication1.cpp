#include <windows.h>
#include <wlanapi.h>
#include <iostream>
#include <string>
#include <vector>

#pragma comment(lib, "wlanapi.lib")
//WlanCloseHandle, WlanEnumInterface, WlanFreeMemory,
// WlanGetProfile, WlanGetProfileList, WlanOpenHandle, WLAN_API_VERSION_2_0, WLAN_INTERFACE_INFO
//WLAN_PROFILE_GET_PLAINTEXT_KEY, WLAN_PROFILE_INFO_LIST

HANDLE open_wlan_handle(int api_version) {
	DWORD negotiated_version = 0;
	HANDLE pWlanHandle = INVALID_HANDLE_VALUE;
	DWORD result = WlanOpenHandle(api_version, NULL, &negotiated_version, &pWlanHandle);

    //Error handling
    if (result != ERROR_SUCCESS) {
        std::cerr << "Failed to open handle to WLAN interface. Error code: " << result << "\n";
        return INVALID_HANDLE_VALUE; // Return an invalid handle to indicate failure
    }

    if (pWlanHandle == NULL || pWlanHandle == INVALID_HANDLE_VALUE)
    {
        std::cerr << "WlanOpenHandle returned an invalid handle.\n";
        return INVALID_HANDLE_VALUE; // Return the error code
    }

    // Handle opened successfully
    std::cout << "Handle to WLAN interface opened successfully. Negotiated version: " << negotiated_version << "\n";



    return pWlanHandle;
}

PWLAN_INTERFACE_INFO_LIST enum_wlan_interfaces(HANDLE pWlanHandle) {

    PWLAN_INTERFACE_INFO_LIST interface_ptr = nullptr;
    DWORD result = WlanEnumInterfaces(pWlanHandle, NULL, &interface_ptr);

    if (result != ERROR_SUCCESS) {
        std::cerr << "Failed to enumerate WLAN interfaces. Error code: " << result << "\n";
        WlanFreeMemory(interface_ptr);// Free memory if not NULL
        exit (1); // Return an invalid handle to indicate failure
    } 
    return interface_ptr;
   
}

void grab_interface_profiles(HANDLE pWlanHandle, PWLAN_INTERFACE_INFO_LIST interface_ptr, GUID &interface_guid) {
    
    PWLAN_PROFILE_INFO_LIST wlan_profiles_ptr = nullptr;
    DWORD result = WlanGetProfileList(pWlanHandle, &interface_guid, NULL, &wlan_profiles_ptr);
    
    if (result != ERROR_SUCCESS) {
        std::cerr << "Failed to retrieve WLAN profiles. Error code: " << result << "\n";
        // Free memory allocated by WlanGetProfileList if wlan_profiles_ptr is not null
        if (wlan_profiles_ptr != nullptr) {
            WlanFreeMemory(wlan_profiles_ptr);
        }
        return;
    }
    WlanFreeMemory(wlan_profiles_ptr);
}

std::wstring utf16_to_wstring(const std::vector<uint16_t>& utf16)
{
    return std::wstring(utf16.begin(), utf16.end());
}
std::wstring parse_utf16_slice(const std::vector<uint16_t>& string_slice)
{
    auto null_index = std::find(string_slice.begin(), string_slice.end(), 0);
    if (null_index != string_slice.end()) {
        return utf16_to_wstring(std::vector<uint16_t>(string_slice.begin(), null_index));
    }
    return L"";
}
std::wstring convert_interface_description(const wchar_t* description) {
    if (description == nullptr) {
        return L"";
    }

    size_t length = wcslen(description);
    std::vector<uint16_t> utf16_string(description, description + length);

    return parse_utf16_slice(utf16_string);
}

int main() {
    HANDLE wlanHandle = open_wlan_handle(WLAN_API_VERSION_2_0);
    if (wlanHandle == INVALID_HANDLE_VALUE) {
        wprintf(L"Failed to open handle to WLAN interface\n");
        WlanCloseHandle(wlanHandle, NULL);
        return 1; // Return error code to indicate failure
    }

    // Enumerate WLAN interfaces
    PWLAN_INTERFACE_INFO_LIST interface_ptr = enum_wlan_interfaces(wlanHandle);
    if (interface_ptr) {
        std::wcout << "List of WLAN interfaces:\n";
        for (DWORD i = 0; i < interface_ptr->dwNumberOfItems; ++i) {
            std::wstring interface_description = convert_interface_description(interface_ptr->InterfaceInfo[i].strInterfaceDescription);
            std::wcout << "Interface #" << i + 1 << " description: " << interface_description << std::endl;
        }
    }
    else {
        std::wcerr << "Failed to retrieve WLAN interface list\n";
    }

    // Close the WLAN handle when done
    WlanCloseHandle(wlanHandle, NULL);
    return 0; // Return success status
}
