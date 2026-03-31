/*
This solution returns to the user some of the properties(metadata) about devices of an requested/all type
It lets the user use the cmd for introducing an type of devices to show details of.
For each approach the details of devices will be written into a file "devices_output.txt" separated with ======
When the user wants specific type of devices(connected to a specific bus), it will input as cmd line arg the requested type
If the type doesn't exist or it exists, but there are no devices connected to that type of bus than go with approach 2
If exists and has at least one connected device than go with approach 1
If the suer hits enter go with approach 2

Approach 1:
    -after existance check of the type with SetupDiGetClassDevsW, and checked to be at least one deivce connected,
    basically, iterate over all devices with SetupDiEnumDeviceInfo, and store the device data so it can be proccesed by 
    the function ProcessAndWriteDevice that will take the data strcuture and make another structre containing the properties
    of the device, that structre is made via calling SetupDiGetDeviceRegistryPropertyW on each porperty to extract it,
    after strcuutre is obtained, write it to the output file "devices_output.txt",
    finally print in the cmd the number of devices recorded

Approach 2:
    -same as approach 1, but here we will take the devices from all types of bus, than print in the cmd the number of devices recorded
*/


#include <windows.h>
#include <setupapi.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#pragma comment(lib, "setupapi.lib")

struct DeviceMetaData {
    std::wstring hardwareID;
    std::wstring description;
    std::wstring manufacturer;
    std::wstring friendlyName;
    std::wstring deviceClass;
};


std::wstring GetDevicePropertyString(HDEVINFO hDevInfo, SP_DEVINFO_DATA& devInfoData, DWORD propId) {
    //get an certain property and return it so it can be saved into the device data structure to be placed in the output file

    DWORD dataType;
    DWORD reqSize = 0;
    
    //check for proprerty existance
    SetupDiGetDeviceRegistryPropertyW(hDevInfo, &devInfoData, propId, &dataType, NULL, 0, &reqSize);

    if (reqSize == 0) {
        return L"";
    }

    std::vector<BYTE> buffer(reqSize);
    //store the proprerty into the buffer and return it
    if (SetupDiGetDeviceRegistryPropertyW(hDevInfo, &devInfoData, propId, &dataType, buffer.data(), reqSize, NULL)) {
        return std::wstring(reinterpret_cast<wchar_t*>(buffer.data()));
    }

    return L"";
}

DeviceMetaData CreateDeviceStruct(HDEVINFO hDevInfo, SP_DEVINFO_DATA& devInfoData) {
    //create the data strcuture of the device with several proprerties(metaparamepoeters)

    DeviceMetaData data;
    data.hardwareID = GetDevicePropertyString(hDevInfo, devInfoData, SPDRP_HARDWAREID);
    data.description = GetDevicePropertyString(hDevInfo, devInfoData, SPDRP_DEVICEDESC);
    data.manufacturer = GetDevicePropertyString(hDevInfo, devInfoData, SPDRP_MFG);
    data.friendlyName = GetDevicePropertyString(hDevInfo, devInfoData, SPDRP_FRIENDLYNAME);
    data.deviceClass = GetDevicePropertyString(hDevInfo, devInfoData, SPDRP_CLASS);
    return data;
}

void ProcessAndWriteDevice(HDEVINFO hDevInfo, SP_DEVINFO_DATA& devInfoData, std::wofstream& file) {
    //take the strcuture returned by SetupDiEnumDeviceInfo and process it to have the data strcuture, than write it to the output file

    DeviceMetaData data = CreateDeviceStruct(hDevInfo, devInfoData);

    file << L"======\n";
    file << L"Hardware ID: " << (data.hardwareID.empty() ? L"N/A" : data.hardwareID) << L"\n";
    file << L"Description: " << (data.description.empty() ? L"N/A" : data.description) << L"\n";
    file << L"Manufacturer: " << (data.manufacturer.empty() ? L"N/A" : data.manufacturer) << L"\n";
    file << L"Friendly Name: " << (data.friendlyName.empty() ? L"N/A" : data.friendlyName) << L"\n";
    file << L"Class: " << (data.deviceClass.empty() ? L"N/A" : data.deviceClass) << L"\n";
    file << L"======\n";
}

void HandleDeviceScanning(std::wstring userInput) {
    //the fucntion to execute the approaches and print to the cmd the result

    int approach = 0;
    std::wstring busType = userInput;

    //check for enter
    if (busType.empty()) {
        approach = 3;
    }
    else {
        //tranfoerm to uppercoase for case matchign
        std::transform(busType.begin(), busType.end(), busType.begin(), ::towupper);

        HDEVINFO testHandle = SetupDiGetClassDevsW(NULL, busType.c_str(), NULL, DIGCF_ALLCLASSES | DIGCF_PRESENT);
        //invalid type given
        if (testHandle != INVALID_HANDLE_VALUE) {
            SP_DEVINFO_DATA testData;
            testData.cbSize = sizeof(SP_DEVINFO_DATA);

            //valid type with connected devices
            if (SetupDiEnumDeviceInfo(testHandle, 0, &testData)) {
                approach = 1;
            }
            else {
                //valid type with no connected devices
                approach = 2;
            }
            SetupDiDestroyDeviceInfoList(testHandle);
        }
        else {
            approach = 2;
        }
    }

    HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;

    //handle approach 1->make a list with certain devices type or approch 2->make a list with all types of devices
    if (approach == 1) {
        hDevInfo = SetupDiGetClassDevsW(NULL, busType.c_str(), NULL, DIGCF_ALLCLASSES | DIGCF_PRESENT);
    }
    else {
        hDevInfo = SetupDiGetClassDevsW(NULL, NULL, NULL, DIGCF_ALLCLASSES | DIGCF_PRESENT);
    }

    if (hDevInfo == INVALID_HANDLE_VALUE) {
        return;
    }

    std::wofstream outFile(L"devices_output.txt");
    SP_DEVINFO_DATA devInfoData;
    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    DWORD count = 0;

    //iterate thorugh the the lisgt of devices and process their data strcuure
    while (SetupDiEnumDeviceInfo(hDevInfo, count, &devInfoData)) {
        ProcessAndWriteDevice(hDevInfo, devInfoData, outFile);
        count++;
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);
    outFile.close();

    if (approach == 1) {
        std::wcout << L"Devices of TYPE: " << busType << L" were read.Number of devices found " << count << L".\n";
    }
    else if (approach == 2) {
        std::wcout << L"Devices of TYPE: " << busType << L" not found.Scanning all types of devices.Number of devices found " << count << L".\n";
    }
    else if (approach == 3) {
        std::wcout << L"Scanning all types of devices.Number of devices found " << count << L".\n";
    }
}


int main() {
    std::wstring input;
    std::getline(std::wcin, input);
    HandleDeviceScanning(input);
    return 0;
}