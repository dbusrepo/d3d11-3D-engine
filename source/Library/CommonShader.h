#pragma once
#include "Common.h"

namespace Library
{
	namespace BSPEngine {

		struct BasicMaterialVertex
		{
			XMFLOAT3 Position;

			BasicMaterialVertex() { }

			BasicMaterialVertex(XMFLOAT3 position)
				: Position(position) { }

			bool same(const BasicMaterialVertex &vertex) const;
		};

	}
}