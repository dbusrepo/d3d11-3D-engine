#include "MathUtility.h"

//-----------------------------------------------------------------------------
// Name : DistanceToLine ()
// Desc : Calculates the distance from the position specified, and
//        the line segment passed into this function.
// Note : Returns a value that is out of range if the point is outside of the
//        extents of vecStart & vecEnd.
//-----------------------------------------------------------------------------
float Library::BSPEngine::DistanceToLineSegment(const XMFLOAT3 & vecPoint, const XMFLOAT3 & vecStart, const XMFLOAT3 & vecEnd)
{
	XMFLOAT3 c, v;
	float    d, t;

	
	// Determine t (the length of the vector from ‘vecStart’ to ‘vecPoint’ projected onto segment)
	//c = vecPoint - vecStart;
	XMStoreFloat3(&c, XMLoadFloat3(&vecPoint) - XMLoadFloat3(&vecStart));
	//v = vecEnd - vecStart;
	XMStoreFloat3(&v, XMLoadFloat3(&vecEnd) - XMLoadFloat3(&vecStart));
	//d = D3DXVec3Length(&v);
	d = XMVectorGetX(XMVector3Length(XMLoadFloat3(&v)));

	// Normalize V
	//v /= d;
	XMStoreFloat3(&v, XMVectorScale(XMLoadFloat3(&v), 1.0f/d));//XMVector3Normalize(XMLoadFloat3(&v)));
	
	// Calculate final t value
	//t = D3DXVec3Dot(&v, &c);
	t = XMVectorGetX(XMVector3Dot(XMLoadFloat3(&v), XMLoadFloat3(&c)));

	// Check to see if ‘t’ is beyond the extents of the line segment
	if (t < 0.01f)     return FLT_MAX;
	if (t > d - 0.01f) return FLT_MAX;

	// Calculate intersection point on the line
	//v.x = vecStart.x + (v.x * t);
	//v.y = vecStart.y + (v.y * t);
	//v.z = vecStart.z + (v.z * t);

	XMStoreFloat3(&v, XMVectorMultiplyAdd(XMLoadFloat3(&v), XMVectorReplicate(t), XMLoadFloat3(&vecStart)));

	// Return the length
	//return D3DXVec3Length(&(vecPoint - v));
	return XMVectorGetX(XMVector3Length(XMVectorSubtract(XMLoadFloat3(&vecPoint), XMLoadFloat3(&v))));
}

//-----------------------------------------------------------------------------
// Name : TransformAABB ()
// Desc : Transforms an axis aligned bounding box, by the specified matrix,
//        outputting new AAB values which are a best fit about that 'virtual
//        transformation'.
//-----------------------------------------------------------------------------
void Library::BSPEngine::TransformAABB(XMFLOAT3 & Min, XMFLOAT3 & Max, const XMFLOAT4X4 & mtxTransform)
{
	//D3DXVECTOR3 BoundsCentre = (Min + Max) / 2.0f;
	XMFLOAT3 BoundsCentre;
	XMStoreFloat3(&BoundsCentre, XMVectorScale(XMVectorAdd(XMLoadFloat3(&Min), XMLoadFloat3(&Max)), 1.0f / 2.0f));

	//D3DXVECTOR3 Extents = Max - BoundsCentre;
	XMFLOAT3 Extents;
	XMStoreFloat3(&Extents, XMVectorSubtract(XMLoadFloat3(&Max), XMLoadFloat3(&BoundsCentre)));

	XMFLOAT3 Ex, Ey, Ez;

	XMFLOAT4X4 mtx = mtxTransform;

	// Clear translation
	mtx._41 = 0; mtx._42 = 0; mtx._43 = 0;

	// Compute new centre
	//D3DXVec3TransformCoord(&BoundsCentre, &BoundsCentre, &mtx);
	XMStoreFloat3(&BoundsCentre, XMVector3TransformCoord(XMLoadFloat3(&BoundsCentre), XMLoadFloat4x4(&mtx)));

	// Compute new extents values
	//Ex = D3DXVECTOR3(mtx._11, mtx._12, mtx._13) * Extents.x;
	//Ey = D3DXVECTOR3(mtx._21, mtx._22, mtx._23) * Extents.y;
	//Ez = D3DXVECTOR3(mtx._31, mtx._32, mtx._33) * Extents.z;
	
	Ex = XMFLOAT3(mtx._11, mtx._12, mtx._13);
	Ey = XMFLOAT3(mtx._21, mtx._22, mtx._23);
	Ez = XMFLOAT3(mtx._31, mtx._32, mtx._33);

	XMStoreFloat3(&Ex, XMVectorScale(XMLoadFloat3(&Ex), Extents.x));
	XMStoreFloat3(&Ey, XMVectorScale(XMLoadFloat3(&Ey), Extents.y));
	XMStoreFloat3(&Ez, XMVectorScale(XMLoadFloat3(&Ez), Extents.z));

	// Calculate new extents actual
	Extents.x = fabsf(Ex.x) + fabsf(Ey.x) + fabsf(Ez.x);
	Extents.y = fabsf(Ex.y) + fabsf(Ey.y) + fabsf(Ez.y);
	Extents.z = fabsf(Ex.z) + fabsf(Ey.z) + fabsf(Ez.z);

	// Calculate final bounding box (add on translation)
	Min.x = (BoundsCentre.x - Extents.x) + mtxTransform._41;
	Min.y = (BoundsCentre.y - Extents.y) + mtxTransform._42;
	Min.z = (BoundsCentre.z - Extents.z) + mtxTransform._43;
	Max.x = (BoundsCentre.x + Extents.x) + mtxTransform._41;
	Max.y = (BoundsCentre.y + Extents.y) + mtxTransform._42;
	Max.z = (BoundsCentre.z + Extents.z) + mtxTransform._43;
}
