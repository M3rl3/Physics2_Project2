#pragma once

#include "../interfaces/iPhysicsWorld.h"

#include "RigidBody.h"
#include "CollisionHandler.h"

#include <vector>

namespace physics {

	class CollisionListener : public iCollisionListener {
	public:
		CollisionListener();
		virtual ~CollisionListener();

		virtual void NotifyCollision(iCollisionBody* bodyA, iCollisionBody* bodyB) override;
	};

	class PhysicsWorld : public iPhysicsWorld {
	public:
		PhysicsWorld();
		virtual ~PhysicsWorld();

		virtual void SetGravity(const glm::vec3& gravity) override;

		virtual void AddBody(iCollisionBody* body) override;
		virtual void RemoveBody(iCollisionBody* body) override;

		virtual void TimeStep(float dt) override;

		virtual void RegisterCollisionListener(iCollisionListener* listener) override;

	private:

		glm::vec3 gravity;

		std::vector <iCollisionBody*> collisionBodies;
		std::vector <RigidBody*> rigidBodies;

		CollisionHandler* collisionHandler;
		iCollisionListener* collisionListener;

		PhysicsWorld(const PhysicsWorld&);
	};
 }