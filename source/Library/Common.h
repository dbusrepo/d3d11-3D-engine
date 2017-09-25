#pragma once

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif

#define WINDOW_WIDTH 1366
#define WINDOW_HEIGHT 768

#include <windows.h>
#include <windowsx.h>
#include <exception>
#include <cassert>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include "RTTI.h"

#include <d3d11_1.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <dinput.h>

#define DeleteObject(object) if((object) != NULL) { delete object; object = NULL; }
#define DeleteObjects(objects) if((objects) != NULL) { delete[] objects; objects = NULL; }
#define ReleaseObject(object) if((object) != NULL) { object->Release(); object = NULL; }

using namespace DirectX;
using namespace DirectX::PackedVector;

namespace Library
{
	ID3D11Buffer* CreateD3DVertexBuffer(ID3D11Device* device, size_t size, bool dynamic, bool streamout, D3D11_SUBRESOURCE_DATA* pData);
	ID3D11Buffer* CreateD3DIndexBuffer(ID3D11Device* device, size_t size, bool dynamic, D3D11_SUBRESOURCE_DATA* pData);

	namespace BSPEngine {

	}

	typedef unsigned char byte;

	class TextHelper
	{
	public:

		static std::wstring s2ws(const std::string& str)
		{
			int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
			std::wstring wstrTo(size_needed, 0);
			MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
			return wstrTo;
		}

		template<typename T>
		static std::wstring ToString(const T& s)
		{
			std::wostringstream oss;
			oss << s;

			return oss.str();
		}

		template<typename T>
		static T FromString(const std::wstring& s)
		{
			T x;
			std::wistringstream iss(s);
			iss >> x;

			return x;
		}
	};

	class Convert
	{
	public:

		// Converts XMVECTOR to XMCOLOR, where XMVECTOR represents a color.
		static XMCOLOR ToXmColor(FXMVECTOR v)
		{
			XMCOLOR dest;
			XMStoreColor(&dest, v);
			return dest;
		}

		// Converts XMVECTOR to XMFLOAT4, where XMVECTOR represents a color.
		static XMFLOAT4 ToXmFloat4(FXMVECTOR v)
		{
			XMFLOAT4 dest;
			XMStoreFloat4(&dest, v);
			return dest;
		}

		static UINT ArgbToAbgr(UINT argb)
		{
			BYTE A = (argb >> 24) & 0xff;
			BYTE R = (argb >> 16) & 0xff;
			BYTE G = (argb >> 8) & 0xff;
			BYTE B = (argb >> 0) & 0xff;

			return (A << 24) | (B << 16) | (G << 8) | (R << 0);
		}

	};


}



