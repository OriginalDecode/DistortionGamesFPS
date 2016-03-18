#include "stdafx.h"
#include "ServerNetworkManager.h"
#include "ServerNetwork.h"
#include <thread>
#include <Utility.h>
#include <PostMaster.h>

#include <NetMessageImportantReply.h>
#include <NetMessageConnectReply.h>
#include <NetMessageRequestConnect.h>
#include <NetMessageOnJoin.h>
#include <NetMessageDisconnect.h>
#include <NetMessageRequestLevel.h>
#include <NetMessageRequestStartGame.h>
#include <NetMessagePingRequest.h>
#include <NetMessagePingReply.h>
#include <NetMessagePosition.h>
#include <NetMessageOnHit.h>
#include <NetMessageOnDeath.h>
#include <NetMessageLevelLoaded.h>

#define BUFFERSIZE 512
#define RECONNECT_ATTEMPTS 10000

ServerNetworkManager::ServerNetworkManager()
	: myAllowNewConnections(false)
{
	PostMaster::GetInstance()->Subscribe(eMessageType::NETWORK_ADD_ENEMY, this);
	PostMaster::GetInstance()->Subscribe(eMessageType::NETWORK_SEND_POSITION, this);
	PostMaster::GetInstance()->Subscribe(eMessageType::NETWORK_ON_DEATH, this);
}

ServerNetworkManager::~ServerNetworkManager()
{
	PostMaster::GetInstance()->UnSubscribe(eMessageType::NETWORK_ADD_ENEMY, this);
	PostMaster::GetInstance()->UnSubscribe(eMessageType::NETWORK_SEND_POSITION, this);
	PostMaster::GetInstance()->UnSubscribe(eMessageType::NETWORK_ON_DEATH, this);

	UnSubscribe(eNetMessageType::POSITION, this);
	UnSubscribe(eNetMessageType::PING_REPLY, this);
	UnSubscribe(eNetMessageType::PING_REQUEST, this);
	UnSubscribe(eNetMessageType::ON_CONNECT, this);
	UnSubscribe(eNetMessageType::ON_DISCONNECT, this);

	myMainIsDone = true;
	myReceieveIsDone = true;
	myIsRunning = false;
	if (myReceieveThread != nullptr)
	{
		myReceieveThread->join();
		delete myReceieveThread;
		myReceieveThread = nullptr;
	}

	if (mySendThread != nullptr)
	{
		mySendThread->join();
		delete mySendThread;
		mySendThread = nullptr;
	}
	delete myNetwork;
	myNetwork = nullptr;
}

void ServerNetworkManager::Initiate()
{
	myClients.Init(16);
	myIsServer = true;
	myNetwork = new ServerNetwork();
	myGID = 0;
	myIDCount = 0;
	__super::Initiate();
	Subscribe(eNetMessageType::POSITION, this);
	Subscribe(eNetMessageType::PING_REPLY, this);
	Subscribe(eNetMessageType::PING_REQUEST, this);
	Subscribe(eNetMessageType::ON_CONNECT, this);
	Subscribe(eNetMessageType::ON_DISCONNECT, this);
}

void ServerNetworkManager::Create()
{
	if (myInstance == nullptr)
	{
		myInstance = new ServerNetworkManager();
		myInstance->Initiate();
	}
}

void ServerNetworkManager::Destroy()
{
	delete myInstance;
	myInstance = nullptr;
}

ServerNetworkManager* ServerNetworkManager::GetInstance()
{
	if (myInstance != nullptr)
	{
		return static_cast<ServerNetworkManager*>(myInstance);
	}
	DL_ASSERT("Instance were null, did you forget to create the ServerNetworkManager?");
	return nullptr;
}

void ServerNetworkManager::StartNetwork(unsigned int aPortNum)
{
	myNetwork->StartServer(aPortNum);
	myNetwork->PrintStatus();
	__super::StartNetwork(aPortNum);
	myIsOnline = true;
}

bool ServerNetworkManager::ListContainsAllClients(const CU::GrowingArray<unsigned int>& someClientIDs) const
{
	if (myClients.Size() != someClientIDs.Size())
	{
		return false;
	}

	for each (const Connection& connection in myClients)
	{
		if (someClientIDs.Find(connection.myID) == someClientIDs.FoundNone)
		{
			return false;
		}
	}

	return true;
}

void ServerNetworkManager::ReceieveThread()
{
	char buffer[BUFFERSIZE];
	std::vector<Buffer> someBuffers;
	while (myIsRunning == true)
	{
		ZeroMemory(&buffer, BUFFERSIZE);
		myNetwork->Receieve(someBuffers);

		if (someBuffers.size() == 0)
		{
			WSAGetLastError();
		}
		for (Buffer message : someBuffers)
		{
			myReceieveBuffer[myCurrentBuffer ^ 1].Add(message);
		}
		ReceieveIsDone();
		WaitForMain();
		someBuffers.clear();
		Sleep(1);

	}
}

void ServerNetworkManager::SendThread()
{
	while (myIsRunning == true)
	{
		for (SendBufferMessage arr : mySendBuffer[myCurrentSendBuffer])
		{
			if (arr.myTargetID == 0)
			{
				for (Connection& connection : myClients)
				{
					if (connection.myID != static_cast<unsigned int>(arr.myBuffer[5]))
					{
						myNetwork->Send(arr.myBuffer, connection.myAddress);
					}
				}
			}
			else 
			{
				for (Connection& connection : myClients)
				{
					if (connection.myID == arr.myTargetID && connection.myID != static_cast<unsigned int>(arr.myBuffer[5]))
					{
						myNetwork->Send(arr.myBuffer, connection.myAddress);
						break;
					}
				}
			}
		}
		mySendBuffer[myCurrentSendBuffer].RemoveAll();
		myCurrentSendBuffer ^= 1;
		Sleep(1);
	}
}

void ServerNetworkManager::CreateConnection(const std::string& aName, const sockaddr_in& aSender)
{
	myIDCount++;
	NetMessageConnectReply connectReply(NetMessageConnectReply::eType::SUCCESS, myIDCount);
	connectReply.PackMessage();
	myNetwork->Send(connectReply.myStream, aSender);

	Sleep(200);
	//for (Connection& connection : myClients)
	//{
	//	if (connection.myAddress.sin_addr.S_un.S_addr == aSender.sin_addr.S_un.S_addr) //._.
	//	{
	//		Utility::PrintEndl("User already connected!", (DARK_RED_BACK | WHITE_TEXT));
	//		return;
	//	}
	//}

	for (Connection& connection : myClients)
	{
		NetMessageOnJoin msg(connection.myName, connection.myID);
		msg.PackMessage();
		myNetwork->Send(msg.myStream, aSender);
	}

	Connection newConnection;
	newConnection.myAddress = aSender;
	newConnection.myID = myIDCount;
	newConnection.myName = aName;
	newConnection.myPingCount = 0;
	newConnection.myIsConnected = true;
	myClients.Add(newConnection);
	myNames[aName] = 1;

	std::string conn(aName + " connected to the server!");
	Utility::PrintEndl(conn, LIGHT_GREEN_TEXT);

	AddMessage(NetMessageOnJoin(aName, myIDCount));
}

void ServerNetworkManager::DisconnectConnection(const Connection& aConnection)
{
	NetMessageDisconnect onDisconnect = NetMessageDisconnect(aConnection.myID);
	onDisconnect.PackMessage();
	myNetwork->Send(onDisconnect.myStream, aConnection.myAddress);
	
	std::string msg(aConnection.myName + " disconnected from server!");
	Utility::PrintEndl(msg, LIGHT_BLUE_TEXT);

	//auto reply on all important messages
	for (ImportantMessage& impMsg : myImportantMessagesBuffer)
	{
		for (ImportantClient& client : impMsg.mySenders)
		{
			if (client.myGID == aConnection.myID)
			{
				client.myHasReplied = true;
				break;
			}
		}
	}

	for (int i = 0; i < myClients.Size(); ++i)
	{
		if (aConnection.myID == myClients[i].myID)
		{
			myClients.RemoveCyclicAtIndex(i);
			break;
		}
	}
	//Send to all other clients which client has been disconnected
	AddMessage(NetMessageDisconnect(aConnection.myID));
}

void ServerNetworkManager::UpdateImportantMessages(float aDeltaTime)
{
	for (ImportantMessage& msg : myImportantMessagesBuffer)
	{
		bool finished = true;
		for (ImportantClient& client : msg.mySenders)
		{
			if (client.myHasReplied == false)
			{
				finished = false;
				client.myTimer += aDeltaTime;
				if (client.myTimer >= 1.f)
				{
					client.myTimer = 0.f;
					
					std::string resend = "Sending important message " + std::to_string(msg.myImportantID) + " of message type id " 
						+ std::to_string(msg.myMessageType) + " to client id " + std::to_string(client.myGID) + " - " + client.myName;
					Utility::PrintEndl(resend, AQUA_TEXT);
					myNetwork->Send(msg.myData, client.myNetworkAddress);
				}
			}
		}
		if (finished == true)
		{
			std::string resend = "All client has replied to the message id " + std::to_string(msg.myImportantID) 
				+ " of message type id " + std::to_string(msg.myMessageType);
			Utility::PrintEndl(resend, YELLOW_TEXT);
			myImportantMessagesBuffer.RemoveCyclic(msg);
		}
	}
}

void ServerNetworkManager::AddImportantMessage(std::vector<char> aBuffer, unsigned int aImportantID)
{
	if (myClients.Size() > 0)
	{
		ImportantMessage msg;
		msg.myData = aBuffer;
		msg.myImportantID = aImportantID;
		msg.myMessageType = aBuffer[0];
		msg.mySenders.Init(myClients.Size());
		for (Connection c : myClients)
		{
			ImportantClient client;
			client.myGID = c.myID;
			client.myNetworkAddress = c.myAddress;
			client.myName = c.myName;
			client.myTimer = 0.f;
			client.myHasReplied = false;
			msg.mySenders.Add(client);
		}
		myImportantMessagesBuffer.Add(msg);
	}
}

void ServerNetworkManager::ReceiveNetworkMessage(const NetMessageRequestConnect& aMessage, const sockaddr_in& aSenderAddress)
{
	if (CheckIfImportantMessage(aMessage) == true)
	{
		NetMessageImportantReply toReply(aMessage.GetImportantID());
		toReply.PackMessage();
		
		myNetwork->Send(toReply.myStream, aSenderAddress);
	}
	if (AlreadyReceived(aMessage) == false)
	{
		if (myAllowNewConnections == false)
		{
			NetMessageConnectReply connectReply(NetMessageConnectReply::eType::FAIL);
			connectReply.PackMessage();
			myNetwork->Send(connectReply.myStream, aSenderAddress);
			//Bounce
		}
	}
}

void ServerNetworkManager::ReceiveNetworkMessage(const NetMessageDisconnect& aMessage, const sockaddr_in&)
{
	for (Connection c : myClients)
	{
		if (c.myID == aMessage.myClientID)
		{
			DisconnectConnection(c);
			break;
		}
	}
}

void ServerNetworkManager::ReceiveNetworkMessage(const NetMessagePingReply& aMessage, const sockaddr_in&)
{
	for (Connection& c : myClients)
	{
		if (c.myID == aMessage.mySenderID)
		{
			c.myPingCount = 0;
			break;
		}
	}
}

void ServerNetworkManager::ReceiveNetworkMessage(const NetMessagePosition& aMessage, const sockaddr_in&)
{
	AddMessage(aMessage);
}

void ServerNetworkManager::ReceiveNetworkMessage(const NetMessagePingRequest& aMessage, const sockaddr_in&)
{
	AddMessage(NetMessagePingReply(), aMessage.mySenderID);
	for (Connection& c : myClients)
	{
		c.myPingCount++;
		if (c.myPingCount > RECONNECT_ATTEMPTS)
		{
			DisconnectConnection(c);
		}
	}
}