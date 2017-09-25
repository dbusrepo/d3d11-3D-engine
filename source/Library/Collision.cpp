#include "BSPTree.h"
#include "Polygon.h"
#include "Door.h"
#include "Collision.h"

Library::BSPEngine::Collision::Collision()
{
	// Setup any required values
	m_nMaxIntersections = 100;
	m_pSpatialTree = nullptr;
	m_nMaxIterations = 10; // 30
	// Allocate memory for intersections
	m_pIntersections = new CollIntersect[m_nMaxIntersections];
}

Library::BSPEngine::Collision::~Collision()
{
	// Release memory
	if (m_pIntersections) delete[]m_pIntersections;

	// Clear vars
	m_pIntersections = NULL;

	// Clear all our data (releases dynamic objects)
	Clear();
}

void Library::BSPEngine::Collision::SetSpatialTree(BSPTree * pTree)
{
	// Store the tree
	m_pSpatialTree = pTree;
}

//-----------------------------------------------------------------------------
// Name : CollideEllipsoid ()
// Desc : This is the main collision detection AND response function for
//        dealing with the interaction of scene and ellipse.
// Note : For ease of understanding, all variables used in this function which
//        begin with a lower case 'e' (i.e. eNormal) denote that the values
//        contained within it are described in ellipsoid space.
//-----------------------------------------------------------------------------
bool Library::BSPEngine::Collision::CollideEllipsoid(const XMFLOAT3 & Center, const XMFLOAT3 & Radius, const XMFLOAT3 & Velocity, XMFLOAT3 & NewCenter, XMFLOAT3 & NewIntegrationVelocity, XMFLOAT3 & CollExtentsMin, XMFLOAT3 & CollExtentsMax)
{
	XMFLOAT3 vecOutputPos, vecOutputVelocity, InvRadius, vecNormal, vecExtents;
	XMFLOAT3 eVelocity, eInputVelocity, eFrom, eTo, vecNewCenter, vecIntersectPoint;
	XMFLOAT3 vecFrom, vecEndPoint, vecAdjust;
	ULONG       IntersectionCount, i;
	//float       fDistance, fDot;
	bool        bHit = false;

	// Default the output to move in clear space, regardless of whether we 
	// return true or not (we're going to alter this if we register a hit)
	// Note : We store this in a separate variable in case the user passed
	//        a reference to the same vector to both vecPos and vecNewPos
	
	//vecOutputPos = Center + Velocity;
	XMStoreFloat3(&vecOutputPos,
		XMVectorAdd(XMLoadFloat3(&Center), XMLoadFloat3(&Velocity)));
	vecOutputVelocity = Velocity;

	// Default our intersection extents
	CollExtentsMin.x = Radius.x;
	CollExtentsMin.y = Radius.y;
	CollExtentsMin.z = Radius.z;
	CollExtentsMax.x = -Radius.x;
	CollExtentsMax.y = -Radius.y;
	CollExtentsMax.z = -Radius.z;

	// Store ellipsoid transformation values
	InvRadius = XMFLOAT3(1.0f / Radius.x, 1.0f / Radius.y, 1.0f / Radius.z);

	// Calculate values in ellipsoid space
	eVelocity = Vec3VecScale(Velocity, InvRadius);
	eFrom = Vec3VecScale(Center, InvRadius);
	//eTo = eFrom + eVelocity;
	XMStoreFloat3(&eTo,
		XMVectorAdd(XMLoadFloat3(&eFrom), XMLoadFloat3(&eVelocity)));

	// Store the input velocity for jump testing
	eInputVelocity = eVelocity;

	// Keep testing until we hit our max iteration limit
	for (i = 0; i < m_nMaxIterations; ++i)
	{
		// Break out if our velocity is too small (optional but sometimes beneficial)
		//if ( D3DXVec3Length( &eVelocity ) < 1e-5f ) break;

		// Attempt scene intersection
		// Note : we are working totally in ellipsoid space at this point, so we specify that both
		// our input values are in ellipsoid space, and we would like our output values in ellipsoid space also.
		if (EllipsoidIntersectScene(eFrom, Radius, eVelocity, m_pIntersections, IntersectionCount, true, true))
		{
			// Retrieve the first collision intersections
			CollIntersect & FirstIntersect = m_pIntersections[0];

			// Calculate the WORLD space sliding normal
			//D3DXVec3Normalize(&vecNormal, &Vec3VecScale(FirstIntersect.IntersectNormal, InvRadius));
			XMStoreFloat3(&vecNormal,
				XMVector3Normalize(XMLoadFloat3(&Vec3VecScale(FirstIntersect.IntersectNormal, InvRadius))));

			// Generate slide velocity
			//fDistance = D3DXVec3Dot(&vecOutputVelocity, &vecNormal);
			//vecOutputVelocity -= vecNormal * fDistance;
			XMStoreFloat3(&vecOutputVelocity, 
				XMVectorNegativeMultiplySubtract(
					XMLoadFloat3(&vecNormal),
					XMVector3Dot(XMLoadFloat3(&vecOutputVelocity), XMLoadFloat3(&vecNormal)),
					XMLoadFloat3(&vecOutputVelocity)
				)
			);
				
			// Set the sphere position to the collision position for the next iteration of testing
			eFrom = FirstIntersect.NewCenter;

			// Project the end of the velocity vector onto the collision plane (at the sphere center)
			//fDistance = D3DXVec3Dot(&(eTo - FirstIntersect.NewCenter), &FirstIntersect.IntersectNormal);
			//eTo -= FirstIntersect.IntersectNormal * fDistance;
			XMStoreFloat3(&eTo,
				XMVectorNegativeMultiplySubtract(
					XMLoadFloat3(&FirstIntersect.IntersectNormal),
					XMVector3Dot(XMVectorSubtract(XMLoadFloat3(&eTo), XMLoadFloat3(&FirstIntersect.NewCenter)),
								 XMLoadFloat3(&FirstIntersect.IntersectNormal)),
					XMLoadFloat3(&eTo)
				)
			);

			// Transform the sphere position back into world space, and recalculate the intersect point
			// (we recalculate because we want our collision extents to be based on the slope of intersections
			//  given in world space, rather than ellipsoid space. This gives us a better quality slope test later).
			vecNewCenter = Vec3VecScale(FirstIntersect.NewCenter, Radius);
			//vecIntersectPoint = vecNewCenter - Vec3VecScale(vecNormal, Radius);
			XMStoreFloat3(&vecIntersectPoint,
				XMVectorSubtract(XMLoadFloat3(&vecNewCenter), XMLoadFloat3(&Vec3VecScale(vecNormal, Radius))));

			// Calculate the min / max collision extents around the ellipsoid center
			//vecExtents = vecIntersectPoint - vecNewCenter;
			XMStoreFloat3(&vecExtents,
				XMVectorSubtract(XMLoadFloat3(&vecIntersectPoint), XMLoadFloat3(&vecNewCenter)));
			if (vecExtents.x > CollExtentsMax.x) CollExtentsMax.x = vecExtents.x;
			if (vecExtents.y > CollExtentsMax.y) CollExtentsMax.y = vecExtents.y;
			if (vecExtents.z > CollExtentsMax.z) CollExtentsMax.z = vecExtents.z;
			if (vecExtents.x < CollExtentsMin.x) CollExtentsMin.x = vecExtents.x;
			if (vecExtents.y < CollExtentsMin.y) CollExtentsMin.y = vecExtents.y;
			if (vecExtents.z < CollExtentsMin.z) CollExtentsMin.z = vecExtents.z;

			// Update the velocity value
			//eVelocity = eTo - eFrom;
			XMStoreFloat3(&eVelocity,
				XMVectorSubtract(XMLoadFloat3(&eTo), XMLoadFloat3(&eFrom)));

			// We hit something
			bHit = true;

			// Filter 'Impulse' jumps
			//if (D3DXVec3Dot(&eVelocity, &eInputVelocity) < 0) { eTo = eFrom; break; }
			if (XMVectorGetX(XMVector3Dot(XMLoadFloat3(&eVelocity), XMLoadFloat3(&eInputVelocity))) < 0.0f)
			{
				eTo = eFrom;
				break;
			}

		} // End if we got some intersections
		else
		{
			// We found no collisions, so break out of the loop
			break;

		} // End if no collision

		// Increment the app counter so that our polygon testing is reset
		//GetGameApp()->IncrementAppCounter();

	} // Next Iteration

	  // Did we register any intersection at all?
	if (bHit)
	{
		// Increment the app counter so that our polygon testing is reset
		//GetGameApp()->IncrementAppCounter();

		// Did we finish neatly or not?
		if (i < m_nMaxIterations)
		{
			// Return our final position in world space
			vecOutputPos = Vec3VecScale(eTo, Radius);

		} // End if in clear space
		else
		{
			// Just find the closest intersection
			eFrom = Vec3VecScale(Center, InvRadius);

			// Attempt to intersect the scene
			IntersectionCount = 0;
			if (EllipsoidIntersectScene(eFrom, Radius, eInputVelocity, m_pIntersections, IntersectionCount, true, true))
			{
				// Retrieve the intersection point in clear space, but ensure that we undo the epsilon
				// shift we apply during the above call. This ensures that when we are stuck between two
				// planes, we don't slowly push our way through.
				//vecOutputPos = m_pIntersections[0].NewCenter - (m_pIntersections[0].IntersectNormal * 1e-3f);
				XMStoreFloat3(&vecOutputPos,
					XMVectorNegativeMultiplySubtract(
						XMLoadFloat3(&m_pIntersections[0].IntersectNormal),
						XMVectorReplicate(1e-3f),
						XMLoadFloat3(&m_pIntersections[0].NewCenter)
					)
				);

				// Scale back into world space
				vecOutputPos = Vec3VecScale(vecOutputPos, Radius);

			} // End if collision
			else
			{
				// Don't move at all, stay where we were
				vecOutputPos = Center;

			} // End if no collision

		} // End if bad situation

	} // End if intersection found

	  // Store the resulting output values
	NewCenter = vecOutputPos;
	NewIntegrationVelocity = vecOutputVelocity;

	// Return hit code
	return bHit;
}

//-----------------------------------------------------------------------------
// Name : EllipsoidIntersectScene ()
// Desc : Test for collision against the database using the ellipsoid specified
// Note : For ease of understanding, all variables used in this function which
//        begin with a lower case 'e' (i.e. eNormal) denote that the values
//        contained within it are described in ellipsoid space.
//-----------------------------------------------------------------------------
bool Library::BSPEngine::Collision::EllipsoidIntersectScene(const XMFLOAT3 & Center, const XMFLOAT3 & Radius, const XMFLOAT3 & Velocity, CollIntersect Intersections[], ULONG & IntersectionCount, bool bInputEllipsoidSpace, bool bReturnEllipsoidSpace)
{
	XMFLOAT3 eCenter, eVelocity, eAdjust, vecEndPoint, InvRadius;
	float       eInterval;
	ULONG       i;

	// Calculate the reciprocal radius to prevent the many divides we would need otherwise
	InvRadius = XMFLOAT3(1.0f / Radius.x, 1.0f / Radius.y, 1.0f / Radius.z);

	// Convert the values specified into ellipsoid space if required
	if (!bInputEllipsoidSpace)
	{
		eCenter = Vec3VecScale(Center, InvRadius);
		eVelocity = Vec3VecScale(Velocity, InvRadius);

	} // End if the input values were not in ellipsoid space
	else
	{
		eCenter = Center;
		eVelocity = Velocity;

	} // End if the input values are already in ellipsoid space

	  // Reset ellipsoid space interval to maximum
	eInterval = 1.0f;

	// Reset initial intersection count to 0 to save the caller having to do this.
	IntersectionCount = 0;

	// Calculate the bounding box of the ellipsoid
	XMFLOAT3 vecCenter = Vec3VecScale(eCenter, Radius);
	XMFLOAT3 vecVelocity = Vec3VecScale(eVelocity, Radius);
	CalculateEllipsoidBounds(vecCenter, Radius, vecVelocity);

	// Collide against tree if available
	if (m_pSpatialTree)
	{
		// Collect the leaf list if necessary
		CollectTreeLeaves();
		EllipsoidIntersectTree(eCenter, Radius, InvRadius, eVelocity, eInterval, Intersections, IntersectionCount);

	} // Next Tree
	
	  // If we were requested to return the values in normal space
	  // then we must take the values back out of ellipsoid space here
	if (!bReturnEllipsoidSpace)
	{
		// For each intersection found
		for (i = 0; i < IntersectionCount; ++i)
		{
			// Transform the new center position and intersection point
			Intersections[i].NewCenter = Vec3VecScale(Intersections[i].NewCenter, Radius);
			Intersections[i].IntersectPoint = Vec3VecScale(Intersections[i].IntersectPoint, Radius);

			// Transform the normal (again we do this in the opposite way to a coordinate)
			XMFLOAT3 Normal = Vec3VecScale(Intersections[i].IntersectNormal, InvRadius);
			//D3DXVec3Normalize(&Normal, &Normal);
			XMStoreFloat3(&Normal,
				XMVector3Normalize(XMLoadFloat3(&Normal)));

			// Store the transformed normal
			Intersections[i].IntersectNormal = Normal;

		} // Next Intersection

	} // End if !bReturnEllipsoidSpace

	  // Return hit.
	return (IntersectionCount > 0);
}

//-----------------------------------------------------------------------------
// Name : CalculateEllipsoidBounds () (Private)
// Desc : Calculate the bounding box of the ellipsoid prior to processing.
//-----------------------------------------------------------------------------
void Library::BSPEngine::Collision::CalculateEllipsoidBounds(const XMFLOAT3 & Center, const XMFLOAT3 & Radius, const XMFLOAT3 & Velocity)
{
	float       fLargestExtent;

	XMFLOAT3& vecMin = m_vecEllipsoidMin;
	XMFLOAT3& vecMax = m_vecEllipsoidMax;

	// Find the largest extent of our ellipsoid including the velocity length all around.
	fLargestExtent = Radius.x;
	if (Radius.y > fLargestExtent) fLargestExtent = Radius.y;
	if (Radius.z > fLargestExtent) fLargestExtent = Radius.z;

	// Reset the bounding box values
	vecMin = XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX);
	vecMax = XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	// Calculate the bounding box extents of where the ellipsoid currently 
	// is, and the position it will be moving to.
	if (Center.x + fLargestExtent > vecMax.x) vecMax.x = Center.x + fLargestExtent;
	if (Center.y + fLargestExtent > vecMax.y) vecMax.y = Center.y + fLargestExtent;
	if (Center.z + fLargestExtent > vecMax.z) vecMax.z = Center.z + fLargestExtent;

	if (Center.x - fLargestExtent < vecMin.x) vecMin.x = Center.x - fLargestExtent;
	if (Center.y - fLargestExtent < vecMin.y) vecMin.y = Center.y - fLargestExtent;
	if (Center.z - fLargestExtent < vecMin.z) vecMin.z = Center.z - fLargestExtent;

	if (Center.x + Velocity.x + fLargestExtent > vecMax.x) vecMax.x = Center.x + Velocity.x + fLargestExtent;
	if (Center.y + Velocity.y + fLargestExtent > vecMax.y) vecMax.y = Center.y + Velocity.y + fLargestExtent;
	if (Center.z + Velocity.z + fLargestExtent > vecMax.z) vecMax.z = Center.z + Velocity.z + fLargestExtent;

	if (Center.x + Velocity.x - fLargestExtent < vecMin.x) vecMin.x = Center.x + Velocity.x - fLargestExtent;
	if (Center.y + Velocity.y - fLargestExtent < vecMin.y) vecMin.y = Center.y + Velocity.y - fLargestExtent;
	if (Center.z + Velocity.z - fLargestExtent < vecMin.z) vecMin.z = Center.z + Velocity.z - fLargestExtent;

	// Add Tolerance values
	//vecMin -= XMFLOAT3(1.0f, 1.0f, 1.0f);
	vecMin.x -= 1.0f;
	vecMin.y -= 1.0f;
	vecMin.z -= 1.0f;
	//vecMax += XMFLOAT3(1.0f, 1.0f, 1.0f);
	vecMax.x += 1.0f;
	vecMax.y += 1.0f;
	vecMax.z += 1.0f;
}

//-----------------------------------------------------------------------------
// Name : EllipsoidIntersectTree () (Private)
// Desc : The internal function which actually tests for intersection against
//        the spatial tree.
//-----------------------------------------------------------------------------
ULONG Library::BSPEngine::Collision::EllipsoidIntersectTree(const XMFLOAT3 & eCenter, const XMFLOAT3 & Radius, const XMFLOAT3 & InvRadius, const XMFLOAT3 & eVelocity, float & eInterval, CollIntersect Intersections[], ULONG & IntersectionCount)
{
	ULONG       NewIndex, FirstIndex;	
	static BSPTree::DoorVector doorsCollision;
	doorsCollision.clear();

	// FirstIndex tracks the first item to be added the intersection list.
	FirstIndex = IntersectionCount;

	// Extract the current application counter to ensure duplicate polys are not tested
	// multiple times in the same iteration / frame
	//ULONG CurrentCounter = GetGameApp()->GetAppCounter();
	for (auto leafIt = begin(m_TreeLeafList); leafIt != end(m_TreeLeafList); ++leafIt)
	{
		BSPTreeLeaf * pLeaf = *leafIt;
		if (!pLeaf) continue;
	
		// Loop through each polygon
		for (size_t i = 0; i < pLeaf->GetPolygonCount(); ++i)
		{
			// Get the polygon
			Polygon * pPoly = pLeaf->GetPolygon(i);
			//if (!pPoly) continue;

			EllipsoidIntersectPolygon(pPoly, NewIndex, FirstIndex, eCenter, Radius, InvRadius, eVelocity, eInterval, Intersections, IntersectionCount);

		} // Next Polygon

		for (auto it = std::begin(pLeaf->m_doorsIndices); it != std::end(pLeaf->m_doorsIndices); ++it) 
		{
			size_t doorIdx = *it;
			Door *door = pLeaf->m_pTree->getDoor(doorIdx);
			if (!door->IsMarked()) {
				door->SetMarked(true);
				doorsCollision.push_back(door);
			}
		}
	
	} // Next Leaf

	for (size_t i = 0; i != doorsCollision.size(); ++i) {
		Door *door = doorsCollision[i];
		door->SetMarked(false);
		
		for (size_t i = 0; i < door->GetPolygonCount(); ++i)
		{
			// Get the polygon
			Polygon * pPoly = door->GetPolygon(i);
			//if (!pPoly) continue;

			EllipsoidIntersectPolygon(pPoly, NewIndex, FirstIndex, eCenter, Radius, InvRadius, eVelocity, eInterval, Intersections, IntersectionCount);
		}
	}

	// Return hit.
	return FirstIndex;

}

void Library::BSPEngine::Collision::EllipsoidIntersectPolygon(const Polygon * pPoly, ULONG &NewIndex, ULONG &FirstIndex, const XMFLOAT3 & eCenter, const XMFLOAT3 & Radius, const XMFLOAT3 & InvRadius, const XMFLOAT3 & eVelocity, float & eInterval, CollIntersect Intersections[], ULONG & IntersectionCount)
{
	XMFLOAT3 eIntersectNormal, eNewCenter;
	XMFLOAT3 ePoints[3];
	bool AddToList;

	// Skip if this poly has already been processed on this iteration
	//if (pPoly->m_nAppCounter == CurrentCounter) continue;
	//pPoly->m_nAppCounter = CurrentCounter;

	// Are we roughly intersecting the polygon?
	if (!AABBIntersectAABB(m_vecEllipsoidMin, m_vecEllipsoidMax, pPoly->m_vecBoundsMin, pPoly->m_vecBoundsMax)) return;

	// Transform normal and normalize (Note how we do not use InvRadius for the normal)
	XMFLOAT3 eNormal = Vec3VecScale(pPoly->m_normal, Radius);
	//D3DXVec3Normalize(&eNormal, &eNormal);
	XMStoreFloat3(&eNormal,
		XMVector3Normalize(XMLoadFloat3(&eNormal)));

	// For each triangle
	for (int j = 0; j < pPoly->m_nVertexCount - 2; ++j)
	{
		// Get points and transform into ellipsoid space
		ePoints[0] = Vec3VecScale((XMFLOAT3&)pPoly->m_pVertex[0], InvRadius);
		ePoints[1] = Vec3VecScale((XMFLOAT3&)pPoly->m_pVertex[j + 1], InvRadius);
		ePoints[2] = Vec3VecScale((XMFLOAT3&)pPoly->m_pVertex[j + 2], InvRadius);

		// Test for intersection with a unit sphere and the ellipsoid space triangle
		if (SphereIntersectTriangle(eCenter, 1.0f, eVelocity, ePoints[0], ePoints[1], ePoints[2], eNormal, eInterval, eIntersectNormal))
		{
			// Calculate our new sphere center at the point of intersection
			if (eInterval > 0) {
				//eNewCenter = eCenter + (eVelocity * eInterval);
				XMStoreFloat3(&eNewCenter,
					XMVectorMultiplyAdd(XMLoadFloat3(&eVelocity),
						XMVectorReplicate(eInterval),
						XMLoadFloat3(&eCenter)));
			}
			else {
				//eNewCenter = eCenter - (eIntersectNormal * eInterval);
				XMStoreFloat3(&eNewCenter,
					XMVectorMultiplyAdd(XMLoadFloat3(&eIntersectNormal),
						XMVectorReplicate(-eInterval),
						XMLoadFloat3(&eCenter)));
			}

			// Where in the array should it go?
			AddToList = false;
			if (IntersectionCount == 0 || eInterval < Intersections[0].Interval)
			{
				// We either have nothing in the array yet, or the new intersection is closer to us
				AddToList = true;
				NewIndex = 0;
				IntersectionCount = 1;

				// Reset, we've cleared the list
				FirstIndex = 0;

			} // End if overwrite existing intersections
			else if (fabsf(eInterval - Intersections[0].Interval) < 1e-5f)
			{
				// It has the same interval as those in our list already, append to 
				// the end unless we've already reached our limit
				if (IntersectionCount < m_nMaxIntersections)
				{
					AddToList = true;
					NewIndex = IntersectionCount;
					IntersectionCount++;

				} // End if we have room to store more

			} // End if the same interval

			  // Add to the list?
			if (AddToList)
			{
				XMFLOAT3 newCenter;
				// Intersections[NewIndex].NewCenter = eNewCenter + (eIntersectNormal * 1e-3f);
				XMStoreFloat3(&newCenter,
					XMVectorMultiplyAdd(XMLoadFloat3(&eIntersectNormal),
						XMVectorReplicate(1e-3f),
						XMLoadFloat3(&eNewCenter)));

				XMFLOAT3 intersectPoint;
				//Intersections[NewIndex].IntersectPoint = eNewCenter - eIntersectNormal;
				XMStoreFloat3(&intersectPoint,
					XMVectorSubtract(XMLoadFloat3(&eNewCenter), XMLoadFloat3(&eIntersectNormal)));

				Intersections[NewIndex].Interval = eInterval;
				Intersections[NewIndex].NewCenter = newCenter; // Push back from the plane slightly to improve accuracy
				Intersections[NewIndex].IntersectPoint = intersectPoint; // The intersection point on the surface of the sphere (and triangle)
				Intersections[NewIndex].IntersectNormal = eIntersectNormal;
				//Intersections[NewIndex].TriangleIndex = 0;
				//Intersections[NewIndex].pObject = NULL;

			} // End if we are inserting in our list

		} // End if collided

	} // Next Triangle
}

void Library::BSPEngine::Collision::CollectTreeLeaves()
{
	// Clear the previous leaf list
	m_TreeLeafList.clear();

	// Collect any leaves we're intersecting
	m_pSpatialTree->CollectLeavesAABB(m_TreeLeafList, m_vecEllipsoidMin, m_vecEllipsoidMax);
}

void Library::BSPEngine::Collision::Clear()
{
    // Empty our STL containers.
	m_TreeLeafList.clear();

	// Reset any variables
	m_pSpatialTree = NULL;
}

//-----------------------------------------------------------------------------
// Name : PointInTriangle () (Static, Overload)
// Desc : Test to see if a point falls within the bounds of a triangle.
// Note : This is an overload function for cases where no normal is provided.
//-----------------------------------------------------------------------------
bool Library::BSPEngine::Collision::PointInTriangle(const XMFLOAT3 & Point, const XMFLOAT3 & v1, const XMFLOAT3 & v2, const XMFLOAT3 & v3)
{
	XMFLOAT3 normal;

	// Generate the triangle normal
	//D3DXVec3Cross(&Normal, &(v2 - v1), &(v3 - v1));
	//D3DXVec3Normalize(&Normal, &Normal);
	XMStoreFloat3(&normal,
		XMVector3Normalize(
			XMVector3Cross(
				XMVectorSubtract(XMLoadFloat3(&v2), XMLoadFloat3(&v1)),
				XMVectorSubtract(XMLoadFloat3(&v3), XMLoadFloat3(&v1))
			)
		)
	);

	// Pass through to standard function
	return Library::BSPEngine::Collision::PointInTriangle(Point, v1, v2, v3, normal);
}

//-----------------------------------------------------------------------------
// Name : PointInTriangle () (Static)
// Desc : Test to see if a point falls within the bounds of a triangle.
//-----------------------------------------------------------------------------
bool Library::BSPEngine::Collision::PointInTriangle(const XMFLOAT3 & Point, const XMFLOAT3 & v1, const XMFLOAT3 & v2, const XMFLOAT3 & v3, const XMFLOAT3 & TriNormal)
{
	// First edge
	//Edge = v2 - v1;
	//Direction = v1 - Point;
	//D3DXVec3Cross(&EdgeNormal, &Edge, &TriNormal);
	// In front of edge?
	//if (D3DXVec3Dot(&Direction, &EdgeNormal) < 0.0f) return false;
	if (XMVectorGetX(
		XMVector3Dot(
			XMVectorSubtract(XMLoadFloat3(&v1), XMLoadFloat3(&Point)),
			XMVector3Cross(
				XMVectorSubtract(XMLoadFloat3(&v2), XMLoadFloat3(&v1)),
				XMLoadFloat3(&TriNormal))
		)
	) < 0.0f) return false;
		
	// Second edge
	//Edge = v3 - v2;
	//Direction = v2 - Point;
	//D3DXVec3Cross(&EdgeNormal, &Edge, &TriNormal);
	// In front of edge?
	//if (D3DXVec3Dot(&Direction, &EdgeNormal) < 0.0f) return false;
	if (XMVectorGetX(
		XMVector3Dot(
			XMVectorSubtract(XMLoadFloat3(&v2), XMLoadFloat3(&Point)),
			XMVector3Cross(
				XMVectorSubtract(XMLoadFloat3(&v3), XMLoadFloat3(&v2)),
				XMLoadFloat3(&TriNormal))
		)
	) < 0.0f) return false;

	// Third edge
	//Edge = v1 - v3;
	//Direction = v3 - Point;
	//D3DXVec3Cross(&EdgeNormal, &Edge, &TriNormal);
	// In front of edge?
	//if (D3DXVec3Dot(&Direction, &EdgeNormal) < 0.0f) return false;
	if (XMVectorGetX(
		XMVector3Dot(
			XMVectorSubtract(XMLoadFloat3(&v3), XMLoadFloat3(&Point)),
			XMVector3Cross(
				XMVectorSubtract(XMLoadFloat3(&v1), XMLoadFloat3(&v3)),
				XMLoadFloat3(&TriNormal))
		)
	) < 0.0f) return false;

	// We are behind all planes
	return true;
}

bool Library::BSPEngine::Collision::PointInAABB(const XMFLOAT3 & Point, const XMFLOAT3 & Min, const XMFLOAT3 & Max, bool bIgnoreX, bool bIgnoreY, bool bIgnoreZ)
{
	// Does the point fall outside any of the AABB planes?
	if (!bIgnoreX && (Point.x < Min.x || Point.x > Max.x)) return false;
	if (!bIgnoreY && (Point.y < Min.y || Point.y > Max.y)) return false;
	if (!bIgnoreZ && (Point.z < Min.z || Point.z > Max.z)) return false;

	// We are behind all planes
	return true;
}

//-----------------------------------------------------------------------------
// Name : RayIntersectPlane () (Static)
// Desc : Determine if the specified ray intersects the plane, and if so
//        at what 't' value.
// Form : 't = ((P - O) . N) / (V . N)' where:
//        P = Ray Origin, O = Point on Plane, V = Ray Velocity, N = Plane Normal
//-----------------------------------------------------------------------------
bool Library::BSPEngine::Collision::RayIntersectPlane(const XMFLOAT3 & Origin, const XMFLOAT3 & Velocity, const XMFLOAT3 & PlaneNormal, const XMFLOAT3 & PlanePoint, float & t, bool BiDirectional)
{
	// Get the length of the 'adjacent' side of the virtual triangle formed
	// by the velocity and normal.
	//float ProjRayLength = D3DXVec3Dot(&Velocity, &PlaneNormal);
	float ProjRayLength = XMVectorGetX(XMVector3Dot(XMLoadFloat3(&Velocity), XMLoadFloat3(&PlaneNormal)));

	// If they are pointing in the same direction, we're can't possibly be colliding (i.e.
	// the dot product returned a positive value, with epsilon testing ). Remember that
	// our Velocity and Plane normal vectors are pointing towards one another in the case
	// where the two can be colliding (i.e. cos(a) >= -1 and <= 0).
	if (BiDirectional == false && ProjRayLength > -1e-5f) return false;

	// Since the interval (0 - 1) for intersection will be the same regardless of the distance
	// to the plane along either the hypotenuse OR the adjacent side of the triangle (bearing
	// in mind that we do 'distance / length_of_edge', and the length of the adjacent side is
	// relative to the length of the hypotenuse), we can just calculate the distance to the plane
	// the 'quick and easy way'(tm) ('shortest distance to' or 'distance along the negative plane normal')
	//float Distance = D3DXVec3Dot(&(Origin - PlanePoint), &PlaneNormal);

	float Distance = XMVectorGetX(XMVector3Dot(XMVectorSubtract(XMLoadFloat3(&Origin), XMLoadFloat3(&PlanePoint)),
										       XMLoadFloat3(&PlaneNormal)));

	// Calculate the actual interval (Distance along the adjacent side / length of adjacent side).
	t = Distance / -ProjRayLength;

	// Outside our valid range? If yes, return no collide.
	if (t < 0.0f || t > 1.0f) return false;

	// We're intersecting
	return true;
}

//-----------------------------------------------------------------------------
// Name : RayIntersectPlane () (Static)
// Desc : Overload of previous function, accepting a plane distance instead.
//-----------------------------------------------------------------------------
bool Library::BSPEngine::Collision::RayIntersectPlane(const XMFLOAT3 & Origin, const XMFLOAT3 & Velocity, const XMFLOAT3 & PlaneNormal, float PlaneDistance, float & t, bool BiDirectional)
{
	// Get the length of the 'adjacent' side of the virtual triangle formed
	// by the velocity and normal.
	//float ProjRayLength = D3DXVec3Dot(&Velocity, &PlaneNormal);
	float ProjRayLength = XMVectorGetX(XMVector3Dot(XMLoadFloat3(&Velocity), XMLoadFloat3(&PlaneNormal)));

	if (BiDirectional == false && ProjRayLength > -1e-5f) return false;

	// Calculate distance to plane along it's normal
	//float Distance = D3DXVec3Dot(&Origin, &PlaneNormal) + PlaneDistance;
	float Distance = XMVectorGetX(XMVector3Dot(XMLoadFloat3(&Origin), XMLoadFloat3(&PlaneNormal)))
						+ PlaneDistance;

	// Calculate the actual interval (Distance along the adjacent side / length of adjacent side).
	t = Distance / -ProjRayLength;

	// Outside our valid range? If yes, return no collide.
	if (t < 0.0f || t > 1.0f) return false;

	// We're intersecting
	return true;
}

//-----------------------------------------------------------------------------
// Name : RayIntersectTriangle () (Static, Overload)
// Desc : Determine if the specified ray intersects the triangle, and if so
//        at what 't' value.
// Note : This overload is called if no normal is available.
//-----------------------------------------------------------------------------

bool Library::BSPEngine::Collision::RayIntersectTriangle(const XMFLOAT3 & Origin, const XMFLOAT3 & Velocity, const XMFLOAT3 & v1, const XMFLOAT3 & v2, const XMFLOAT3 & v3, float & t, bool BiDirectional)
{
	XMFLOAT3 Normal;

	// Generate the triangle normal
	//D3DXVec3Cross(&Normal, &(v2 - v1), &(v3 - v1));
	//D3DXVec3Normalize(&Normal, &Normal);
	XMStoreFloat3(&Normal,
		XMVector3Normalize(
			XMVector3Cross(
			 XMVectorSubtract(XMLoadFloat3(&v2), XMLoadFloat3(&v1)),
				XMVectorSubtract(XMLoadFloat3(&v3), XMLoadFloat3(&v1))
			)
		)
	);


	// Pass through to standard function
	return Collision::RayIntersectTriangle(Origin, Velocity, v1, v2, v3, Normal, t, BiDirectional);

}

//-----------------------------------------------------------------------------
// Name : RayIntersectTriangle () (Static)
// Desc : Determine if the specified ray intersects the triangle, and if so
//        at what 't' value.
//-----------------------------------------------------------------------------
bool Library::BSPEngine::Collision::RayIntersectTriangle(const XMFLOAT3 & Origin, const XMFLOAT3 & Velocity, const XMFLOAT3 & v1, const XMFLOAT3 & v2, const XMFLOAT3 & v3, const XMFLOAT3 & TriNormal, float & t, bool BiDirectional)
{
	XMFLOAT3 Point;

	// First calculate the intersection point with the triangle plane
	if (!Collision::RayIntersectPlane(Origin, Velocity, TriNormal, v1, t, BiDirectional)) return false;

	// Calculate the intersection point on the plane
	//Point = Origin + (Velocity * t);
	XMStoreFloat3(&Point,
		XMVectorMultiplyAdd(
			XMLoadFloat3(&Velocity),
			XMVectorReplicate(t),
			XMLoadFloat3(&Origin)
		)
	);
	// If this point does not fall within the bounds of the triangle, we are not intersecting
	if (!Collision::PointInTriangle(Point, v1, v2, v3, TriNormal)) return false;

	// We're intersecting the triangle
	return true;
}

//-----------------------------------------------------------------------------
// Name : RayIntersectAABB () (Static)
// Desc : This function test to see if a ray intersects the specified axis
//        aligned bounding box.
//-----------------------------------------------------------------------------
bool Library::BSPEngine::Collision::RayIntersectAABB(const XMFLOAT3 & Origin, const XMFLOAT3 & Velocity, const XMFLOAT3 & Min, const XMFLOAT3 & Max, float & t, bool bIgnoreX, bool bIgnoreY, bool bIgnoreZ)
{
	float tMin = -FLT_MAX, tMax = FLT_MAX;
	XMFLOAT3 ExtentsMin, ExtentsMax;

	// Calculate required values
	//ExtentsMin = Min - Origin;
	XMStoreFloat3(&ExtentsMin,
		XMVectorSubtract(XMLoadFloat3(&Min), XMLoadFloat3(&Origin)));
	//ExtentsMax = Max - Origin;
	XMStoreFloat3(&ExtentsMax,
		XMVectorSubtract(XMLoadFloat3(&Max), XMLoadFloat3(&Origin)));

	float RecipDirVec[3] = { 1.0f / Velocity.x, 1.0f / Velocity.y, 1.0f / Velocity.z };
	float VelocityVec[3] = { Velocity.x, Velocity.y, Velocity.z };
	float ExtentsMinVec[3] = { ExtentsMin.x, ExtentsMin.y, ExtentsMin.z };
	float ExtentsMaxVec[3] = { ExtentsMax.x, ExtentsMax.y, ExtentsMax.z };

	// Test box 'slabs'
	for (int i = 0; i < 3; ++i)
	{
		// Ignore this component?
		if (i == 0 && bIgnoreX) continue;
		else if (i == 1 && bIgnoreY) continue;
		else if (i == 2 && bIgnoreZ) continue;

		// Is it pointing toward?
		if (fabsf(VelocityVec[i]) > 1e-3f)
		{
			float t1 = ExtentsMaxVec[i] * RecipDirVec[i];
			float t2 = ExtentsMinVec[i] * RecipDirVec[i];
			// Reorder if necessary
			if (t1 > t2) { float fTemp = t1; t1 = t2; t2 = fTemp; }
			// Compare and validate
			if (t1 > tMin) tMin = t1;
			if (t2 < tMax) tMax = t2;
			if (tMin  > tMax) return false;
			if (tMax < 0) return false;

		} // End if toward

	} // Next 'Slab'

	// Pick the correct t value
	if (tMin > 0) t = tMin; else t = tMax;

	// We intersected!
	return true;

}

bool Library::BSPEngine::Collision::SphereIntersectPlane(const XMFLOAT3 & Center, float Radius, const XMFLOAT3 & Velocity, const XMFLOAT3 & PlaneNormal, const XMFLOAT3 & PlanePoint, float & tMax)
{
	//numer = D3DXVec3Dot(&(Center - PlanePoint), &PlaneNormal) - Radius;
	float numer = XMVectorGetX(
						XMVector3Dot(
							XMVectorSubtract(XMLoadFloat3(&Center), XMLoadFloat3(&PlanePoint)),
							XMLoadFloat3(&PlaneNormal)
						)
				   ) - Radius;
	// denom = D3DXVec3Dot(&Velocity, &PlaneNormal);
	float denom = XMVectorGetX(
						XMVector3Dot(
							XMLoadFloat3(&Velocity), XMLoadFloat3(&PlaneNormal)
						)
					);

	// Are we already overlapping?
	if (numer < 0.0f || denom > -0.0000001f)
	{
		// The sphere is moving away from the plane
		if (denom > -1e-5f) return false;

		// Sphere is too far away from the plane
		if (numer < -Radius) return false;

		// Calculate the penetration depth
		tMax = numer;

		// Intersecting!
		return true;

	} // End if overlapping

	// We are not overlapping, perform ray-plane intersection
	float t = -(numer / denom);

	// Ensure we are within range
	if (t < 0.0f || t > tMax) return false;

	// Store interval
	tMax = t;

	// Intersecting!
	return true;
}

//-----------------------------------------------------------------------------
// Name : SphereIntersectLineSegment () (Static)
// Desc : Determine whether or not the sphere intersects a line segment.
//-----------------------------------------------------------------------------
bool Library::BSPEngine::Collision::SphereIntersectLineSegment(const XMFLOAT3 & Center, float Radius, const XMFLOAT3 & Velocity, const XMFLOAT3 & v1, const XMFLOAT3 & v2, float & tMax, XMFLOAT3 & CollisionNormal)
{
	XMFLOAT3 E, L, X, Y, PointOnEdge, CollisionCenter;
	float       a, b, c, d, e, t, n;

	// Setup the equation values
	//E = v2 - v1;
	XMStoreFloat3(&E, XMVectorSubtract(XMLoadFloat3(&v2), XMLoadFloat3(&v1)));
	//L = Center - v1;
	XMStoreFloat3(&L, XMVectorSubtract(XMLoadFloat3(&Center), XMLoadFloat3(&v1)));

	// Re-normalize the cylinder radius with segment length (((P - C) x E)² = r²)
	//e = D3DXVec3Length(&E);
	e = XMVectorGetX(XMVector3Length(XMLoadFloat3(&E)));

	// If e == 0 we can't possibly succeed, the edge is degenerate and we'll
	// get a divide by 0 error in the normalization and 2nd order solving.
	if (e < 1e-5f) return false;

	// Normalize the line vector
	//E /= e;
	XMStoreFloat3(&E, XMVectorDivide(XMLoadFloat3(&E), XMVectorReplicate(e)));

	// Generate cross values
	//D3DXVec3Cross(&X, &L, &E);
	XMStoreFloat3(&X,
		XMVector3Cross(
			XMLoadFloat3(&L), XMLoadFloat3(&E)
		)
	);
	
	//D3DXVec3Cross(&Y, &Velocity, &E);
	XMStoreFloat3(&Y,
		XMVector3Cross(
			XMLoadFloat3(&Velocity), XMLoadFloat3(&E)
		)
	);

	// Setup the input values for the quadratic equation
	//a = D3DXVec3LengthSq(&Y);
	a = XMVectorGetX(XMVector3LengthSq(XMLoadFloat3(&Y)));
	//b = 2.0f * D3DXVec3Dot(&X, &Y);
	b = 2.0f * XMVectorGetX(XMVector3Dot(XMLoadFloat3(&X), XMLoadFloat3(&Y)));
	//c = D3DXVec3LengthSq(&X) - (Radius*Radius);
	c = XMVectorGetX(XMVector3LengthSq(XMLoadFloat3(&X))) - (Radius*Radius);

	// If the sphere center is already inside the cylinder, we need an overlap test
	if (c < 0.0f)
	{
		// Find the distance along the line where our sphere center is positioned.
		// (i.e. sphere center projected down onto line)
		//d = D3DXVec3Dot(&L, &E);
		d = XMVectorGetX(XMVector3Dot(XMLoadFloat3(&L), XMLoadFloat3(&E)));

		// Is this before or after line start?
		if (d < 0.0f)
		{
			// The point is before the beginning of the line, test against the first vertex
			return SphereIntersectPoint(Center, Radius, Velocity, v1, tMax, CollisionNormal);

		} // End if before line start
		else if (d > e)
		{
			// The point is after the end of the line, test against the second vertex
			return SphereIntersectPoint(Center, Radius, Velocity, v2, tMax, CollisionNormal);

		} // End if after line end
		else
		{
			// Point within the line segment
			//PointOnEdge = v1 + E * d;
			XMStoreFloat3(&PointOnEdge,
								XMVectorMultiplyAdd(
									XMLoadFloat3(&E),
									XMVectorReplicate(d),
									XMLoadFloat3(&v1))
							);

			// Generate collision normal
			//CollisionNormal = Center - PointOnEdge;
			XMStoreFloat3(&CollisionNormal,
				XMVectorSubtract(XMLoadFloat3(&Center), XMLoadFloat3(&PointOnEdge)));
			//n = D3DXVec3Length(&CollisionNormal);
			n = XMVectorGetX(XMVector3Length(XMLoadFloat3(&CollisionNormal)));
			//CollisionNormal /= n;
			XMStoreFloat3(&CollisionNormal,
				XMVectorDivide(XMLoadFloat3(&CollisionNormal), XMVectorReplicate(n)));

			// Calculate t value (remember we only enter here if we're already overlapping)
			// Remember, when we're overlapping we have no choice but to return a physical distance (the penetration depth)
			t = n - Radius;
			if (tMax < t) return false;

			// Store t and return
			tMax = t;

			// Edge Overlap
			return true;

		} // End if inside line segment

	} // End if sphere inside cylinder

	// If we are already checking for overlaps, return
	if (tMax < 0.0f) return false;

	// Solve the quadratic for t
	if (!SolveCollision(a, b, c, t)) return false;

	// Is the segment too far away?
	if (t > tMax) return false;

	// Calculate the new sphere center at the time of collision
	//CollisionCenter = Center + Velocity * t;
	XMStoreFloat3(&CollisionCenter,
						XMVectorMultiplyAdd(
							XMLoadFloat3(&Velocity),
							XMVectorReplicate(t),
							XMLoadFloat3(&Center))
					);

	// Project this down onto the edge
	//d = D3DXVec3Dot(&(CollisionCenter - v1), &E);
	d = XMVectorGetX(
		XMVector3Dot(
			XMVectorSubtract(XMLoadFloat3(&CollisionCenter), XMLoadFloat3(&v1)),
			XMLoadFloat3(&E)
		)
	);

	// Simply check whether we need to test the end points as before
	if (d < 0.0f)
		return SphereIntersectPoint(Center, Radius, Velocity, v1, tMax, CollisionNormal);
	else if (d > e)
		return SphereIntersectPoint(Center, Radius, Velocity, v2, tMax, CollisionNormal);

	// Calculate the Point of contact on the line segment
	//PointOnEdge = v1 + E * d;
	XMStoreFloat3(&PointOnEdge,
						XMVectorMultiplyAdd(
							XMLoadFloat3(&E),
							XMVectorReplicate(d),
							XMLoadFloat3(&v1))
					);

	// We can now generate our normal, store the interval and return
	//D3DXVec3Normalize(&CollisionNormal, &(CollisionCenter - PointOnEdge));
	XMStoreFloat3(&CollisionNormal,
		XMVector3Normalize(
			XMVectorSubtract(XMLoadFloat3(&CollisionCenter),
						     XMLoadFloat3(&PointOnEdge))
		));
	
	tMax = t;

	// Intersecting!
	return true;
}

//-----------------------------------------------------------------------------
// Name : SphereIntersectPoint () (Static)
// Desc : Determine whether or not the sphere intersects a point.
//-----------------------------------------------------------------------------
bool Library::BSPEngine::Collision::SphereIntersectPoint(const XMFLOAT3 & Center, float Radius, const XMFLOAT3 & Velocity, const XMFLOAT3 & Point, float & tMax, XMFLOAT3 & CollisionNormal)
{
	XMFLOAT3 L, CollisionCenter;
	float       a, b, c, l, l2, t;

	// Setup the equation values
	//L = Center - Point;
	XMStoreFloat3(&L, XMVectorSubtract(XMLoadFloat3(&Center), XMLoadFloat3(&Point)));
	//l2 = D3DXVec3LengthSq(&L);
	l2 = XMVectorGetX(XMVector3LengthSq(XMLoadFloat3(&L)));

	// Setup the input values for the quadratic equation
	//a = D3DXVec3LengthSq(&Velocity);
	a = XMVectorGetX(XMVector3LengthSq(XMLoadFloat3(&Velocity)));
	//b = 2.0f * D3DXVec3Dot(&Velocity, &L);
	b = 2.0f * XMVectorGetX(XMVector3Dot(XMLoadFloat3(&Velocity), XMLoadFloat3(&L)));
	c = l2 - (Radius * Radius);

	// If c < 0 then we are overlapping, return the overlap
	if (c < 0.0f)
	{
		// Remember, when we're overlapping we have no choice 
		// but to return a physical distance (the penetration depth)
		l = sqrtf(l2);
		t = l - Radius;

		// Outside our range?
		if (tMax < t) return false;

		// Generate the collision normal
		//CollisionNormal = L / l;
		XMStoreFloat3(&CollisionNormal,
			XMVectorDivide(XMLoadFloat3(&L), XMVectorReplicate(l)));

		// Store t and return
		tMax = t;

		// Vertex Overlap
		return true;

	} // End if overlapping 

	  // If we are already checking for overlaps, return
	if (tMax < 0.0f) return false;

	// Solve the quadratic for t
	if (!SolveCollision(a, b, c, t)) return false;

	// Is the vertex too far away?
	if (t > tMax) return false;

	// Calculate the new sphere position at the time of contact
	//CollisionCenter = Center + Velocity * t;
	XMStoreFloat3(&CollisionCenter,
		XMVectorMultiplyAdd(
			XMLoadFloat3(&Velocity),
			XMVectorReplicate(t),
			XMLoadFloat3(&Center))
	);

	// We can now generate our normal, store the interval and return
	//D3DXVec3Normalize(&CollisionNormal, &(CollisionCenter - Point));
	XMStoreFloat3(&CollisionNormal,
		XMVector3Normalize(
			XMVectorSubtract(XMLoadFloat3(&CollisionCenter),
				XMLoadFloat3(&Point))
		));

	tMax = t;

	// Intersecting!
	return true;

}

//-----------------------------------------------------------------------------
// Name : SphereIntersectTriangle () (Static)
// Desc : Determine whether or not the sphere intersects a triangle.
//-----------------------------------------------------------------------------
bool Library::BSPEngine::Collision::SphereIntersectTriangle(const XMFLOAT3 & Center, float Radius, const XMFLOAT3 & Velocity, const XMFLOAT3 & v1, const XMFLOAT3 & v2, const XMFLOAT3 & v3, const XMFLOAT3 & TriNormal, float & tMax, XMFLOAT3 & CollisionNormal)
{
	float       t = tMax;
	XMFLOAT3 CollisionCenter;
	bool        bCollided = false;

	// Find the time of collision with the triangle's plane.
	if (!SphereIntersectPlane(Center, Radius, Velocity, TriNormal, v1, t)) return false;

	// Calculate the sphere's center at the point of collision with the plane
	if (t < 0) {
		//CollisionCenter = Center + (TriNormal * -t);
		XMStoreFloat3(&CollisionCenter,
			XMVectorMultiplyAdd(
				XMLoadFloat3(&TriNormal),
				XMVectorReplicate(-t),
				XMLoadFloat3(&Center))
			);
	}
	else {
		//CollisionCenter = Center + (Velocity * t);
		XMStoreFloat3(&CollisionCenter,
			XMVectorMultiplyAdd(
				XMLoadFloat3(&Velocity),
				XMVectorReplicate(t),
				XMLoadFloat3(&Center))
		);
	}

	// If this point is within the bounds of the triangle, we have found the collision
	if (PointInTriangle(CollisionCenter, v1, v2, v3, TriNormal))
	{
		// Collision normal is just the triangle normal
		CollisionNormal = TriNormal;
		tMax = t;

		// Intersecting!
		return true;

	} // End if point within triangle interior

	// Otherwise we need to test each edge
	bCollided |= SphereIntersectLineSegment(Center, Radius, Velocity, v1, v2, tMax, CollisionNormal);
	bCollided |= SphereIntersectLineSegment(Center, Radius, Velocity, v2, v3, tMax, CollisionNormal);
	bCollided |= SphereIntersectLineSegment(Center, Radius, Velocity, v3, v1, tMax, CollisionNormal);
	return bCollided;
}

//-----------------------------------------------------------------------------
// Name : AABBIntersectAABB () (Static)
// Desc : Determine if the two Axis Aligned bounding boxes specified are
//        intersecting with one another.
//-----------------------------------------------------------------------------
bool Library::BSPEngine::Collision::AABBIntersectAABB(const XMFLOAT3 & Min1, const XMFLOAT3 & Max1, const XMFLOAT3 & Min2, const XMFLOAT3 & Max2, bool bIgnoreX, bool bIgnoreY, bool bIgnoreZ)
{
	return (bIgnoreX || Min1.x <= Max2.x) && (bIgnoreY || Min1.y <= Max2.y) &&
		(bIgnoreZ || Min1.z <= Max2.z) && (bIgnoreX || Max1.x >= Min2.x) &&
		(bIgnoreY || Max1.y >= Min2.y) && (bIgnoreZ || Max1.z >= Min2.z);

}

//-----------------------------------------------------------------------------
// Name : AABBIntersectAABB () (Static, Overload)
// Desc : Determine if the two Axis Aligned bounding boxes specified are
//        intersecting with one another.
// Note : This overload also tests whether Bounds2 is fully contained within
//        Bounds1.
//-----------------------------------------------------------------------------
bool Library::BSPEngine::Collision::AABBIntersectAABB(bool & bContained, const XMFLOAT3 & Min1, const XMFLOAT3 & Max1, const XMFLOAT3 & Min2, const XMFLOAT3 & Max2, bool bIgnoreX, bool bIgnoreY, bool bIgnoreZ)
{
	// Set to true by default
	bContained = true;

	// Does the point fall outside any of the AABB planes?
	if (!bIgnoreX && (Min2.x < Min1.x || Min2.x > Max1.x)) bContained = false;
	else if (!bIgnoreY && (Min2.y < Min1.y || Min2.y > Max1.y)) bContained = false;
	else if (!bIgnoreZ && (Min2.z < Min1.z || Min2.z > Max1.z)) bContained = false;
	else if (!bIgnoreX && (Max2.x < Min1.x || Max2.x > Max1.x)) bContained = false;
	else if (!bIgnoreY && (Max2.y < Min1.y || Max2.y > Max1.y)) bContained = false;
	else if (!bIgnoreZ && (Max2.z < Min1.z || Max2.z > Max1.z)) bContained = false;

	// Return immediately if it's fully contained
	if (bContained == true) return true;

	// Perform full intersection test
	return (bIgnoreX || Min1.x <= Max2.x) && (bIgnoreY || Min1.y <= Max2.y) &&
		(bIgnoreZ || Min1.z <= Max2.z) && (bIgnoreX || Max1.x >= Min2.x) &&
		(bIgnoreY || Max1.y >= Min2.y) && (bIgnoreZ || Max1.z >= Min2.z);

}

Library::BSPEngine::Collision::CLASSIFYTYPE Library::BSPEngine::Collision::PointClassifyPlane(const XMFLOAT3 & Point, const XMFLOAT3 & PlaneNormal, float PlaneDistance)
{
	// Calculate distance from plane
	//float fDistance = D3DXVec3Dot(&Point, &PlaneNormal) + PlaneDistance;
	float fDistance = XMVectorGetX(XMVector3Dot(XMLoadFloat3(&Point), XMLoadFloat3(&PlaneNormal)))
						+ PlaneDistance;

	// Retrieve classification
	CLASSIFYTYPE Location = CLASSIFY_ONPLANE;
	if (fDistance < -1e-3f) Location = CLASSIFY_BEHIND;
	if (fDistance >  1e-3f) Location = CLASSIFY_INFRONT;

	// Return the classification
	return Location;
}

//-----------------------------------------------------------------------------
// Name : RayClassifyPlane () (Static)
// Desc : Classify the ray against a plane.
//-----------------------------------------------------------------------------
Library::BSPEngine::Collision::CLASSIFYTYPE Library::BSPEngine::Collision::RayClassifyPlane(const XMFLOAT3 & Origin, const XMFLOAT3 & Velocity, const XMFLOAT3 & PlaneNormal, float PlaneDistance)
{
	CLASSIFYTYPE Location[2];
	ULONG        Infront = 0, Behind = 0, OnPlane = 0, i;

	// Classify the two end points of our ray
	Location[0] = PointClassifyPlane(Origin, PlaneNormal, PlaneDistance);
	XMFLOAT3 endPoint;
	XMStoreFloat3(&endPoint,
		XMVectorAdd(XMLoadFloat3(&Origin),
				    XMLoadFloat3(&Velocity)));
	Location[1] = PointClassifyPlane(endPoint, PlaneNormal, PlaneDistance);

	// Count up the locations
	for (i = 0; i < 2; ++i)
	{
		// Check the position
		if (Location[i] == CLASSIFY_INFRONT)
			Infront++;
		else if (Location[i] == CLASSIFY_BEHIND)
			Behind++;
		else
		{
			OnPlane++;
			Infront++;
			Behind++;

		} // End if on plane

	} // Next ray point

	  // Return Result
	if (OnPlane == 2) return CLASSIFY_ONPLANE;     // On Plane
	if (Behind == 2) return CLASSIFY_BEHIND;      // Behind
	if (Infront == 2) return CLASSIFY_INFRONT;     // In Front
	return CLASSIFY_SPANNING; // Spanning
}

//-----------------------------------------------------------------------------
// Name : PolyClassifyPlane () (Static)
// Desc : Classify the poly against a plane.
//-----------------------------------------------------------------------------
Library::BSPEngine::Collision::CLASSIFYTYPE Library::BSPEngine::Collision::PolyClassifyPlane(void * pVertices, ULONG VertexCount, ULONG Stride, const XMFLOAT3 & PlaneNormal, float PlaneDistance)
{
	ULONG   Infront = 0, Behind = 0, OnPlane = 0, i;
	UCHAR   Location = 0;
	float	Result = 0;
	UCHAR  *pBuffer = (UCHAR*)pVertices;

	// Loop round each vector
	for (i = 0; i < VertexCount; ++i)
	{
		// Calculate distance from plane
		XMFLOAT3 *vertex = (XMFLOAT3*)pBuffer;
		//float fDistance = D3DXVec3Dot((D3DXVECTOR3*)pBuffer, &PlaneNormal) + PlaneDistance;
		float fDistance = XMVectorGetX(
			XMVector3Dot(XMLoadFloat3(vertex), XMLoadFloat3(&PlaneNormal)))
			+ PlaneDistance;
		pBuffer += Stride;

		// Retrieve classification
		Location = CLASSIFY_ONPLANE;
		if (fDistance < -1e-3f) Location = CLASSIFY_BEHIND;
		if (fDistance >  1e-3f) Location = CLASSIFY_INFRONT;

		// Check the position
		if (Location == CLASSIFY_INFRONT)
			Infront++;
		else if (Location == CLASSIFY_BEHIND)
			Behind++;
		else
		{
			OnPlane++;
			Infront++;
			Behind++;

		} // End if on plane

	} // Next Vertex

	  // Return Result
	if (OnPlane == VertexCount) return CLASSIFY_ONPLANE;     // On Plane
	if (Behind == VertexCount) return CLASSIFY_BEHIND;      // Behind
	if (Infront == VertexCount) return CLASSIFY_INFRONT;     // In Front
	return CLASSIFY_SPANNING; // Spanning
}

//-----------------------------------------------------------------------------
// Name : AABBClassifyPlane () (Static)
// Desc : Classify the AABB against a plane.
//-----------------------------------------------------------------------------
Library::BSPEngine::Collision::CLASSIFYTYPE Library::BSPEngine::Collision::AABBClassifyPlane(const XMFLOAT3 & Min, const XMFLOAT3 & Max, const XMFLOAT3 & PlaneNormal, float PlaneDistance)
{
	XMFLOAT3 NearPoint, FarPoint;

	// Calculate near / far extreme points
	if (PlaneNormal.x > 0.0f) { FarPoint.x = Max.x; NearPoint.x = Min.x; }
	else { FarPoint.x = Min.x; NearPoint.x = Max.x; }

	if (PlaneNormal.y > 0.0f) { FarPoint.y = Max.y; NearPoint.y = Min.y; }
	else { FarPoint.y = Min.y; NearPoint.y = Max.y; }

	if (PlaneNormal.z > 0.0f) { FarPoint.z = Max.z; NearPoint.z = Min.z; }
	else { FarPoint.z = Min.z; NearPoint.z = Max.z; }

	// If near extreme point is outside, then the AABB is totally outside the plane
	//if (D3DXVec3Dot(&PlaneNormal, &NearPoint) + PlaneDistance > 0.0f) return CLASSIFY_INFRONT;
	if (XMVectorGetX(
		XMVector3Dot(
			XMLoadFloat3(&PlaneNormal),
			XMLoadFloat3(&NearPoint)
		)
	) + PlaneDistance > 0.0f) return CLASSIFY_INFRONT;

	// If far extreme point is outside, then the AABB is intersecting the plane
	//if (D3DXVec3Dot(&PlaneNormal, &FarPoint) + PlaneDistance > 0.0f) return CLASSIFY_SPANNING;
	if (XMVectorGetX(
		XMVector3Dot(
			XMLoadFloat3(&PlaneNormal),
			XMLoadFloat3(&FarPoint)
		)
	) + PlaneDistance > 0.0f) return CLASSIFY_SPANNING;

	// We're behind
	return CLASSIFY_BEHIND;
}

bool Library::BSPEngine::Collision::SolveCollision(float a, float b, float c, float & t)
{
	float d, one_over_two_a, t0, t1, temp;

	// Basic equation solving
	d = b*b - 4 * a*c;

	// No root if d < 0
	if (d < 0.0f) return false;

	// Setup for calculation
	d = sqrtf(d);
	one_over_two_a = 1.0f / (2.0f * a);

	// Calculate the two possible roots
	t0 = (-b - d) * one_over_two_a;
	t1 = (-b + d) * one_over_two_a;

	// Order the results
	if (t1 < t0) { temp = t0; t0 = t1; t1 = temp; }

	// Fail if both results are negative
	if (t1 < 0.0f) return false;

	// Return the first positive root
	if (t0 < 0.0f) t = t1; else t = t0;

	// Solution found
	return true;
}
