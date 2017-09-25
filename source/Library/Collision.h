#pragma once
#include "Common.h"

//-----------------------------------------------------------------------------
// Main Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : Collision (Class)
// Desc : The purpose of this class is to provide all support necessary for
//        geometric intersection testing, and their utility functions.
//        Several static functions are defined for public use for various
//        operations such as ray plane testing etc.
//-----------------------------------------------------------------------------
namespace Library
{

	namespace BSPEngine
	{
		class BSPTree;
		class BSPTreeLeaf;

		class Collision
		{
		public:
			// Enumerators
			enum CLASSIFYTYPE { CLASSIFY_ONPLANE = 0, CLASSIFY_BEHIND = 1, CLASSIFY_INFRONT = 2, CLASSIFY_SPANNING = 3 };

			typedef std::vector<BSPTreeLeaf*> LeafVector;

			struct CollIntersect
			{
				XMFLOAT3     NewCenter;      // The new sphere/ellipsoid center point
				XMFLOAT3     IntersectPoint; // The point on the exterior of the sphere/ellipsoid where contact happened
				XMFLOAT3     IntersectNormal;// The intersection normal (sliding plane)
				float        Interval;       // The Time of intersection (Centre + Velocity * Interval)
				ULONG        TriangleIndex;  // The index of the triangle we are intersecting
				//DynamicObject * pObject;   // If we hit a dynamic object, it will be stored here.
			};

			Collision();
			~Collision();
						
			void SetSpatialTree(BSPTree *pTree);
			bool CollideEllipsoid(const XMFLOAT3& Center, const XMFLOAT3& Radius, const XMFLOAT3& Velocity, XMFLOAT3& NewCenter, XMFLOAT3& NewIntegrationVelocity, XMFLOAT3& CollExtentsMin, XMFLOAT3& CollExtentsMax);
			bool EllipsoidIntersectScene(const XMFLOAT3& Center, const XMFLOAT3& Radius, const XMFLOAT3& Velocity, CollIntersect Intersections[], ULONG & IntersectionCount, bool bInputEllipsoidSpace = false, bool bReturnEllipsoidSpace = false);
			void CalculateEllipsoidBounds(const XMFLOAT3& Center, const XMFLOAT3& Radius, const XMFLOAT3& Velocity);
			ULONG EllipsoidIntersectTree(const XMFLOAT3& eCenter, const XMFLOAT3& Radius, const XMFLOAT3& InvRadius, const XMFLOAT3& eVelocity, float& eInterval, CollIntersect Intersections[], ULONG & IntersectionCount);
			void EllipsoidIntersectPolygon(const Polygon * pPoly, ULONG &NewIndex, ULONG &FirstIndex, const XMFLOAT3& eCenter, const XMFLOAT3& Radius, const XMFLOAT3& InvRadius, const XMFLOAT3& eVelocity, float& eInterval, CollIntersect Intersections[], ULONG & IntersectionCount);
			void CollectTreeLeaves();
			void Clear();

			//-------------------------------------------------------------------------
			// Public Static Functions for This Class.
			//-------------------------------------------------------------------------
			static bool         PointInTriangle(const XMFLOAT3& Point, const XMFLOAT3& v1, const XMFLOAT3& v2, const XMFLOAT3& v3);
			static bool         PointInTriangle(const XMFLOAT3& Point, const XMFLOAT3& v1, const XMFLOAT3& v2, const XMFLOAT3& v3, const XMFLOAT3& TriNormal);
			static bool         PointInAABB(const XMFLOAT3& Point, const XMFLOAT3& Min, const XMFLOAT3& Max, bool bIgnoreX = false, bool bIgnoreY = false, bool bIgnoreZ = false);

			static bool         RayIntersectPlane(const XMFLOAT3& Origin, const XMFLOAT3& Velocity, const XMFLOAT3& PlaneNormal, const XMFLOAT3& PlanePoint, float& t, bool BiDirectional = false);
			static bool         RayIntersectPlane(const XMFLOAT3& Origin, const XMFLOAT3& Velocity, const XMFLOAT3& PlaneNormal, float PlaneDistance, float& t, bool BiDirectional = false);
			static bool         RayIntersectTriangle(const XMFLOAT3& Origin, const XMFLOAT3& Velocity, const XMFLOAT3& v1, const XMFLOAT3& v2, const XMFLOAT3& v3, float& t, bool BiDirectional = false);
			static bool         RayIntersectTriangle(const XMFLOAT3& Origin, const XMFLOAT3& Velocity, const XMFLOAT3& v1, const XMFLOAT3& v2, const XMFLOAT3& v3, const XMFLOAT3& TriNormal, float& t, bool BiDirectional = false);
			static bool         RayIntersectAABB(const XMFLOAT3& Origin, const XMFLOAT3& Velocity, const XMFLOAT3& Min, const XMFLOAT3& Max, float& t, bool bIgnoreX = false, bool bIgnoreY = false, bool bIgnoreZ = false);

			static bool         SphereIntersectPlane(const XMFLOAT3& Center, float Radius, const XMFLOAT3& Velocity, const XMFLOAT3& PlaneNormal, const XMFLOAT3& PlanePoint, float& tMax);
			static bool         SphereIntersectLineSegment(const XMFLOAT3& Center, float Radius, const XMFLOAT3& Velocity, const XMFLOAT3& v1, const XMFLOAT3& v2, float& tMax, XMFLOAT3& CollisionNormal);
			static bool         SphereIntersectPoint(const XMFLOAT3& Center, float Radius, const XMFLOAT3& Velocity, const XMFLOAT3& Point, float& tMax, XMFLOAT3& CollisionNormal);
			static bool         SphereIntersectTriangle(const XMFLOAT3& Center, float Radius, const XMFLOAT3& Velocity, const XMFLOAT3& v1, const XMFLOAT3& v2, const XMFLOAT3& v3, const XMFLOAT3& TriNormal, float& tMax, XMFLOAT3& CollisionNormal);

			static bool         AABBIntersectAABB(const XMFLOAT3& Min1, const XMFLOAT3& Max1, const XMFLOAT3& Min2, const XMFLOAT3& Max2, bool bIgnoreX = false, bool bIgnoreY = false, bool bIgnoreZ = false);
			static bool         AABBIntersectAABB(bool & bContained, const XMFLOAT3& Min1, const XMFLOAT3& Max1, const XMFLOAT3& Min2, const XMFLOAT3& Max2, bool bIgnoreX = false, bool bIgnoreY = false, bool bIgnoreZ = false);

			static CLASSIFYTYPE PointClassifyPlane(const XMFLOAT3& Point, const XMFLOAT3& PlaneNormal, float PlaneDistance);
			static CLASSIFYTYPE RayClassifyPlane(const XMFLOAT3& Origin, const XMFLOAT3& Velocity, const XMFLOAT3& PlaneNormal, float PlaneDistance);
			static CLASSIFYTYPE PolyClassifyPlane(void * pVertices, ULONG VertexCount, ULONG Stride, const XMFLOAT3& PlaneNormal, float PlaneDistance);
			static CLASSIFYTYPE AABBClassifyPlane(const XMFLOAT3& Min, const XMFLOAT3& Max, const XMFLOAT3& PlaneNormal, float PlaneDistance);


		private:
			//-------------------------------------------------------------------------
			// Private Static Functions for This Class.
			//-------------------------------------------------------------------------
			static bool               SolveCollision(float a, float b, float c, float& t);
			inline static XMFLOAT3    Vec3VecScale(const XMFLOAT3& v1, const XMFLOAT3& v2) { return XMFLOAT3(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z); }

			//-------------------------------------------------------------------------
			// Private Variables for This Class.
			//-------------------------------------------------------------------------
			BSPTree *m_pSpatialTree; // The spatial tree we can use for our broadphase
			LeafVector m_TreeLeafList;  // List of leaves we are intersecting in our spatial tree

			CollIntersect *m_pIntersections; // Internal buffer for storing intersection information
			USHORT         m_nMaxIntersections;    // The total number of intersections which we should record
			USHORT         m_nMaxIterations;       // The maximum number of collision test iterations we should try before failing

			XMFLOAT3  m_vecEllipsoidMin;
			XMFLOAT3  m_vecEllipsoidMax;
		};

	}

}