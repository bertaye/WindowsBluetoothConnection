This project is forked from: https://github.com/Plutoberth/SonyHeadphonesClient for the purpose of creating a Bluetooth Plugin for Unity Windows & Editor (in Windows).
I appreciate the work of the base repo since they saved a lot of time for me (and hopefully for many others).

## Changes
I edited the BLE device detection part slightly, and converted the project into a lightweight DLL project so that other developers can add the BLE feauture their projects easily.
One can use CMake to create the dll directly.
```
mkdir build
cd build
cmake ../
cmake --build .
```
that's all, you can use WindowsBluetoothAPI.h header before importing the DLL. Here is a simple test code;
```cpp
#include <Windows.h>
#include <iostream>
#include <vector>

#include "WindowsBluetoothAPI.h"

// Function pointer declarations using decltype
decltype(&InitBluetooth) ptrInitBluetooth;
decltype(&DisposeBluetooth) ptrDisposeBluetooth;
decltype(&GetDevices) ptrGetDevices;
decltype(&ConnectTo) ptrConnectTo;
decltype(&Disconnect) ptrDisconnect;
decltype(&Send) ptrSend;
decltype(&Receive) ptrReceive;

bool loadBluetoothFunctions() {
    HMODULE hModule = LoadLibrary(TEXT("___PATH_TO_YOUR_DLL___"));
    if (!hModule) {
        std::cerr << "Failed to load the DLL." << std::endl;
        return false;
    }

    // Use decltype to ensure the correct function types
    ptrInitBluetooth = reinterpret_cast<decltype(ptrInitBluetooth)>(GetProcAddress(hModule, "InitBluetooth"));
    ptrDisposeBluetooth = reinterpret_cast<decltype(ptrDisposeBluetooth)>(GetProcAddress(hModule, "DisposeBluetooth"));
    ptrGetDevices = reinterpret_cast<decltype(ptrGetDevices)>(GetProcAddress(hModule, "GetDevices"));
    ptrConnectTo = reinterpret_cast<decltype(ptrConnectTo)>(GetProcAddress(hModule, "ConnectTo"));
    ptrDisconnect = reinterpret_cast<decltype(ptrDisconnect)>(GetProcAddress(hModule, "Disconnect"));
    ptrSend = reinterpret_cast<decltype(ptrSend)>(GetProcAddress(hModule, "Send"));
    ptrReceive = reinterpret_cast<decltype(ptrReceive)>(GetProcAddress(hModule, "Receive"));

    // Check if all functions were loaded successfully
    if (!ptrInitBluetooth || !ptrDisposeBluetooth || !ptrGetDevices || !ptrConnectTo ||
        !ptrDisconnect || !ptrSend || !ptrReceive) {
        std::cerr << "Failed to load one or more functions." << std::endl;
        FreeLibrary(hModule);
        return false;
    }

    return true;
}

int main() {
    if (!loadBluetoothFunctions()) {
        std::cerr << "Bluetooth functions could not be loaded." << std::endl;
        return 1;
    }


    // Example usage of the functions
    if (ptrInitBluetooth() == BT_SUCCESS) {
        std::cout << "Bluetooth initialized successfully." << std::endl;
    }
    else {
        std::cerr << "Failed to initialize Bluetooth." << std::endl;
        return 1;
    }

    BluetoothDevice* devices = NULL;
    int size = 0;
    ptrGetDevices(&devices, &size);

    for (int i = 0; i < size; i++) {
        std::cout << "Device ID: " << devices[i].id << std::endl;
        std::cout << "Device Name: " << devices[i].name << std::endl;
    }
    int deviceId;
    std::cout<<"Enter the device ID to connect: ";
    std::cin>>deviceId;

    if (ptrConnectTo(deviceId) == BT_SUCCESS) {
		std::cout << "Connected to device with ID: " << deviceId << std::endl;
	}
    else {
		std::cerr << "Failed to connect to device with ID: " << deviceId << std::endl;
		return 1;
	}

    char* buffer = new char[1024];
	int buffer_size = 1024;
	std::cout<<"enter message to send: ";
	std::cin>>buffer;
    if (ptrSend(&buffer, buffer_size) == BT_SUCCESS) {
		std::cout << "message sent successfully." << std::endl;
	}
    else {
		std::cerr << "failed to send message." << std::endl;
		return 1;
	}
    
    while (true) {
		int receivedBytes = 0;
		char* receivedBuffer = new char[1024];
        if (ptrReceive(&receivedBuffer, 1024, &receivedBytes) == BT_SUCCESS) {
			std::cout << "Received message: " << receivedBuffer << std::endl;
		}
        else {
			std::cerr << "Failed to receive message." << std::endl;
			return 1;
		}
    }
    // Clean up and close
    if (ptrDisposeBluetooth() == BT_SUCCESS) {
        std::cout << "Bluetooth disposed successfully." << std::endl;
    }
    else {
        std::cerr << "Failed to dispose Bluetooth." << std::endl;
        return 1;
    }

    return 0;
}

```
