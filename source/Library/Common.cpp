#include "D3DAppException.h"
#include "Common.h"

ID3D11Buffer * Library::CreateD3DVertexBuffer(ID3D11Device* device, size_t size, bool dynamic, bool streamout, D3D11_SUBRESOURCE_DATA * pData)
{
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.ByteWidth = (UINT)size;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	// Select the appropriate binding locations based on the passed in flags
	if (streamout)
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;
	else
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	// Select the appropriate usage and CPU access flags based on the passed
	if (dynamic)
	{
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else
	{
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.CPUAccessFlags = 0;
	}

	// Create the buffer with the specified configuration
	ID3D11Buffer* pBuffer = 0;
	HRESULT hr = device->CreateBuffer(&desc, pData, &pBuffer);
	if (FAILED(hr))
	{
		throw D3DAppException("ID3D11Device::CreateBuffer() failed", hr);
		// Handle the error here...
		return(0);
	}

	return(pBuffer);
}

ID3D11Buffer * Library::CreateD3DIndexBuffer(ID3D11Device* device, size_t size, bool dynamic, D3D11_SUBRESOURCE_DATA * pData)
{
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.ByteWidth = (UINT)size;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	// Select the appropriate usage and CPU access flags based on the passed
	// in flags
	if (dynamic)
	{
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else
	{
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.CPUAccessFlags = 0;
	}

	// Create the buffer with the specified configuration
	ID3D11Buffer* pBuffer = 0;
	HRESULT hr = device->CreateBuffer(&desc, pData, &pBuffer);
	if (FAILED(hr))
	{
		throw D3DAppException("ID3D11Device::CreateBuffer() failed", hr);
		// Handle the error here...
		return(0);
	}

	return(pBuffer);
}
