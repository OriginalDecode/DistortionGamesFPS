#pragma once
#include <functional>
#include <Vector.h>
#include <Matrix44.h>

namespace physx
{
	class PxSimulationEventCallback;
}

class Entity;
struct PhysEntityData;

namespace physx
{
	class PxRigidDynamic;
	class PxRigidStatic;
	class PxShape;
	class PxActor;
}

namespace Prism
{
	class PhysicsManager;
	class PhysEntity;

	class PhysicsInterface
	{
	public:
		static void Create();
		static void Destroy();
		static PhysicsInterface* GetInstance();

#ifdef THREAD_PHYSICS
		void InitThread();
		void ShutdownThread();
#endif
		
		void EndFrame();

		void RayCast(const CU::Vector3<float>& aOrigin, const CU::Vector3<float>& aNormalizedDirection, float aMaxRayDistance, std::function<void(Entity*, const CU::Vector3<float>&, const CU::Vector3<float>&)> aFunctionToCall);
		void AddForce(physx::PxRigidDynamic* aDynamicBody, const CU::Vector3<float>& aDirection, float aMagnitude);
		void SetVelocity(physx::PxRigidDynamic* aDynamicBody, const CU::Vector3<float>& aVelocity);
		void SetPosition(physx::PxRigidDynamic* aDynamicBody, const CU::Vector3<float>& aPosition);

		int CreatePlayerController(const CU::Vector3<float>& aStartPosition);
		void Move(int aId, const CU::Vector3<float>& aDirection, float aMinDisplacement, float aDeltaTime);
		void UpdateOrientation(physx::PxRigidDynamic* aDynamicBody, physx::PxShape** aShape, float* aThread4x4);
		bool GetAllowedToJump(int aId);
		void SetPosition(int aId, const CU::Vector3<float>& aPosition);
		void GetPosition(int aId, CU::Vector3<float>& aPositionOut);

		void SubscribeToTriggers(physx::PxSimulationEventCallback* aSubscriber);

		void Create(PhysEntity* aEntity, const PhysEntityData& aPhysData
			, float* aOrientation, const std::string& aFBXPath
			, physx::PxRigidDynamic** aDynamicBodyOut, physx::PxRigidStatic** aStaticBodyOut
			, physx::PxShape*** someShapesOut);
		void Remove(physx::PxRigidDynamic* aDynamic);
		void Remove(physx::PxRigidStatic* aStatic);

	private:
		// Requires PhysX includes!!
		PhysicsManager* GetManager() const;

		PhysicsInterface();
		~PhysicsInterface();
		PhysicsManager* myManager;
		static PhysicsInterface* myInstance;
	};
}