#include "../include/PhysicsFactory.h"
#include "../include/PhysicsWorld.h"
#include "../include/RigidBody.h"

namespace physics {
	PhysicsFactory::PhysicsFactory() : iPhysicsFactory() {

	}

	PhysicsFactory::~PhysicsFactory() {

	}

	iPhysicsWorld* PhysicsFactory::CreateWorld()
	{
		PhysicsWorld* world = new PhysicsWorld();

		dynamicsWorld = world->GetDynamicsWorld();
		return world;
	}

	iRigidBody* PhysicsFactory::CreateRigidBody(const RigidBodyDesc& desc, iShape* shape)
	{
		return new RigidBody(desc, shape);
	}

	iSoftBody* PhysicsFactory::CreateSoftBody(const SoftBodyDesc& desc)
	{
		return nullptr;
	}
}