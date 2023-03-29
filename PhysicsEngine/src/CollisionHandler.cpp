#include "../include/CollisionHandler.h"
#include "../include/RigidBody.h"

#include <glm/gtx/projection.hpp>

namespace physics
{
	// Page 224 Chapter 5 Basic Primitive Tests
	// Christer Ericson - Real-time collision detection
	bool TestMovingSphereSphere(
		const glm::vec3& s0Center, const float s0Radius,
		const glm::vec3& s1Center, const float s1Radius,
		const glm::vec3& v0, const glm::vec3& v1, float& t)
	{
		glm::vec3 s = s1Center - s0Center;	// Vector between sphere centers
		glm::vec3 v = v1 - v0;				// Relative motion of s1 with respect to stationary s0
		float r = s1Radius + s0Radius;		// Sum of sphere radii
		float c = glm::dot(s, s) - r * r;

		if (c < 0.f) {
			// Already intersecting, not moving towards a collision
			t = 0.f;
			return true;
		}

		float a = glm::dot(v, v);
		if (a < std::numeric_limits<float>::epsilon())
			return false;		// Spheres not moving relative to eachother

		float b = glm::dot(v, s);
		if (b >= 0.f)
			return false;		// Spheres not moving towards eachother

		float d = (b * b) - (a * c);
		if (d < 0.f)
			return false;		// No real-valued root, spheres do not intersect

		t = (-b - sqrt(d)) / a;
		return true;
	}

	// Page #? Chapter 5 Basic Primitive Tests
	// Christer Ericson - Real-time collision detection
	bool TestMovingSpherePlane(const glm::vec3& prevPosition, const glm::vec3& currPosition, float radius, const glm::vec3& norm, float dotProduct)
	{
		// Get distance for both a and b from the plane
		float adist = glm::dot(prevPosition, norm) - dotProduct;
		float bdist = glm::dot(currPosition, norm) - dotProduct;

		// Intersects if on different sides of a plane (distance have different signs)
		if (adist * bdist < 0.f)
			return true;

		// Intersects if start or end position within radius from plane
		if (abs(adist) < radius || abs(bdist) <= radius)
			return true;

		// No intersection
		return false;
	}

	CollisionHandler::CollisionHandler()
	{

	}

	CollisionHandler::~CollisionHandler()
	{

	}

	// From Christer Ericson book
	glm::vec3 CollisionHandler::ClosestPtPointPlane(const glm::vec3& pt, const glm::vec3& planeNormal, float planeDotProduct)
	{
		float t = glm::dot(planeNormal, pt) - planeDotProduct;
		return pt - t * planeNormal;
	}

	bool CollisionHandler::CollideSphereSphere(float dt, RigidBody* bodyA, SphereShape* sphereA,
		RigidBody* bodyB, SphereShape* sphereB)
	{
		if (bodyA->IsStatic() && bodyB->IsStatic())
			return false;

		// TestMovingSphereSphere
		// if There is no a collision detected we will return false
		glm::vec3 v0 = bodyA->position - bodyA->previousPosition;
		glm::vec3 v1 = bodyB->position - bodyB->previousPosition;
		float t = 0.f;

		if (!TestMovingSphereSphere(
			bodyA->previousPosition, // We want PreviousPosition
			sphereA->GetRadius(),
			bodyB->previousPosition, // We want PreviousPosition
			sphereB->GetRadius(),
			v0, v1, t))
		{
			// There is no collision or future collision
			return false;
		}

		if (t > dt) {
			// Collision will not happen this time step
			return false;
		}

		// Otherwise, we will handle the collision here

		// calculate what percent of DT to reverse the Verlet Step

		// This might be wrong...
		float fractDT = t / (glm::length(v0) + glm::length(v1));
		float revDT = (1.0f - fractDT) * dt;
		bodyA->VerletStep1(-revDT);
		bodyB->VerletStep1(-revDT);

		// p = mv
		// p: momentum
		// m: mass
		// v: velocity

		// Total Mass & Momentum calculation
		float totalMass = bodyB->mass + bodyA->mass;
		float aFactor = bodyB->mass / totalMass;
		float bFactor = bodyA->mass / totalMass;

		glm::vec3 aMomentum = bodyA->linearVelocity * bodyA->mass;
		glm::vec3 bMomentum = bodyB->linearVelocity * bodyB->mass;
		glm::vec3 totalMomentum = aMomentum + bMomentum;

		aMomentum = totalMomentum * aFactor;
		bMomentum = totalMomentum * bFactor;

		// Check if any part of the objects are still overlapping.
		// We need to nudge the objects back more if they are still overlapping.
		glm::vec3 vec = bodyB->position - bodyA->position;
		float vecLength = glm::length(vec);
		float overlap = vecLength - (sphereA->GetRadius() + sphereB->GetRadius());

		if (overlap < std::numeric_limits<float>::epsilon())
		{
			glm::vec3 overlapVec = vec;
			overlapVec = glm::normalize(overlapVec);
			overlapVec *= -overlap;

			// Nudge out of the collision a bit more
			if (!bodyA->isStatic) bodyA->position -= overlapVec * aFactor;
			if (!bodyB->isStatic) bodyB->position += overlapVec * bFactor;

			vec = bodyB->position - bodyA->position;
			vecLength = glm::length(vec);
		}
		vec /= vecLength;

		// Calculate elastic and inelastic momentum for both bodyA and bodyB
		float elasticity = 0.4f;

		glm::vec3 aElasticMomentum = vec * (glm::length(aMomentum) * elasticity);
		glm::vec3 aInelasticMomentum = vec * glm::length(aMomentum) * (1.f - elasticity);

		glm::vec3 bElasticMomentum = vec * (glm::length(bMomentum) * elasticity) * -1.f;
		glm::vec3 bInelasticMomentum = vec * glm::length(bMomentum) * (1.f - elasticity);

		// Finally use our elastic & inelastic momentums to calculate a new velocity
		bodyA->linearVelocity -= (aElasticMomentum + aInelasticMomentum) * bodyA->invMass * bodyA->restitution;
		bodyB->linearVelocity += (bElasticMomentum + bInelasticMomentum) * bodyB->invMass * bodyB->restitution;

		// Verlet integrate
		bodyA->VerletStep1(revDT);
		bodyB->VerletStep1(revDT);

		return true;
	}


	bool CollisionHandler::CollideSpherePlane(float dt, RigidBody* sphere, SphereShape* sphereShape,
		RigidBody* plane, PlaneShape* planeShape)
	{
		// TestMovingSpherePlane
		if (!TestMovingSpherePlane(sphere->previousPosition, sphere->position, sphereShape->GetRadius(),
			planeShape->GetNormal(), planeShape->GetDotProduct()))
		{
			return false;
		}

		glm::vec3 closestPoint = ClosestPtPointPlane(sphere->position, planeShape->GetNormal(), planeShape->GetDotProduct());
		glm::vec3 overlapVector = closestPoint - sphere->position;
		float overlapLength = glm::length(overlapVector);
		float linearVelocityLength = glm::length(sphere->linearVelocity);
		float angularVelocityLength = glm::length(sphere->angularVelocity);

		if (linearVelocityLength > 0.f || angularVelocityLength > 0.f)
		{
			float velocity = glm::length(sphere->linearVelocity);
			float fractDt = 0.f;
			if (velocity != 0.0f)
			{
				fractDt = sphereShape->GetRadius() * ((sphereShape->GetRadius() / overlapLength) - 1.0f) / velocity;
			}
			float partialDt = (1.f - fractDt) * dt;

			// Reverse the sphere out of the plane
			sphere->VerletStep1(-partialDt);

			// calculate the reflection (Bounce) off the plane
			glm::vec3 prevVelocity = sphere->linearVelocity;
			glm::vec3 reflect = glm::reflect(sphere->linearVelocity, planeShape->GetNormal());
			sphere->linearVelocity = reflect;

			// calculate impact info
			glm::vec3 impactComponent = glm::proj(sphere->linearVelocity, planeShape->GetNormal());
			glm::vec3 impactTangent = sphere->linearVelocity - impactComponent;


			glm::vec3 relativePoint = glm::normalize(closestPoint - sphere->position) * sphereShape->GetRadius();
			float surfaceVelocity = sphereShape->GetRadius() * glm::length(sphere->angularVelocity);
			glm::vec3 rotationDirection = glm::normalize(glm::cross(relativePoint - sphere->position, sphere->angularVelocity));



			// Detect if we are bouncing off the plane, or "moving" along it.
			if (glm::dot(impactTangent, planeShape->GetNormal()) > 0.f)
			{
				sphere->ApplyImpulseAtPoint(-2.f * impactComponent * sphere->mass, relativePoint);
				//sphere->ApplyImpulseAtPoint( , relativePoint);
			}
			else
			{
				glm::vec3 impactForce = impactTangent * -1.f * sphere->mass * plane->friction;
				sphere->ApplyForceAtPoint(impactForce, relativePoint);
			}

			if (glm::dot(sphere->linearVelocity, planeShape->GetNormal()) == 0.0f)
			{
				glm::vec3 force = surfaceVelocity * rotationDirection * sphere->mass * plane->friction;
				sphere->ApplyForce(force);
			}

			sphere->UpdateAcceleration();

			// Move the sphere into the new direction
			sphere->VerletStep1(partialDt);

			//if (glm::length(impactTangent) > 0.001f)
			//{
			//	sphere->m_Velocity += impactTangent * 0.1f;
			//}

			// Here we ensure we are on the right side of the plane
			closestPoint = ClosestPtPointPlane(sphere->position, planeShape->GetNormal(), planeShape->GetDotProduct());
			overlapVector = closestPoint - sphere->position;
			overlapLength = glm::length(overlapVector);
			if (overlapLength < sphereShape->GetRadius())
			{
				// we are still colliding!!!

				sphere->position += planeShape->GetNormal() * (sphereShape->GetRadius() - overlapLength);

				float velocityDotNormal = glm::dot(sphere->linearVelocity, planeShape->GetNormal());

				if (velocityDotNormal < 0.f)
				{
					sphere->linearVelocity -= planeShape->GetNormal() * velocityDotNormal;
				}
			}
			else
			{
				sphere->linearVelocity *= sphere->restitution;
			}

		}
		else
		{
			return false;
		}

		return true;
	}


	void CollisionHandler::Collide(float dt, std::vector<iCollisionBody*>& bodies, std::vector<CollidingBodies>& collisions)
	{
		int bodyCount = bodies.size();
		bool collision = false;
		for (int idxA = 0; idxA < bodyCount - 1; idxA++)
		{
			iCollisionBody* bodyA = bodies[idxA];

			for (int idxB = idxA + 1; idxB < bodyCount; idxB++)
			{
				iCollisionBody* bodyB = bodies[idxB];

				collision = false;

				// CHECK WHICH BODY TYPES...
				if (bodyA->GetBodyType() == BodyType::RigidBody)
				{
					if (bodyB->GetBodyType() == BodyType::RigidBody)
					{
						collision = CollideRigidRigid(dt, RigidBody::Cast(bodyA), RigidBody::Cast(bodyB));
					}
					else if (bodyB->GetBodyType() == BodyType::SoftBody)
					{
						// Dont have soft bodies atm
					}
					else
					{
						// We don't know this type of body
					}
				}
				// Dont have soft bodies atm
				else if (bodyA->GetBodyType() == BodyType::SoftBody)
				{
					if (bodyB->GetBodyType() == BodyType::RigidBody)
					{
						// Dont have soft bodies atm
					}
					else if (bodyB->GetBodyType() == BodyType::SoftBody)
					{
						// Dont have soft bodies atm
					}
					else
					{
						// don't know this type of body
					}
				}
				else
				{
					// don't know this type of body
				}

				if (collision)
				{
					collisions.push_back(CollidingBodies(bodyA, bodyB));
				}
			}
		}
	}

	bool CollisionHandler::CollideRigidRigid(float dt, RigidBody* rigidA, RigidBody* rigidB)
	{
		iShape* shapeB = rigidA->GetShape();
		iShape* shapeA = rigidB->GetShape();

		bool collision = false;

		if (shapeA->GetShapeType() == ShapeType::Sphere)
		{
			if (shapeB->GetShapeType() == ShapeType::Sphere)
			{
				collision = CollideSphereSphere(dt, rigidA, SphereShape::Cast(shapeA), rigidB, SphereShape::Cast(shapeB));
			}
			else if (shapeB->GetShapeType() == ShapeType::Plane)
			{
				collision = CollideSpherePlane(dt, rigidA, SphereShape::Cast(shapeA), rigidB, PlaneShape::Cast(shapeB));
			}
			else if (shapeB->GetShapeType() == ShapeType::Box)
			{
				//CollideSphereBox(dt, bodyA, SphereShape::Cast(shapeA), bodyB, BoxShape::Cast(shapeB));
			}
			else
			{
				// We don't have this handled at the moment.
			}
		}
		else if (shapeA->GetShapeType() == ShapeType::Plane)
		{
			if (shapeB->GetShapeType() == ShapeType::Sphere)
			{
				collision = CollideSpherePlane(dt, rigidB, SphereShape::Cast(shapeB), rigidA, PlaneShape::Cast(shapeA));
			}
			else if (shapeB->GetShapeType() == ShapeType::Plane)
			{
				// Dont have plane plane collisions atm.
			}
			else
			{
				// Don't have this handled at the moment.
			}
		}
		else
		{
			// Don't have this handled at the moment.
		}

		return collision;
	}
}
