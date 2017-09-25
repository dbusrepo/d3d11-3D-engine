#pragma once

#include "Common.h"
#include "DrawableComponent.h"

namespace Library
{
	class D3DApp;
	class CameraComponent;

	namespace BSPEngine {

		class BSPTree;
		class SceneMaterial;
		class TextureResources;
		class Collision;
		class Player;
	
		class Scene
		{

		public:
			Scene(D3DApp *pApp);
			~Scene();

			void Initialize();
			void Draw(const Timer &timer);

			bool Update(float TimeScale);

			bool UpdatePlayer(Player *pPlayer, float TimeScale, bool onlyTest);
			Player *GetPlayer() { return m_pPlayer; }
			
		private:

			struct InfoPlayerStart {
				bool isValid = false;
				float origin[3];
				float angles[3];
			};

			void SetupPlayer();
			bool LoadSceneFromFile(char * strFileName);
			bool LoadTextures(FILE *);
			bool LoadMeshes(FILE *);
			bool LoadDoors(FILE *);
			bool LoadPlayerInfo(FILE *);		
			bool LoadLights(FILE *);
			bool LoadClusterMaps();

			void Release();
			
			InfoPlayerStart m_infoPlayerStart;
			
			D3DApp *m_pApp;
			BSPTree *m_pSpatialTree;	
			TextureResources *m_pTextureResouces;
			Collision *m_pCollision;
			Player *m_pPlayer;
		};
		

	}

}