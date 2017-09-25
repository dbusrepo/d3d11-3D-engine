#pragma once
#include "Common.h"

namespace Library
{
	namespace BSPEngine {
		float DistanceToLineSegment(const XMFLOAT3& vecPoint, const XMFLOAT3& vecStart, const XMFLOAT3& vecEnd);
		void  TransformAABB(XMFLOAT3 & Min, XMFLOAT3 & Max, const XMFLOAT4X4 & mtxTransform);
	}

}