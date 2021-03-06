#pragma once
#include <functional>
#include <Vector.h>
#include <Matrix44.h>

struct InputComponentData;

namespace physx
{
	class PxSimulationEventCallback;
}

class Entity;
class PhysicsComponent;
struct PhysicsComponentData;

namespace physx
{
	class PxRigidDynamic;
	class PxRigidStatic;
	class PxShape;
	class PxActor;
}



namespace Prism
{
	struct PhysicsCallbackStruct;
	class PhysicsManager;


	class PhysicsInterface
	{
	public:
		static void Create(std::function<void(PhysicsComponent*, PhysicsComponent*, bool)> anOnTriggerCallback, bool aIsServer
			, std::function<void(PhysicsComponent*, PhysicsComponent*)> aOnContactCallback);
		static void Destroy();
		static PhysicsInterface* GetInstance();

#ifdef THREAD_PHYSICS
		void InitThread();
		void ShutdownThread();
#endif
		bool GetInitDone() const;
		
		void EndFrame();

		void RayCast(const CU::Vector3<float>& aOrigin, const CU::Vector3<float>& aNormalizedDirection, float aMaxRayDistance
			, std::function<void(PhysicsComponent*, const CU::Vector3<float>&, const CU::Vector3<float>&, const CU::Vector3<float>&)> aFunctionToCall, const PhysicsComponent* aComponent);
		void AddForce(physx::PxRigidDynamic* aDynamicBody, const CU::Vector3<float>& aDirection, float aMagnitude);
		void SetVelocity(physx::PxRigidDynamic* aDynamicBody, const CU::Vector3<float>& aVelocity);
		void TeleportToPosition(physx::PxRigidDynamic* aDynamicBody, const CU::Vector3<float>& aPosition);
		void TeleportToPosition(physx::PxRigidStatic* aStaticBody, const CU::Vector3<float>& aPosition);
		void TeleportToPosition(int aID, const CU::Vector3<float>& aPosition);
		void MoveToPosition(physx::PxRigidDynamic* aDynamicBody, const CU::Vector3<float>& aPosition);

		int CreatePlayerController(const CU::Vector3<float>& aStartPosition, PhysicsComponent* aComponent, bool aShouldAddToScene);
		void Move(int aId, const CU::Vector3<float>& aDirection, float aMinDisplacement, float aDeltaTime);

		void MoveForward(bool move);
		void MoveBackward(bool move);

		void UpdateOrientation(physx::PxRigidDynamic* aDynamicBody, physx::PxShape** aShape, float* aThread4x4);
		void UpdateOrientation(physx::PxRigidStatic* aStaticBody, physx::PxShape** aShape, float* aThread4x4);
		bool GetAllowedToJump(int aId);
		void SetPosition(int aId, const CU::Vector3<float>& aPosition);
		void GetPosition(int aId, CU::Vector3<float>& aPositionOut);

		void SubscribeToTriggers(physx::PxSimulationEventCallback* aSubscriber);

		void Create(PhysicsComponent* aComponent, const PhysicsCallbackStruct& aPhysData
			, float* aOrientation, const std::string& aFBXPath
			, physx::PxRigidDynamic** aDynamicBodyOut, physx::PxRigidStatic** aStaticBodyOut
			, physx::PxShape*** someShapesOut, bool aShouldAddToScene, bool aShouldBeSphere);
		void Add(physx::PxRigidDynamic* aDynamic);
		void Add(physx::PxRigidStatic* aStatic);
		void Add(int aCapsuleID);
		void Remove(physx::PxRigidDynamic* aDynamic, const PhysicsComponentData& aData);
		void Remove(physx::PxRigidStatic* aStatic, const PhysicsComponentData& aData);
		void Remove(int aCapsuleID);
		void Sleep(physx::PxRigidDynamic* aDynamic);
		void Sleep(int aCapsuleID);
		void Wake(physx::PxRigidDynamic* aDynamic);
		void Wake(int aCapsuleID);
		int GetFPS();
		void SetClientSide(bool aIsClientSide = false);
		void SetClientID(int anID);
		void SetPlayerOrientation(CU::Matrix44<float>* anOrientation);
		void SetPlayerInputData(const InputComponentData& aData);
		void SetPlayerGID(int anID);
	private:
		// Requires PhysX includes!!
		PhysicsManager* GetManager() const;
		PhysicsInterface(std::function<void(PhysicsComponent*, PhysicsComponent*, bool)> anOnTriggerCallback, bool aIsServer
			, std::function<void(PhysicsComponent*, PhysicsComponent*)> aOnContactCallback);
		~PhysicsInterface();
		PhysicsManager* myManager;
		static PhysicsInterface* myInstance;
	};
}