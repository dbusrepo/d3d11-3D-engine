#include "Common.h"
#include "CCollision.h"

//-----------------------------------------------------------------------------
// Name : PointInTriangle () (Static)
// Desc : Test to see if a point falls within the bounds of a triangle.
//-----------------------------------------------------------------------------
bool PointInTriangle(const XMFLOAT3 & Point, const XMFLOAT3 & v1, const XMFLOAT3 & v2, const XMFLOAT3 & v3, const XMFLOAT3 & TriNormal)
{
	// First edge
	//Edge = v2 - v1;
	//Direction = v1 - Point;
	//D3DXVec3Cross(&EdgeNormal, &Edge, &TriNormal);
	// In front of edge?
	//if (D3DXVec3Dot(&Direction, &EdgeNormal) < 0.0f) return false;
	if (XMVectorGetX(
		XMVector3Dot(
			XMVectorSubtract(XMLoadFloat3(&v1), XMLoadFloat3(&Point)),
			XMVector3Cross(
				XMVectorSubtract(XMLoadFloat3(&v2), XMLoadFloat3(&v1)),
				XMLoadFloat3(&TriNormal))
		)
	) < 0.0f) return false;

	// Second edge
	//Edge = v3 - v2;
	//Direction = v2 - Point;
	//D3DXVec3Cross(&EdgeNormal, &Edge, &TriNormal);
	// In front of edge?
	//if (D3DXVec3Dot(&Direction, &EdgeNormal) < 0.0f) return false;
	if (XMVectorGetX(
		XMVector3Dot(
			XMVectorSubtract(XMLoadFloat3(&v2), XMLoadFloat3(&Point)),
			XMVector3Cross(
				XMVectorSubtract(XMLoadFloat3(&v3), XMLoadFloat3(&v2)),
				XMLoadFloat3(&TriNormal))
		)
	) < 0.0f) return false;

	// Third edge
	//Edge = v1 - v3;
	//Direction = v3 - Point;
	//D3DXVec3Cross(&EdgeNormal, &Edge, &TriNormal);
	// In front of edge?
	//if (D3DXVec3Dot(&Direction, &EdgeNormal) < 0.0f) return false;
	if (XMVectorGetX(
		XMVector3Dot(
			XMVectorSubtract(XMLoadFloat3(&v3), XMLoadFloat3(&Point)),
			XMVector3Cross(
				XMVectorSubtract(XMLoadFloat3(&v1), XMLoadFloat3(&v3)),
				XMLoadFloat3(&TriNormal))
		)
	) < 0.0f) return false;

	// We are behind all planes
	return true;
}

//-----------------------------------------------------------------------------
// Name : RayIntersectPlane () (Static)
// Desc : Determine if the specified ray intersects the plane, and if so
//        at what 't' value.
// Form : 't = ((P - O) . N) / (V . N)' where:
//        P = Ray Origin, O = Point on Plane, V = Ray Velocity, N = Plane Normal
//-----------------------------------------------------------------------------
bool RayIntersectPlane(const XMFLOAT3 & Origin, const XMFLOAT3 & Velocity, const XMFLOAT3 & PlaneNormal, const XMFLOAT3 & PlanePoint, float & t, bool BiDirectional)
{
	// Get the length of the 'adjacent' side of the virtual triangle formed
	// by the velocity and normal.
	//float ProjRayLength = D3DXVec3Dot(&Velocity, &PlaneNormal);
	float ProjRayLength = XMVectorGetX(XMVector3Dot(XMLoadFloat3(&Velocity), XMLoadFloat3(&PlaneNormal)));

	// If they are pointing in the same direction, we're can't possibly be colliding (i.e.
	// the dot product returned a positive value, with epsilon testing ). Remember that
	// our Velocity and Plane normal vectors are pointing towards one another in the case
	// where the two can be colliding (i.e. cos(a) >= -1 and <= 0).
	if (BiDirectional == false && ProjRayLength > -1e-5f) return false;

	// Since the interval (0 - 1) for intersection will be the same regardless of the distance
	// to the plane along either the hypotenuse OR the adjacent side of the triangle (bearing
	// in mind that we do 'distance / length_of_edge', and the length of the adjacent side is
	// relative to the length of the hypotenuse), we can just calculate the distance to the plane
	// the 'quick and easy way'(tm) ('shortest distance to' or 'distance along the negative plane normal')
	//float Distance = D3DXVec3Dot(&(Origin - PlanePoint), &PlaneNormal);

	float Distance = XMVectorGetX(XMVector3Dot(XMVectorSubtract(XMLoadFloat3(&Origin), XMLoadFloat3(&PlanePoint)),
		XMLoadFloat3(&PlaneNormal)));

	// Calculate the actual interval (Distance along the adjacent side / length of adjacent side).
	t = Distance / -ProjRayLength;

	// Outside our valid range? If yes, return no collide.
	if (t < 0.f || t > 1.f) return false;

	// We're intersecting
	return true;
}

//-----------------------------------------------------------------------------
// Name : RayIntersectPlane () (Static)
// Desc : Overload of previous function, accepting a plane distance instead.
//-----------------------------------------------------------------------------
bool RayIntersectPlane(const XMFLOAT3 & Origin, const XMFLOAT3 & Velocity, const XMFLOAT3 & PlaneNormal, float PlaneDistance, float & t, bool BiDirectional)
{
	// Get the length of the 'adjacent' side of the virtual triangle formed
	// by the velocity and normal.
	//float ProjRayLength = D3DXVec3Dot(&Velocity, &PlaneNormal);
	float ProjRayLength = XMVectorGetX(XMVector3Dot(XMLoadFloat3(&Velocity), XMLoadFloat3(&PlaneNormal)));

	if (BiDirectional == false && ProjRayLength > -1e-5f) return false;

	// Calculate distance to plane along it's normal
	//float Distance = D3DXVec3Dot(&Origin, &PlaneNormal) + PlaneDistance;
	float Distance = XMVectorGetX(XMVector3Dot(XMLoadFloat3(&Origin), XMLoadFloat3(&PlaneNormal)))
		+ PlaneDistance;

	// Calculate the actual interval (Distance along the adjacent side / length of adjacent side).
	t = Distance / -ProjRayLength;

	// Outside our valid range? If yes, return no collide.
	if (t < 0.0f || t > 1.0f) return false;

	// We're intersecting
	return true;
}

//-----------------------------------------------------------------------------
// Name : RayIntersectTriangle () (Static)
// Desc : Determine if the specified ray intersects the triangle, and if so
//        at what 't' value.
//-----------------------------------------------------------------------------
bool RayIntersectTriangle(const XMFLOAT3 & Origin, const XMFLOAT3 & Velocity, const XMFLOAT3 & v1, const XMFLOAT3 & v2, const XMFLOAT3 & v3, const XMFLOAT3 & TriNormal, bool BiDirectional)
{
	XMFLOAT3 Point;
	float t;

	// First calculate the intersection point with the triangle plane
	if (!RayIntersectPlane(Origin, Velocity, TriNormal, v1, t, BiDirectional)) return false;

	// Calculate the intersection point on the plane
	//Point = Origin + (Velocity * t);
	XMStoreFloat3(&Point,
		XMVectorMultiplyAdd(
			XMLoadFloat3(&Velocity),
			XMVectorReplicate(t),
			XMLoadFloat3(&Origin)
		)
	);
	// If this point does not fall within the bounds of the triangle, we are not intersecting
	if (t >= 1-1e-4f || !PointInTriangle(Point, v1, v2, v3, TriNormal)) return false;

	// We're intersecting the triangle
	return true;
}

CLASSIFYTYPE PointClassifyPlane(const XMFLOAT3 & Point, const XMFLOAT3 & PlaneNormal, float PlaneDistance)
{
	// Calculate distance from plane
	//float fDistance = D3DXVec3Dot(&Point, &PlaneNormal) + PlaneDistance;
	float fDistance = XMVectorGetX(XMVector3Dot(XMLoadFloat3(&Point), XMLoadFloat3(&PlaneNormal)))
		+ PlaneDistance;

	// Retrieve classification
	CLASSIFYTYPE Location = CLASSIFY_ONPLANE;
	if (fDistance < -1e-3f) Location = CLASSIFY_BEHIND;
	if (fDistance >  1e-3f) Location = CLASSIFY_INFRONT;

	// Return the classification
	return Location;
}