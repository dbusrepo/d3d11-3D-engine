#pragma once

#include <exception>
#include <Windows.h>
#include <string>

namespace Library
{
	class D3DAppException : public std::exception
	{
	public:
		D3DAppException(const char* const& message, HRESULT hr = S_OK);

		HRESULT HR() const;
		std::wstring whatw() const;

	private:
		HRESULT mHR;
	};
}
