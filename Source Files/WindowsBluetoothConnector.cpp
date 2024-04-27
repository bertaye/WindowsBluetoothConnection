#include "WindowsBluetoothConnector.h"

void WSAStartupWrapper()
{
	int iResult;
	WSADATA wsaData;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		throw std::runtime_error("WSAStartup failed: " + std::to_string(iResult));
	}
}

WindowsBluetoothConnector::WindowsBluetoothConnector()
{
	static bool startedUp = false;
	if (!startedUp)
	{
		::WSAStartupWrapper();
		startedUp = true;
	}
}

void WindowsBluetoothConnector::connect(const std::string& addrStr)
{
	if (this->_socket == INVALID_SOCKET)
	{
		this->_initSocket();
	}

	SOCKADDR_BTH sab = { 0 };
	sab.addressFamily = AF_BTH;
	sab.serviceClassId = RFCOMM_PROTOCOL_UUID;
	sab.btAddr = MACStringToLong(addrStr);

	if (::connect(this->_socket, (sockaddr*)&sab, sizeof(sab)))
	{
		std::cerr << std::to_string(WSAGetLastError());
		throw RecoverableException("Couldn't connect: " + std::to_string(WSAGetLastError()), true);
	}
	this->_connected = true;
}

WindowsBluetoothConnector::~WindowsBluetoothConnector()
{
	if (this->_socket != INVALID_SOCKET)
	{
		::closesocket(this->_socket);
	}
}

int WindowsBluetoothConnector::send(char* buf, size_t length)
{
	auto bytesSent = ::send(this->_socket, buf, length, 0);
	if (bytesSent == SOCKET_ERROR)
	{
		throw std::exception("Send failed with error: %d", WSAGetLastError());
	}
	return bytesSent;
}

int WindowsBluetoothConnector::recv(char* buf, size_t length)
{
	auto bytesReceived = ::recv(this->_socket, buf, length, 0);
	if (bytesReceived == SOCKET_ERROR)
	{
		throw std::exception("Receive failed with error: %d",WSAGetLastError());
	}
	buf[bytesReceived] = '\0';
	return bytesReceived;
}

std::vector<BluetoothDevice> WindowsBluetoothConnector::getConnectedDevices()
{
	std::vector<BluetoothDevice> res;
	std::vector<BluetoothDevice> devsInRadio;

	HANDLE radio = NULL;
	BLUETOOTH_FIND_RADIO_PARAMS radioSearchParams = { sizeof(BLUETOOTH_FIND_RADIO_PARAMS) };
	HBLUETOOTH_RADIO_FIND radioFindHandle = NULL;

	// Search only for connected devices
	BLUETOOTH_DEVICE_SEARCH_PARAMS dev_search_params = { sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS) };
	dev_search_params.fReturnAuthenticated = TRUE;
	dev_search_params.fReturnRemembered = TRUE;
	dev_search_params.fReturnUnknown = TRUE;
	dev_search_params.fReturnConnected = TRUE;
	dev_search_params.fIssueInquiry = TRUE;
	dev_search_params.cTimeoutMultiplier = 2;
	// Iterate for available bluetooth radio devices
	radioFindHandle = BluetoothFindFirstRadio(&radioSearchParams, &radio);
	if (!radioFindHandle)
	{
		if (ERROR_NO_MORE_ITEMS == GetLastError())
		{
			throw RecoverableException(NO_BLUETOOTH_DEVICES_ERROR);
		}
		else
		{
			throw std::runtime_error("BluetoothFindFirstRadio() failed with error code " + std::to_string(GetLastError()));
		}
	}

	do {
		dev_search_params.hRadio = radio;
		devsInRadio = _findDevicesInRadio(&dev_search_params);
		res.insert(res.end(), devsInRadio.begin(), devsInRadio.end());
	} while (BluetoothFindNextRadio(radioFindHandle, &radio));

	// No more radio, close the radio handle
	if (!BluetoothFindRadioClose(radioFindHandle))
		throw std::runtime_error("BluetoothFindRadioClose(bt) failed with error code " + std::to_string(GetLastError()));

	return res;
}

void WindowsBluetoothConnector::disconnect() noexcept
{
	if (this->_socket != INVALID_SOCKET)
	{
		this->_connected = false;
		shutdown(this->_socket, SD_BOTH);
		closesocket(this->_socket);
		this->_socket = INVALID_SOCKET;
	}
}

bool WindowsBluetoothConnector::isConnected() noexcept
{
	return this->_connected;
}

void WindowsBluetoothConnector::_initSocket()
{
	SOCKET sock = ::socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
	if (sock == INVALID_SOCKET)
	{
		throw std::runtime_error("Couldn't create socket: " + std::to_string(WSAGetLastError()));
	}

	ULONG enable = TRUE;
	if (::setsockopt(sock, SOL_RFCOMM, SO_BTH_AUTHENTICATE, reinterpret_cast<char*>(&enable), sizeof(enable)))
	{
		throw std::runtime_error("Couldn't set SO_BTH_AUTHENTICATE: " + std::to_string(WSAGetLastError()));
	}

	if (::setsockopt(sock, SOL_RFCOMM, SO_BTH_ENCRYPT, reinterpret_cast<char*>(&enable), sizeof(enable)))
	{
		throw std::runtime_error("Couldn't set SO_BTH_ENCRYPT: " + std::to_string(WSAGetLastError()));
	}

	this->_socket = sock;
}

std::string WindowsBluetoothConnector::_wstringToUtf8(const std::wstring& wstr)
{
	std::string strTo;
	const int num_chars = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.length(), NULL, 0, NULL, NULL);

	if (num_chars > 0)
	{
		strTo.resize(num_chars);
		WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.length(), &strTo[0], num_chars, NULL, NULL);
	}
	return strTo;
}

std::vector<BluetoothDevice> WindowsBluetoothConnector::_findDevicesInRadio(BLUETOOTH_DEVICE_SEARCH_PARAMS* search_params)
{
	std::vector<BluetoothDevice> res;

	BLUETOOTH_DEVICE_INFO device_info = { sizeof(BLUETOOTH_DEVICE_INFO),0, };

	HBLUETOOTH_DEVICE_FIND dev_find_handle = NULL;

	// For each radio, get the first device
	dev_find_handle = BluetoothFindFirstDevice(search_params, &device_info);

	if (!dev_find_handle)
	{
		if (ERROR_NO_MORE_ITEMS == GetLastError())
		{
			//No devices were found, so we can just return an empty vector
			return res;
		}
		else
		{
			throw std::runtime_error("BluetoothFindFirstDevice() failed with error code: " + std::to_string(GetLastError()));
		}
	}
	int deviceId = 0;
	// Get the device info
	do {
		std::string dvName = _wstringToUtf8(device_info.szName);
		std::string dvMac = MACBytesToString(device_info.Address.rgBytes);
		BluetoothDevice device = {};
		device.id = deviceId++;
		size_t dvNameMemcpySize = dvName.size() > sizeof(device.name) ? sizeof(device.name) : dvName.size();
		memcpy(device.name, dvName.c_str(), dvNameMemcpySize);
		
		size_t dvMacMemcpySize = dvMac.size() > sizeof(device.mac) ? sizeof(device.mac) : dvMac.size();
		memcpy(device.mac, dvMac.c_str(), dvMacMemcpySize);

		res.push_back(std::move(device));
	} while (BluetoothFindNextDevice(dev_find_handle, &device_info));

	// NO more device, close the device handle
	if (!BluetoothFindDeviceClose(dev_find_handle))
		throw std::runtime_error("BluetoothFindDeviceClose(bt_dev) failed with error code: " + std::to_string(GetLastError()));

	return res;
}
