#include "../include/PhysicsWorld.h"

#include <iostream>

namespace physics {

	PhysicsWorld::PhysicsWorld() : 
		iPhysicsWorld(),
		collisionHandler(nullptr)
	{
		collisionHandler = new CollisionHandler();
		collisionListener = new CollisionListener();
	}

	PhysicsWorld::PhysicsWorld(const PhysicsWorld&) {

	}
	
	PhysicsWorld::~PhysicsWorld() {

		collisionBodies.clear();
		rigidBodies.clear();

		delete collisionHandler;
		delete collisionListener;

		for (int i = 0; i < collisionBodies.size(); i++) {
			delete collisionBodies[i];
		}

		for (int i = 0; i < rigidBodies.size(); i++) {
			delete rigidBodies[i];
		}
	}

	CollisionListener::CollisionListener() {

	}

	CollisionListener::~CollisionListener() {

	}

	void CollisionListener::NotifyCollision(iCollisionBody* bodyA, iCollisionBody* bodyB) {
		//std::cout << "Collision!" << std::endl;
	}

	void PhysicsWorld::SetGravity(const glm::vec3& gravity)
	{
		this->gravity = gravity;
	}

	void PhysicsWorld::AddBody(iCollisionBody* body) {

		if (body == nullptr) {
			return;
		}
		
		if (std::find(collisionBodies.begin(), collisionBodies.end(), body) != collisionBodies.end()) {
			std::cout << "Error: collision body already added." << std::endl;
			return;
		}
		else {
			collisionBodies.push_back(body);
		}

		BodyType currentBodyType = body->GetBodyType();

		if (currentBodyType == BodyType::RigidBody) {

			RigidBody* rigidBody = RigidBody::Cast(body);

			if (std::find(rigidBodies.begin(), rigidBodies.end(), body) != rigidBodies.end()) {
				std::cout << "Error: rigid body already added." << std::endl;
				return;
			}
			else {
				rigidBodies.push_back(rigidBody);
			}
		}
		else if (currentBodyType == BodyType::SoftBody) {

			// Dont have soft bodies atm
			// TODO;
			return;
		}
		else {
			// No idea what type this is
			return;
		}
	}

	void PhysicsWorld::RemoveBody(iCollisionBody* body) {

		std::vector<iCollisionBody*>::iterator itFind =
			std::find(collisionBodies.begin(), collisionBodies.end(), body);

		if (itFind != collisionBodies.end()) {
			collisionBodies.erase(itFind);
		}
		else {
			std::cout << "Error: collision body not found for removal." << std::endl;
			return;
		}
	}

	void PhysicsWorld::RegisterCollisionListener(iCollisionListener* listener) {
		collisionListener = listener;
	}

	void PhysicsWorld::TimeStep(float dt) {

		int bodyCount = collisionBodies.size();
		int rigidBodyCount = rigidBodies.size();

		for (int i = 0; i < rigidBodyCount; i++) {
			rigidBodies[i]->Update(dt);
		}

		// Velocity Verlet steps
		// Step #0 Update everything
		for (int i = 0; i < rigidBodyCount; i++) {
			if (!rigidBodies[i]->IsStatic()) {
				rigidBodies[i]->SetGravityAcceleration(gravity);
				rigidBodies[i]->UpdateAcceleration();
			}
		}

		// Step #3 : Verlet
		// velocity += acceleration * (dt/2)
		for (int i = 0; i < rigidBodyCount; i++) {
			if (!rigidBodies[i]->IsStatic()) {
				rigidBodies[i]->VerletStep3(dt);
				rigidBodies[i]->ApplyDamping(dt / 2.f);
			}
		}

		// Step #1 : Verlet
		// position += ( velocity+acceleration * (dt/2) ) * dt
		for (int i = 0; i < rigidBodyCount; i++) {
			if (!rigidBodies[i]->IsStatic()) {
				rigidBodies[i]->VerletStep1(dt);
			}
		}

		// Collisions
		std::vector<CollidingBodies> collisions;
		collisionHandler->Collide(dt, collisionBodies, collisions);
		//collisionHandler->CollideRigidRigid((collisionBodies.size() - 1), )

		// List of collisions
		for (int i = 0; i < collisions.size(); i++) {
			collisionListener->NotifyCollision(collisions[i].bodyA, collisions[i].bodyB);
		}

		// Step #2 : Verlet
		// velocity += acceleration * (dt/2)
		for (int i = 0; i < rigidBodyCount; i++) {
			rigidBodies[i]->VerletStep2(dt);
			rigidBodies[i]->ApplyDamping(dt / 2.f);
			rigidBodies[i]->KillForces();
		}
	}
}

