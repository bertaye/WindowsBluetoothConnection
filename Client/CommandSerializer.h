#pragma once

#include "Constants.h"
#include "ByteMagic.h"
#include <cstddef>
#include <vector>
#include <stdexcept>

namespace CommandSerializer
{
	using Buffer = std::vector<unsigned char>;

	//escape special chars
	Buffer escapeSpecials(const Buffer& src);
	unsigned char sumChecksum(const Buffer& src);
	//Package a serialized command according to the protocol
	/*
	unk: Seems to be some type of ack value? Based on current understanding (1|0)
    References:
    * DataType
    * CommandBluetoothSender:sendCommandWithRetries
    * BluetoothSenderWrapper.sendCommandViaBluetooth
	*/
	Buffer packageDataForBt(const Buffer& src, DATA_TYPE dataType, unsigned int unk);
	//Probably set unk to zero
	Buffer serializeNcAndAsmSetting(NC_ASM_EFFECT ncAsmEffect, NC_ASM_SETTING_TYPE ncAsmSettingType, unsigned int unk, ASM_SETTING_TYPE asmSettingType, ASM_ID asmId, unsigned char asmLevel);
}
