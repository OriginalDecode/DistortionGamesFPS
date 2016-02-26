#pragma once
#include "../Game_Shared/SharedNetworkManager.h"
class ClientNetwork;
class ClientNetworkManager : public SharedNetworkManager
{
public:

	void Initiate() override;
	static void Create();
	static void Destroy();
	static ClientNetworkManager* GetInstance();

	void StartNetwork() override;
	void ConnectToServer(const char* aServerIP = "127.0.0.1");

	const CU::GrowingArray<OtherClients>& GetClients();

private:
	ClientNetworkManager();
	~ClientNetworkManager();
	static ClientNetworkManager* myInstance;

	ClientNetwork* myNetwork;
	void HandleMessage(const NetMessageConnectMessage& aMessage, const sockaddr_in& aSenderAddress) override;
	void HandleMessage(const NetMessagePingRequest& aMessage, const sockaddr_in& aSenderAddress) override;
	void HandleMessage(const NetMessageOnJoin& aMessage, const sockaddr_in& aSenderAddress) override;
	void HandleMessage(const NetMessagePosition& aMessage, const sockaddr_in& aSenderAddress) override;

	void ReceieveThread() override;
	void SendThread() override;

	CU::GrowingArray<OtherClients> myClients;

};
