#pragma once
#include "Component.h"

class Behavior;
class PhysicsComponent;

class AIComponent : public Component
{
public:
	AIComponent(Entity& anEntity, const AIComponentData& aData, CU::Matrix44<float>& anOrientation);
	~AIComponent();

	void Reset() override;

	void Update(float aDelta) override;

	static eComponentType GetTypeStatic();
	eComponentType GetType() override;

	void HandleRaycast(PhysicsComponent* aComponent, const CU::Vector3<float>& aDirection, const CU::Vector3<float>& aHitPosition, const CU::Vector3<float>& aHitNormal);

private:
	void operator=(AIComponent&) = delete;
	std::function<void(PhysicsComponent*, const CU::Vector3<float>&, const CU::Vector3<float>&, const CU::Vector3<float>&)> myRaycastHandler;

	void Move(float aDelta, Entity* aClosestPlayer);
	void SetOrientation(const CU::Vector3<float>& aLookInDirection, float aDelta);

	void Shoot(Entity* aClosestPlayer);

	const AIComponentData& myData;

	Behavior* myBehavior;

	CU::Matrix44<float>& myOrientation;

	float myShootTimer;

	CU::GrowingArray<Entity*> myBullets;
	int myBulletIndex;
	bool myHasRaycasted;

	float myAttackAnimationTimeCurrent;

	Entity* myTarget;

	bool myHasJustSpawned;
};

inline eComponentType AIComponent::GetTypeStatic()
{
	return eComponentType::AI;
}

inline eComponentType AIComponent::GetType()
{
	return GetTypeStatic();
}