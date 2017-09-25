#pragma once
#include <DirectXMath.h>
using namespace DirectX;

bool RayIntersectTriangle(const XMFLOAT3 & Origin, const XMFLOAT3 & Velocity, const XMFLOAT3 & v1, const XMFLOAT3 & v2, const XMFLOAT3 & v3, const XMFLOAT3 & TriNormal, bool BiDirectional);
CLASSIFYTYPE PointClassifyPlane(const XMFLOAT3 & Point, const XMFLOAT3 & PlaneNormal, float PlaneDistance);
bool RayIntersectPlane(const XMFLOAT3 & Origin, const XMFLOAT3 & Velocity, const XMFLOAT3 & PlaneNormal, float PlaneDistance, float & t, bool BiDirectional);