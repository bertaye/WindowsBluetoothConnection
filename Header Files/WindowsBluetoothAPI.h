#ifndef WINDOWS_BLUETOOTH_API_H_INCLUDED
#define WINDOWS_BLUETOOTH_API_H_INCLUDED

#ifndef BLUETOOTH_API
#define BLUETOOTH_API __declspec(dllexport)
#endif

#ifdef BLUETOOTH_API_INTERNAL

#include "WindowsBluetoothConnector.h"
WindowsBluetoothConnector *connector = NULL;
std::vector<BluetoothDevice> ConnectedDevices;
#else
typedef struct BluetoothDevice {
	int32_t id;
	char name[512];
	char mac[512];
} BluetoothDevice;
#endif

#define CHECK_INITIALIZATION()\
do{\
 if(connector == NULL){\
	return BT_NOT_INITIALIZED;\
 }\
} while(0);\

#define CHECK_IS_CONNECTED()\
do{\
 if(connector->isConnected() != true){\
	return BT_NOT_CONNECTED;\
 }\
} while(0);\


typedef enum BluetoothResult {
	BT_SUCCESS = 0,
	BT_ERROR = 1,
	BT_NOT_INITIALIZED = 2,
	BT_NOT_CONNECTED = 3,
	BT_DEVICE_NOT_FOUND = 4,
	BT_NOT_SUPPORTED = 5,
	BT_NOT_AUTHORIZED = 6,
	BT_NOT_PAIRED = 7,
	BT_NOT_READY = 8,
	BT_NOT_ENABLED = 9,
}BluetoothResult;


extern "C" {
	BLUETOOTH_API int InitBluetooth();
	BLUETOOTH_API int DisposeBluetooth();
	BLUETOOTH_API int GetDevices(BluetoothDevice** devices, int* size);
	BLUETOOTH_API int ConnectTo(int deviceId);
	BLUETOOTH_API int Disconnect();
	BLUETOOTH_API int Send(char* buffer, int buffer_size);
	BLUETOOTH_API int Receive(char** buffer, int buffer_size, int* receivedBytes);
}

#endif //WINDOWS_BLUETOOTH_API_H_INCLUDED
