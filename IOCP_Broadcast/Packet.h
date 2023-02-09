#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Ŭ���̾�Ʈ�� ���� ��Ŷ�� �����ϴ� ����ü
struct PacketData
{
	UINT32 ClientIndex = 0;
	UINT32 DataSize = 0;
	char* pPacketData = nullptr;

	void Set(PacketData& vlaue)
	{
		ClientIndex = vlaue.ClientIndex;
		DataSize = vlaue.DataSize;

		pPacketData = new char[vlaue.DataSize];
		CopyMemory(pPacketData, vlaue.pPacketData, vlaue.DataSize);
	}

	void Set(UINT32 clientIndex, UINT32 dataSize, char* pData)
	{
		ClientIndex = clientIndex;
		DataSize = dataSize;

		pPacketData = new char[dataSize];
		CopyMemory(pPacketData, pData, DataSize);
	}

	void Release()
	{
		delete pPacketData;
	}
};