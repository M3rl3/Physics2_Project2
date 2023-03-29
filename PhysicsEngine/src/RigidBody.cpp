#include "../include/RigidBody.h"

namespace physics {

	RigidBody::RigidBody() {
	}

	RigidBody::RigidBody(const RigidBody&) {
	}

	RigidBody::RigidBody(const RigidBodyDesc& desc, iShape* shape) : 
		iRigidBody(),
		shape(shape),
		position(desc.position),
		rotation(desc.rotation),
		isStatic(desc.isStatic),
		linearVelocity(desc.linearVelocity),
		angularVelocity(desc.angularVelocity),
		restitution(desc.restitution),
		friction(desc.friction),
		linearDamping(desc.linearDamping),
		angularDamping(desc.angularDamping)
	{
		if (isStatic || desc.mass <= 0.f) {
			mass = 0.f;
			invMass = 0.f;
			isStatic = true;
		}
		else
		{
			mass = desc.mass;
			invMass = 1.f / mass;
		}
	}

	RigidBody::~RigidBody() {
	}

	RigidBody* RigidBody::Cast(iCollisionBody* body) {
		return dynamic_cast<RigidBody*>(body);
	}

	bool RigidBody::IsStatic() {
		return isStatic;
	}

	void RigidBody::SetRenderPosition(glm::vec3* position) {
		this->renderPosition = position;
	}

	// Subject to change
	void RigidBody::Update(float v) {
		if (renderPosition != nullptr) {
			renderPosition->x += v;
			renderPosition->y += v;
			renderPosition->z += v;
		}
	}

	iShape* RigidBody::GetShape() {
		return this->shape;
	}

	void RigidBody::GetPosition(glm::vec3& position) {
		position = this->position;
	}

	void RigidBody::SetPosition(const glm::vec3& position) {
		this->position = position;
	}

	void RigidBody::GetRotation(glm::quat& rotation) {
		rotation = this->rotation;
	}

	void RigidBody::SetRotation(const glm::quat& rotation) {
		this->rotation = rotation;
	}

	void RigidBody::ApplyForce(const glm::vec3& force) {
		this->force += force;
	}

	void RigidBody::ApplyForceAtPoint(const glm::vec3& force, const glm::vec3& relativePoint) {
		ApplyForce(force);
		ApplyTorque(glm::cross(relativePoint, force));
	}

	void RigidBody::ApplyImpulse(const glm::vec3& impulse) {
		this->linearVelocity += impulse * this->invMass;
	}

	void RigidBody::ApplyImpulseAtPoint(const glm::vec3& impulse, const glm::vec3& relativePoint) {
		ApplyTorqueImpulse(glm::cross(relativePoint, impulse));
	}

	void RigidBody::ApplyTorque(const glm::vec3& torque) {
		this->torque += torque;
	}

	void RigidBody::ApplyTorqueImpulse(const glm::vec3& torqueImpulse) {
		this->angularVelocity = torqueImpulse;
	}

	void RigidBody::SetGravityAcceleration(const glm::vec3& gravity) {
		this->gravity = gravity;
	}

	void RigidBody::UpdateAcceleration() {

		if (this->isStatic)
			return;

		this->linearAcceleration = this->force * this->invMass + this->gravity;
		this->angularAcceleration = this->torque;
	}

	void RigidBody::VerletStep1(float dt) {

		if (this->isStatic) {
			return;
		}

		this->previousPosition = this->position;
		this->position += (this->linearVelocity + this->linearAcceleration * (dt * 0.5f)) * dt;

		glm::vec3 rotationAxis = (this->angularVelocity + this->angularAcceleration * (dt * 0.5f)) * dt;

		float rotationAngle = glm::length(rotationAxis);

		rotationAxis = glm::normalize(rotationAxis);

		if (rotationAngle != 0.f) {
			glm::quat rot = glm::angleAxis(rotationAngle, rotationAxis);
			this->rotation *= rot;
		}
	}

	void RigidBody::VerletStep2(float dt) {

		if (this->isStatic)
			return;

		this->linearVelocity += this->linearAcceleration * (dt * 0.5f);
		this->angularVelocity += this->angularAcceleration * (dt * 0.5f);
	}

	void RigidBody::VerletStep3(float dt) {
		VerletStep2(dt);
	}

	void RigidBody::KillForces() {

		this->force = glm::vec3(0.f);
		this->torque = glm::vec3(0.f);
	}

	void RigidBody::ApplyDamping(float dt) {

		this->linearVelocity *= pow(1.f - this->linearDamping, dt);
		this->angularVelocity *= pow(1.f - this->angularDamping, dt);

		if (glm::length(this->linearVelocity) < 0.001f) {
			this->linearVelocity = glm::vec3(0.f);
		}
		if (glm::length(this->angularVelocity) < 0.001f) {
			this->angularVelocity = glm::vec3(0.f);
		}
	}
}

