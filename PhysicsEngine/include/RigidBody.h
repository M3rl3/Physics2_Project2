#pragma once

#include "../interfaces/iRigidBody.h"
#include "../interfaces/RigidBodyDesc.h"
#include "../interfaces/iShape.h"

namespace physics {

	class RigidBody : public iRigidBody {

	public:
		RigidBody();
		virtual ~RigidBody();

		RigidBody(const RigidBodyDesc& desc, iShape* shape);

		static RigidBody* Cast(iCollisionBody* body);

		iShape* GetShape();

		bool IsStatic();
		virtual void GetPosition(glm::vec3& position) override;
		virtual void SetPosition(const glm::vec3& position) override;

		virtual void GetRotation(glm::quat& rotation) override;
		virtual void SetRotation(const glm::quat& rotation) override;

		virtual void ApplyForce(const glm::vec3& force) override;
		virtual void ApplyImpulse(const glm::vec3& impulse) override;

		virtual void ApplyForceAtPoint(const glm::vec3& force, const glm::vec3& relativePoint) override;
		virtual void ApplyImpulseAtPoint(const glm::vec3& impulse, const glm::vec3& relativePoint) override;

		virtual void ApplyTorque(const glm::vec3& torque) override;
		virtual void ApplyTorqueImpulse(const glm::vec3& torqueImpulse) override;

		void SetGravityAcceleration(const glm::vec3& gravity);
		void UpdateAcceleration();

		void VerletStep1(float dt);
		void VerletStep2(float dt);
		void VerletStep3(float dt);

		void KillForces();

		void ApplyDamping(float dt);

		void SetRenderPosition(glm::vec3* position);

		void Update(float v);

		friend class CollisionHandler;

	private:
		float mass;
		float invMass;
		float friction;
		float restitution;
		float linearDamping;
		float angularDamping;
		bool isStatic;

		glm::vec3 position;
		glm::vec3 previousPosition;
		glm::vec3 linearVelocity;
		glm::vec3 linearAcceleration;

		glm::vec3 force;
		glm::vec3 torque;
		glm::vec3 gravity;

		glm::quat rotation;
		glm::vec3 angularVelocity;
		glm::vec3 angularAcceleration;

		iShape* shape;

		glm::vec3* renderPosition;

		RigidBody(const RigidBody&);
	};
}