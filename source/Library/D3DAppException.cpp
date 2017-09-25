#include "D3DAppException.h"

namespace Library
{
	D3DAppException::D3DAppException(const char* const& message, HRESULT hr)
		: exception(message), mHR(hr)
	{
	}

	HRESULT D3DAppException::HR() const
	{
		return mHR;
	}

	std::wstring D3DAppException::whatw() const
	{
		std::string whatString(what());
		std::wstring whatw;
		whatw.assign(whatString.begin(), whatString.end());

		return whatw;
	}
}