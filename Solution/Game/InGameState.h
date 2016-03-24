#pragma once
#include "GameState.h"
#include <Matrix.h>
#include <GrowingArray.h>
#include <Subscriber.h>
#include <NetworkSubscriber.h>

class ClientLevel;
class ClientLevelFactory;
class MessageState;


namespace CU
{
	class InputWrapper;
}

namespace Prism
{
	class Sprite;
	class TextProxy;
}

namespace GUI
{
	class GUIManager;
	class Cursor;
}

class InGameState : public GameState, public Subscriber, public NetworkSubscriber
{
public:
	InGameState(int aLevelID);
	~InGameState();

	void InitState(StateStackProxy* aStateStackProxy, GUI::Cursor* aCursor) override;
	void EndState() override;

	const eStateStatus Update(const float& aDeltaTime) override;
	void Render() override;
	void ResumeState() override;

	void ReceiveMessage(const GameStateMessage& aMessage) override;

	void ReceiveNetworkMessage(const NetMessageAllClientsComplete& aMessage, const sockaddr_in& aSenderAddress) override;
	void ReceiveNetworkMessage(const NetMessageLevelComplete& aMessage, const sockaddr_in& aSenderAddress) override;
	void ReceiveNetworkMessage(const NetMessageLoadLevel& aMessage, const sockaddr_in& aSenderAddress) override;

	void OnResize(int aWidth, int aHeight) override;

private:
	GUI::GUIManager* myGUIManager;
	ClientLevel* myLevel;
	ClientLevelFactory* myLevelFactory;

	int myLevelToLoad;
	bool myShouldLoadLevel;

	bool myLevelComplete;
	bool myCanStartNextLevel;

	Prism::TextProxy* myText;
};