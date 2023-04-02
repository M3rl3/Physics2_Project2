#include "Conversion.h"

#include "BoxShape.h"
#include "PlaneShape.h"
#include "SphereShape.h"
#include "CylinderShape.h"

#include "RigidBody.h"

void physics::CastGLMQuat(const btQuaternion& in, glm::quat* out)
{
	CastFloat(in.x(), &out->x);
	CastFloat(in.y(), &out->y);
	CastFloat(in.z(), &out->z);
	CastFloat(in.w(), &out->w);
}

void physics::CastGLMVec3(const btVector3& in, glm::vec3* out)
{
	CastFloat(0.0f, &out->p);
	CastFloat(0.0f, &out->r);
	CastFloat(0.0f, &out->s);
	CastFloat(0.0f, &out->t);
	CastFloat(in.x(), &out->x);
	CastFloat(in.y(), &out->y);
	CastFloat(in.z(), &out->z);
}

void physics::CastFloat(const btScalar& in, float* out)
{
	*out = in;
}

void physics::CastGLMMat4(const btTransform& in, glm::mat4* out)
{
}

void physics::CastBulletQuaternion(const glm::quat& in, btQuaternion* out)
{
	out->setX(in.x);
	out->setY(in.y);
	out->setZ(in.z);
	out->setW(in.w);
}

void physics::CastBulletVector3(const glm::vec3& in, btVector3* out)
{
	out->setX(in.x);
	out->setY(in.y);
	out->setZ(in.z);
}

void physics::CastBulletScalar(const float in, btScalar* out)
{
	*out = in;
}

btCollisionShape* physics::CastBulletShape(iShape* shape)
{
	switch (shape->GetShapeType())
	{
	case ShapeType::Ghost:
	{
		btEmptyShape* btEmpty = new btEmptyShape();

		return btEmpty;
	} break;

	case ShapeType::Box:
	{
		BoxShape* box = BoxShape::Cast(shape);

		btVector3 halfExtents;

		CastBulletVector3(box->GetHalfExtents(), &halfExtents);

		btBoxShape* btBox = new btBoxShape(halfExtents);

		return btBox;
	} break;

	case ShapeType::Plane:
	{
		PlaneShape* plane = PlaneShape::Cast(shape);

		btVector3 normal;
		btScalar planeConstant;

		CastBulletVector3(plane->GetNormal(), &normal);
		CastBulletScalar(plane->GetDotProduct(), &planeConstant);

		btStaticPlaneShape* btPlane = new btStaticPlaneShape(normal, planeConstant);

		return btPlane;
	} break;

	case ShapeType::Sphere:
	{
		SphereShape* sphere = SphereShape::Cast(shape);

		btScalar radius;

		CastBulletScalar(sphere->GetRadius(), &radius);

		btSphereShape* btSphere = new btSphereShape(radius);

		return btSphere;

	} break;

	case ShapeType::Cylinder:
	{
		CylinderShape* mesh = CylinderShape::Cast(shape);

		btVector3 btHalfExtents;

		CastBulletVector3(mesh->GetHalfExtents(), &btHalfExtents);

		btCylinderShape* btCylinder = new btCylinderShape(btHalfExtents);

		return btCylinder;
	} break;

	default:
		break;

		return 0;
	}
}

btRigidBody* physics::CastBulletRigidBody(iCollisionBody* body)
{
	if (body == nullptr)
		return nullptr;

	return dynamic_cast<RigidBody*>(body)->GetBulletBody();
}