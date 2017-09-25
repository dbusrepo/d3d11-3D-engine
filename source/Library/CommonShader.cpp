#include "CommonShader.h"

bool Library::BSPEngine::BasicMaterialVertex::same(const BasicMaterialVertex & v) const
{
	float f = 0.0f, d;

	d = this->Position.x - v.Position.x;
	f += d*d;
	d = this->Position.y - v.Position.y;
	f += d*d;
	d = this->Position.z - v.Position.z;
	f += d*d;

	return f < 0.01f;
}
