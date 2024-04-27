#define BLUETOOTH_API_INTERNAL
#include "WindowsBluetoothAPI.h"

int InitBluetooth()
{
	connector = new WindowsBluetoothConnector();
	if (connector == nullptr)
	{
		return BT_ERROR;
	}
	
	return BT_SUCCESS;
}

int DisposeBluetooth()
{
	CHECK_INITIALIZATION();

	delete connector;
	return BT_SUCCESS;
}

int GetDevices(BluetoothDevice** devices, int* size)
{
	CHECK_INITIALIZATION();
	try {
		ConnectedDevices = connector->getConnectedDevices();
	}
	catch(std::exception e)
	{
		return BT_ERROR;
	}
	(*devices) = ConnectedDevices.data();
	(*size) = ConnectedDevices.size();
	return BT_SUCCESS;
}

int ConnectTo(int id)
{
	CHECK_INITIALIZATION();
	for(const auto& device : ConnectedDevices)
	{
		if (device.id == id)
		{
			try {
				connector->connect(device.mac);
			}
			catch(std::exception e)
			{
				return BT_ERROR;
			}
			return BT_SUCCESS;
		}
	}
	return BT_DEVICE_NOT_FOUND;
}

int Disconnect()
{
	CHECK_INITIALIZATION();

	return BT_SUCCESS;
}

int Send(char** buffer, int buffer_size)
{
	CHECK_INITIALIZATION();
	CHECK_IS_CONNECTED();
	try {
		connector->send(*buffer, buffer_size);
	}
	catch(std::exception e)
	{
		return BT_ERROR;
	}
	return BT_SUCCESS;
}

int Receive(char** buffer, int buffer_size, int* receivedBytes)
{
	CHECK_INITIALIZATION();
	CHECK_IS_CONNECTED();
	try {
		(*receivedBytes) = connector->recv(*buffer, buffer_size);
	}
	catch (std::exception e)
	{
		return BT_ERROR;
	}
	return BT_SUCCESS;
}
