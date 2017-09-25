#pragma once

#include "Common.h"

namespace Library
{
	class MatrixHelper
	{
	public:
		static const XMFLOAT4X4 Identity;

	private:
		MatrixHelper();
		MatrixHelper(const MatrixHelper& rhs);
		MatrixHelper& operator=(const MatrixHelper& rhs);
	};
}