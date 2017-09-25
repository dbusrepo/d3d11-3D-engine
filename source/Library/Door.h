#pragma once

#include "BSPTree.h"

namespace Library
{
	namespace BSPEngine {

		//-----------------------------------------------------------------------------
		// Name : Door (Class)
		// Desc : 
		//-----------------------------------------------------------------------------
		class Door : public BrushEntity {
		public:
			Door();
			virtual ~Door();

			virtual void Init(size_t index, const XMFLOAT3 &bmin, const XMFLOAT3 &bmax, float distToOpen = 150.f);
			virtual bool CheckOpenCondition(const XMFLOAT3 &pos);

			void Update(float timeScale);
			bool BuildRenderData(BSPTree *pBspTree, DoorMaterial *pDoorMaterial);
			bool IsActive() { return m_state != CLOSE; }
			void Active() { 
				if (IsActive()) return;
				m_state = OPENING; 
			}

		protected:
			enum DoorState {
				CLOSE, 
				OPENING,
				OPEN,
				CLOSING
			};
			
			XMFLOAT3 computeBBoxCenter();
			void UpdateBspLeaves();

			virtual void StepOpen(float timeScale);
			virtual void StepClose(float timeScale);

			DoorState m_state;
			float m_distToOpen;
			
		};

		class TMoveDoor : public Door {
		public:
			TMoveDoor();
			~TMoveDoor();
			
			virtual void Init(size_t index, const XMFLOAT3 &bmin, const XMFLOAT3 &bmax, const XMFLOAT3 &moveVec, float speed, float distToOpen = 150.f);
			virtual bool CheckOpenCondition(const XMFLOAT3 &pos);
			
		private:

			virtual void StepOpen(float timeScale);
			virtual void StepClose(float timeScale);

			void UpdateBBox(const XMFLOAT3 &tVec);
			void UpdatePolygons(const XMFLOAT3 &tVec);

			XMFLOAT3 m_origCenter;
			//XMFLOAT3 m_curCenter;
			XMFLOAT3 m_moveVec;
			float m_tstep;
			float m_speed;
			float m_tdist;
		};

	}

}