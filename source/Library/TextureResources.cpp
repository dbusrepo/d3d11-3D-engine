#include "TextureResources.h"
#include "D3DAppException.h"
#include <WICTextureLoader.h>
//#include <DDSTextureLoader.h>

using namespace std;

namespace Library {

	BSPEngine::TextureResources::TextureResources()
	{
	}

	BSPEngine::TextureResources::~TextureResources()
	{
		for (auto it = begin(mTextureItems); it != end(mTextureItems); ++it) {
			TextureItem *item = it->second;
			delete item;
			it->second = nullptr;
		}
	}

	void BSPEngine::TextureResources::LoadTexture(ID3D11Device *device, const std::string &texName, const std::wstring &textureFileName, uint16_t texID)
	{

		TextureItem *textureItem = new TextureItem;
		
		textureItem->texName = texName;
		HRESULT hr = CreateWICTextureFromFileEx(device, textureFileName.c_str(),
			0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, 0, D3D11_RESOURCE_MISC_GENERATE_MIPS, true, &textureItem->mTexture, &textureItem->mTextureView);
		if (FAILED(hr))
		{
			throw D3DAppException("CreateDDSTextureFromFile() failed.", hr);
		}
		
		mTextureItems.insert({ texID , textureItem });
	}

	BSPEngine::TextureItem * BSPEngine::TextureResources::getTextureItem(uint16_t texID)
	{
		auto it = mTextureItems.find(texID);
		if (it != end(mTextureItems)) {
			return it->second;
		}
		return nullptr;
	}

	void BSPEngine::TextureResources::addTextureDiffuseMap(const std::wstring & textureFilePath, uint16_t texID)
	{
		mDiffuseMapInfo.push_back({ textureFilePath, texID, nullptr } );
	}

	void BSPEngine::TextureResources::createTextureArrayDiffuseMaps(ID3D11Device *device, ID3D11DeviceContext *deviceContext)
	{
		createTextureArrays(device, deviceContext, mDiffuseMapInfo, mTextureDiffuseViews, false);
	}

	void BSPEngine::TextureResources::addTextureNormalMap(const std::wstring & textureFilePath, uint16_t texID)
	{
		mNormalMapInfo.push_back({ textureFilePath, texID, nullptr });
	}

	void BSPEngine::TextureResources::createTextureArrayNormalMaps(ID3D11Device * device, ID3D11DeviceContext * deviceContext)
	{
		createTextureArrays(device, deviceContext, mNormalMapInfo, mTextureNormalArrayViews, true);
	}

	void BSPEngine::TextureResources::addLightMap(const std::wstring & LightMapFilePath)
	{
		mLightMapInfo.push_back({ LightMapFilePath, 0, nullptr });
	}

	void BSPEngine::TextureResources::createTextureArrayLightMaps(ID3D11Device * device, ID3D11DeviceContext * deviceContext)
	{
		UINT numLightmaps = (UINT)mLightMapInfo.size();

		if (numLightmaps < 1) return;
		
		std::vector<D3D11_SUBRESOURCE_DATA> subDataVec;
		subDataVec.reserve(mLightMapInfo.size());
		UINT lightMapWidth, lightMapHeight;		

		for (auto it = std::begin(mLightMapInfo); it != end(mLightMapInfo); it++)
		{
			const std::wstring &texFilePath = it->filePath;
			ScratchImage *image = new ScratchImage;
			LoadFromDDSFile(texFilePath.c_str(), DDS_FLAGS_NONE, nullptr, *image);
			
			D3D11_SUBRESOURCE_DATA subData;
			const Image *lightMap = image->GetImage(0, 0, 0);
			subData.pSysMem = lightMap->pixels;
			subData.SysMemPitch = (UINT)lightMap->rowPitch;
			subData.SysMemSlicePitch = (UINT)lightMap->slicePitch;
			subDataVec.push_back(subData);
			lightMapWidth = (UINT)lightMap->width; // lightmaps have same size
			lightMapHeight = (UINT)lightMap->height;
		}

		// Setup the description of the texture array.
		D3D11_TEXTURE2D_DESC textureDesc;
		ZeroMemory(&textureDesc, sizeof(textureDesc));

		textureDesc.Width = lightMapWidth;
		textureDesc.Height = lightMapHeight;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = numLightmaps;
		textureDesc.Format = DXGI_FORMAT_R8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;

		// Create the texture array.
		ID3D11Texture2D *textureArray;
		HRESULT hr = device->CreateTexture2D(&textureDesc, &subDataVec[0], &textureArray);
		if (FAILED(hr))
		{
			throw D3DAppException("CreateTexture2D() failed.", hr);
		}

		// Setup the shader resource view description.
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDesc.Texture2DArray.ArraySize = numLightmaps;
		srvDesc.Texture2DArray.FirstArraySlice = 0;
		srvDesc.Texture2DArray.MipLevels = 1;
		srvDesc.Texture2DArray.MostDetailedMip = 0;

		// Create the shader resource view for the texture array.
		hr = device->CreateShaderResourceView(textureArray, &srvDesc, &mLightMapsArrayView);
		if (FAILED(hr))
		{
			throw D3DAppException("CreateShaderResourceView() failed.", hr);
		}

		ReleaseObject(textureArray); // keep only view reference

		mLightMapInfo.clear();
		mLightMapInfo.shrink_to_fit();
	}

	void BSPEngine::TextureResources::addClusterMap(const std::wstring & ClusterMapFilePath)
	{
		mClusterMapInfo.push_back({ ClusterMapFilePath, 0, nullptr });
	}

	void BSPEngine::TextureResources::createTextureArrayClusterMaps(ID3D11Device * device, ID3D11DeviceContext * deviceContext)
	{
		UINT numClusterMaps = (UINT)mClusterMapInfo.size();

		if (numClusterMaps < 1) return;

		std::vector<D3D11_SUBRESOURCE_DATA> subDataVec;
		subDataVec.reserve(mClusterMapInfo.size());
		UINT clusterMapWidth, clusterMapHeight;

		for (auto it = std::begin(mClusterMapInfo); it != end(mClusterMapInfo); it++)
		{
			const std::wstring &texFilePath = it->filePath;
			ScratchImage *image = new ScratchImage;
			LoadFromDDSFile(texFilePath.c_str(), DDS_FLAGS_NONE, nullptr, *image);

			D3D11_SUBRESOURCE_DATA subData;
			const Image *clusterMap = image->GetImage(0, 0, 0);
			subData.pSysMem = clusterMap->pixels;
			subData.SysMemPitch = (UINT)clusterMap->rowPitch;
			subData.SysMemSlicePitch = (UINT)clusterMap->slicePitch;
			subDataVec.push_back(subData);
			clusterMapWidth = (UINT)clusterMap->width; // cluster maps have same size
			clusterMapHeight = (UINT)clusterMap->height;
		}

		// Setup the description of the texture array.
		D3D11_TEXTURE2D_DESC textureDesc;
		ZeroMemory(&textureDesc, sizeof(textureDesc));

		textureDesc.Width = clusterMapWidth;
		textureDesc.Height = clusterMapHeight;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = numClusterMaps;
		textureDesc.Format = DXGI_FORMAT_R16_UINT;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;

		// Create the texture array.
		ID3D11Texture2D *textureArray;
		HRESULT hr = device->CreateTexture2D(&textureDesc, &subDataVec[0], &textureArray);
		if (FAILED(hr))
		{
			throw D3DAppException("CreateTexture2D() failed.", hr);
		}

		// Setup the shader resource view description.
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDesc.Texture2DArray.ArraySize = numClusterMaps;
		srvDesc.Texture2DArray.FirstArraySlice = 0;
		srvDesc.Texture2DArray.MipLevels = 1;
		srvDesc.Texture2DArray.MostDetailedMip = 0;

		// Create the shader resource view for the texture array.
		hr = device->CreateShaderResourceView(textureArray, &srvDesc, &mClustersMapArrayView);
		if (FAILED(hr))
		{
			throw D3DAppException("CreateShaderResourceView() failed.", hr);
		}

		ReleaseObject(textureArray); // keep only view reference

		mClusterMapInfo.clear();
		mClusterMapInfo.shrink_to_fit();

		m_clusterMapSize = XMFLOAT2((float)clusterMapWidth, (float)clusterMapHeight);
	}

	void BSPEngine::TextureResources::createTextureArrays(ID3D11Device * device, ID3D11DeviceContext * deviceContext, std::vector<TexInfo> &mapInfoVec, std::vector<ID3D11ShaderResourceView*>& textureArraysViewsVec, bool normalMaps)
	{
		std::map<std::pair<size_t, size_t>, std::vector<TexInfo>> imagesMap;

		for (auto it = std::begin(mapInfoVec); it != end(mapInfoVec); it++) {
			const std::wstring &texFilePath = it->filePath;

			ScratchImage *image = new ScratchImage;
			//LoadFromWICFile(texFilePath.c_str(), WIC_FLAGS_FORCE_RGB, nullptr, *image);
			LoadFromDDSFile(texFilePath.c_str(), DDS_FLAGS_NONE, nullptr, *image);
			std::pair<size_t, size_t> sizeKey = { image->GetMetadata().width, image->GetMetadata().height };
			imagesMap[sizeKey].push_back({ it->filePath, it->texId, image });
		}

		textureArraysViewsVec.clear();

		for (auto it = std::begin(imagesMap); it != end(imagesMap); it++) {
			std::pair<size_t, size_t> imageSize = it->first;
			std::vector<TexInfo> &texInfoVec = it->second;
			size_t numTextures = texInfoVec.size();


			std::vector<D3D11_SUBRESOURCE_DATA> subDataVec;

			for (auto texIt = std::begin(texInfoVec); texIt != end(texInfoVec); texIt++) {
				ScratchImage *image = texIt->image;

				// load each mipmap
				size_t numMipMaps = image->GetImageCount();

				for (size_t i = 0; i != numMipMaps; ++i) {
					D3D11_SUBRESOURCE_DATA subData;
					const Image *mipMap = image->GetImage(i, 0, 0);
					subData.pSysMem = mipMap->pixels;
					subData.SysMemPitch = (UINT)mipMap->rowPitch;
					subData.SysMemSlicePitch = (UINT)mipMap->slicePitch;
					subDataVec.push_back(subData);
				}
			}


			// Setup the description of the texture array.
			D3D11_TEXTURE2D_DESC textureDesc;
			ZeroMemory(&textureDesc, sizeof(textureDesc));

			textureDesc.Width = (UINT)imageSize.first;
			textureDesc.Height = (UINT)imageSize.second;
			textureDesc.MipLevels = 0;
			textureDesc.ArraySize = (UINT)numTextures;
			textureDesc.Format = (normalMaps) ? DXGI_FORMAT_R8G8B8A8_UNORM : DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			textureDesc.SampleDesc.Count = 1;
			textureDesc.SampleDesc.Quality = 0;
			textureDesc.Usage = D3D11_USAGE_DEFAULT;
			textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
			textureDesc.CPUAccessFlags = 0;
			textureDesc.MiscFlags = 0;// D3D11_RESOURCE_MISC_GENERATE_MIPS;

			// Create the texture array.
			ID3D11Texture2D *textureArray;
			HRESULT hr = device->CreateTexture2D(&textureDesc, &subDataVec[0], &textureArray);
			if (FAILED(hr))
			{
				throw D3DAppException("CreateTexture2D() failed.", hr);
			}

			/*
			UINT subRscIdx = 0;
			for (auto texIt = std::begin(texInfoVec); texIt != end(texInfoVec); texIt++) {
			ScratchImage *image = texIt->image;
			deviceContext->UpdateSubresource(textureArray, subRscIdx++, nullptr,
			image->GetPixels(), imageSize.first * 4, 0); // imageSize.first * imageSize.second * 4);
			}
			*/

			// Setup the shader resource view description.
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
			ZeroMemory(&srvDesc, sizeof(srvDesc));
			srvDesc.Format = textureDesc.Format;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
			srvDesc.Texture2DArray.ArraySize = (UINT)numTextures;
			srvDesc.Texture2DArray.FirstArraySlice = 0;
			srvDesc.Texture2DArray.MipLevels = -1;
			srvDesc.Texture2DArray.MostDetailedMip = 0;

			// Create the shader resource view for the texture array.
			ID3D11ShaderResourceView *textureArrayView;
			hr = device->CreateShaderResourceView(textureArray, &srvDesc, &textureArrayView);
			if (FAILED(hr))
			{
				throw D3DAppException("CreateShaderResourceView() failed.", hr);
			}

			textureArraysViewsVec.push_back(textureArrayView);

			if (!normalMaps) {
				uint16_t texArrayIndex = (uint16_t)(textureArraysViewsVec.size() - 1);
				uint16_t texIndexInTextureArray = 0;
				for (auto texIt = std::begin(texInfoVec); texIt != end(texInfoVec); texIt++) {

					mTexIDToTextureArrayRef[texIt->texId] = { texArrayIndex, texIndexInTextureArray };
					texIndexInTextureArray++;

					delete texIt->image;
					texIt->image = nullptr;
				}
			}
			else {
				for (auto texIt = std::begin(texInfoVec); texIt != end(texInfoVec); texIt++) {
					delete texIt->image;
					texIt->image = nullptr;
				}
			}

			ReleaseObject(textureArray); // keep only view reference

		}

		mapInfoVec.clear();
		mapInfoVec.shrink_to_fit();
	}

	BSPEngine::TextureItem::TextureItem()
	{
	}

	BSPEngine::TextureItem::~TextureItem()
	{
		ReleaseObject(mTexture);
		ReleaseObject(mTextureView);
	}

}


