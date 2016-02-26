#pragma once
#include "NetMessage.h"
#include <GrowingArray.h>

class NetMessageConnectMessage : public NetMessage
{
public:
	NetMessageConnectMessage(const std::string& aName, short aServerID);
	NetMessageConnectMessage();

	NetMessageConnectMessage(sockaddr_in anAddress);
	~NetMessageConnectMessage();

	//void Init(const std::string& aName, short aServerID);

	std::string myName;
	short myServerID;
	CU::GrowingArray<unsigned short> myClientsOnServer;
protected:

	void DoSerialize(StreamType& aStream) override;
	void DoDeSerialize(StreamType& aStream) override;

};

