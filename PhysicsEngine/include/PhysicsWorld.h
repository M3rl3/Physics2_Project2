#pragma once

#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/btBulletCollisionCommon.h>

#include "iPhysicsWorld.h"
#include "RigidBody.h"

#include <vector>

namespace physics {

	class PhysicsWorld : public iPhysicsWorld {
	public:
		PhysicsWorld();
		virtual ~PhysicsWorld();

		virtual void SetGravity(const glm::vec3& gravity) override;

		virtual void AddBody(iCollisionBody* body) override;
		virtual void RemoveBody(iCollisionBody* body) override;

		virtual void TimeStep(float dt) override;

		virtual void RegisterCollisionListener(iCollisionListener* listener) override;

		btDiscreteDynamicsWorld* GetDynamicsWorld(void);

	private:

		btDefaultCollisionConfiguration* collisionConfiguration;
		btCollisionDispatcher* dispatcher;
		btBroadphaseInterface* broadphase;
		btSequentialImpulseConstraintSolver* solver;
		btDiscreteDynamicsWorld* dynamicsWorld;

		PhysicsWorld(const PhysicsWorld&);
		PhysicsWorld& operator=(const PhysicsWorld&) {
			return *this;
		}
	};
 }