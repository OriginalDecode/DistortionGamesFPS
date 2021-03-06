#include "stdafx.h"
#include "ClientNetwork.h"
#include "NetMessageRequestConnect.h"
#include <DL_Debug.h>
#define BUFFERSIZE 512

ClientNetwork::ClientNetwork()
{
	Reset();
}

void ClientNetwork::Reset()
{
	myPingCount = 0;
	ZeroMemory(&myServerAddress, sizeof(myServerAddress));
	ZeroMemory(&myLocalServerAddress, sizeof(myLocalServerAddress));
	
	myName = "resetted";
	myIP = "resetted";
}


ClientNetwork::~ClientNetwork()
{
	closesocket(mySocket);
	WSACleanup();
}

void ClientNetwork::StartNetwork(int aPortNum)
{
	myPort = static_cast<uint16_t>(aPortNum);
	if (WSAStartup(MAKEWORD(2, 2), &myWSAData) != 0)
	{
		DL_ASSERT("WSAStartup Failed [Client]");
	}

	ZeroMemory(&myLocalServerAddress, sizeof(myLocalServerAddress));
	myLocalServerAddress.sin_family = AF_INET;
	myLocalServerAddress.sin_port = htons(myPort);
	myLocalServerAddress.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	//mySocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if ((mySocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
	{
		DL_ASSERT("Failed to set socket!");
	}

	DWORD nonBlocking = 1;
	if (ioctlsocket(mySocket, FIONBIO, &nonBlocking) != 0)
	{
		DL_ASSERT("Failed to set non-blocking socket.");
	}
}

void ClientNetwork::Send(const std::vector<char>& anArray)
{
	if (sendto(mySocket, &anArray[0], anArray.size(), 0, (struct sockaddr *) &myServerAddress, sizeof(myServerAddress)) == SOCKET_ERROR)
	{
		DL_ASSERT("Failed to send");
	}
}

void ClientNetwork::Send(const std::vector<char>& anArray, const sockaddr_in& aTargetAddress)
{
	if (sendto(mySocket, &anArray[0], anArray.size(), 0, (struct sockaddr *) &aTargetAddress, sizeof(aTargetAddress)) == SOCKET_ERROR)
	{
		DL_ASSERT("Failed to send");
	}
}

void ClientNetwork::Receieve(std::vector<Buffer>& someBuffers)
{
	int toReturn;
	char buffer[BUFFERSIZE];
	ZeroMemory(&buffer, BUFFERSIZE);
	Buffer toPushback;
	while ((toReturn = recv(mySocket, buffer, BUFFERSIZE, 0)) > 0)
	{
		memcpy(&toPushback.myData, &buffer[0], toReturn*sizeof(char));
		toPushback.myLength = toReturn;
		someBuffers.push_back(toPushback);
	}
}

bool ClientNetwork::ConnectToServer(const char* anIP)
{
	Reset();
	myIP = anIP;

	ZeroMemory(&myServerAddress, sizeof(myServerAddress));
	myServerAddress.sin_family = AF_INET;
	myServerAddress.sin_port = htons(myPort);
	myServerAddress.sin_addr.S_un.S_addr = inet_addr(myIP.c_str());

	ZeroMemory(&myLocalServerAddress, sizeof(myLocalServerAddress));
	myLocalServerAddress.sin_family = AF_INET;
	myLocalServerAddress.sin_port = htons(myPort);
	myLocalServerAddress.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	
	return true;
}

const CU::GrowingArray<OtherClients>& ClientNetwork::GetClientList()
{
	return myClients;
}
