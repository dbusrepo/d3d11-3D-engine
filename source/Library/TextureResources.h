#pragma once
#include "Common.h"
#include <DirectXTex.h>

namespace Library {

	namespace BSPEngine {

		struct TextureItem {
			TextureItem();
			~TextureItem();
			std::string texName;
			ID3D11Resource *mTexture = nullptr;
			ID3D11ShaderResourceView* mTextureView = nullptr;
		};

		struct TexInfo {
			std::wstring filePath;
			uint16_t texId;
			ScratchImage *image;
		};

		struct TexArrayRef {
			uint16_t mTexArrayIndex; // texture array index
			uint16_t mTexIndex; // texture index in the texture array
		};

		struct TextureArray {
			TextureArray() {}
			~TextureArray() {}
			std::pair<size_t, size_t> m_imageSize;
			ID3D11Resource *mTextureArray = nullptr;
			ID3D11ShaderResourceView* mTextureArrayView = nullptr;
		};

		class TextureResources {
		public:

			TextureResources();
			~TextureResources();
			
			void LoadTexture(ID3D11Device *device, const std::string &texName, const std::wstring &textureFileName, uint16_t texID);
			TextureItem *getTextureItem(uint16_t texID);

			void addTextureDiffuseMap(const std::wstring &textureFilePath, uint16_t texID);
			void createTextureArrayDiffuseMaps(ID3D11Device *device, ID3D11DeviceContext *deviceContext);
			void addTextureNormalMap(const std::wstring &textureFilePath, uint16_t texID);
			void createTextureArrayNormalMaps(ID3D11Device *device, ID3D11DeviceContext *deviceContext);
			void addLightMap(const std::wstring &LightMapFilePath);
			void createTextureArrayLightMaps(ID3D11Device *device, ID3D11DeviceContext *deviceContext);
			void addClusterMap(const std::wstring &ClusterMapFilePath);
			void createTextureArrayClusterMaps(ID3D11Device *device, ID3D11DeviceContext *deviceContext);

			uint16_t getTexArrayIndex(uint16_t texID) {
				return mTexIDToTextureArrayRef[texID].mTexArrayIndex;
			}
			uint16_t getTexIndexInTexArray(uint16_t texID) { 
				return mTexIDToTextureArrayRef[texID].mTexIndex;
			}

			ID3D11ShaderResourceView *getTextureArrayDiffuseView(uint16_t texArrayID) {
				return mTextureDiffuseViews[texArrayID];
			}
			ID3D11ShaderResourceView *getTextureArrayNormalView(uint16_t texArrayID) {
				return mTextureNormalArrayViews[texArrayID];
			}
			ID3D11ShaderResourceView *getLightMapsArrayView() {
				return mLightMapsArrayView;
			}
			ID3D11ShaderResourceView *getClusterMapsArrayView() {
				return mClustersMapArrayView;
			}
			XMFLOAT2 getClusterMapSize() {
				return m_clusterMapSize;
			}
		private:
			TextureResources(const TextureResources& rhs);
			TextureResources& operator=(const TextureResources& rhs);

			void createTextureArrays(ID3D11Device *device, ID3D11DeviceContext *deviceContext, std::vector<TexInfo> &mapInfoVec, std::vector<ID3D11ShaderResourceView *> &textureArraysViewsVec, bool normalMaps);

			std::map<uint16_t, TextureItem*> mTextureItems;

			std::map<uint16_t, TexArrayRef> mTexIDToTextureArrayRef;
			std::vector<ID3D11ShaderResourceView *> mTextureDiffuseViews;
			std::vector<ID3D11ShaderResourceView *> mTextureNormalArrayViews;
			std::vector<TexInfo> mDiffuseMapInfo;
			std::vector<TexInfo> mNormalMapInfo;
			std::vector<TexInfo> mLightMapInfo;
			std::vector<TexInfo> mClusterMapInfo;
			ID3D11ShaderResourceView *mLightMapsArrayView;
			ID3D11ShaderResourceView *mClustersMapArrayView;
			XMFLOAT2 m_clusterMapSize;
		};
	}
}