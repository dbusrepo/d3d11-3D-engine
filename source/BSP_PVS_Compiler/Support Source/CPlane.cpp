//-----------------------------------------------------------------------------
// File: CPlane.cpp
//
// Desc: This class is a standard Plane class, contains all plane processing
//

//-----------------------------------------------------------------------------
// Specific includes required for this class
//-----------------------------------------------------------------------------
#include "CPlane.h"
#include "Common.h" // For Math Constants

//-----------------------------------------------------------------------------
// Desc : CPlane3 member functions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CPlane3() (CONSTRUCTOR)
// Desc : Constructor for this class.
//-----------------------------------------------------------------------------
CPlane3::CPlane3()
{
	// Initialise anything we need
    Normal      = CVector3( 0.0f, 0.0f, 0.0f );
    Distance    = 0.0f;
}

//-----------------------------------------------------------------------------
// Name : CPlane3() (ALTERNATE CONSTRUCTOR)
// Desc : Constructor for this class.
//-----------------------------------------------------------------------------
CPlane3::CPlane3( const CVector3& vecNormal, float fDistance )
{
	// Initialise anything we need
    Normal      = vecNormal; Normal.Normalize();
    Distance    = fDistance;
}

//-----------------------------------------------------------------------------
// Name : CPlane3() (ALTERNATE CONSTRUCTOR)
// Desc : Constructor for this class.
//-----------------------------------------------------------------------------
CPlane3::CPlane3( const CVector3& vecNormal, const CVector3& vecPointOnPlane )
{
	// Initialise anything we need
    Normal      = vecNormal; Normal.Normalize();
    Distance    = -vecPointOnPlane.Dot(Normal);
}

//-----------------------------------------------------------------------------
// Name : ClassifyPoint ()
// Desc : Classifies the vector relative to the plane
//-----------------------------------------------------------------------------
CLASSIFYTYPE CPlane3::ClassifyPoint( const CVector3& vecPoint, float * Dist ) const
{
	float result	= vecPoint.Dot(Normal) + Distance;
    if (Dist) *Dist = result;
	if ( result < -EPSILON ) return CLASSIFY_BEHIND;
	if ( result > EPSILON  ) return CLASSIFY_INFRONT;
	return CLASSIFY_ONPLANE;
}

//-----------------------------------------------------------------------------
// Name : ClassifyPoly ()
// Desc : Classifies the vertices passed relative to the plane
// Note : Because this is a multi purpose function, we require information
//        about the size of each individual vertex in the array passed. This
//        is used to seek to the start of each vertex regardless of its size.
//-----------------------------------------------------------------------------
CLASSIFYTYPE CPlane3::ClassifyPoly( const CVector3 pVertices[], unsigned long VertexCount, unsigned long VertexStride ) const
{
    unsigned char  *pVerts  = (unsigned char*)pVertices;
	unsigned long   Infront = 0, Behind = 0, OnPlane=0;
    float	        result  = 0;

	// Loop round each vector
	for ( unsigned long i = 0; i < VertexCount; i++, pVerts += VertexStride )
    {
        // Calculate distance
		result = (*(CVector3*)pVerts).Dot( Normal ) + Distance;
		// Check the position
		if (result > EPSILON ) {
			Infront++;
		} else if (result < -EPSILON) {
			Behind++;
		} else {
			OnPlane++;
			Infront++;
			Behind++;
        } // End if Result tested
	} // End For Each vector

    // Return Result
	if ( OnPlane == VertexCount ) return CLASSIFY_ONPLANE;
	if ( Behind  == VertexCount ) return CLASSIFY_BEHIND;
	if ( Infront == VertexCount ) return CLASSIFY_INFRONT;
	return CLASSIFY_SPANNING;
}

//-----------------------------------------------------------------------------
// Name : ClassifyLine ()
// Desc : Classifies the line passed relative to the plane
//-----------------------------------------------------------------------------
CLASSIFYTYPE CPlane3::ClassifyLine( const CVector3& Point1, const CVector3& Point2 ) const
{
	int		Infront = 0, Behind = 0, OnPlane=0;
	float	result  = 0;

    // Calculate distance
	result = Point1.Dot( Normal ) + Distance;
	// Check the position
	if (result > EPSILON ) {
		Infront++;
	} else if (result < -EPSILON) {
		Behind++;
	} else {
		OnPlane++;
		Infront++;
		Behind++;
    } // End if Result tested

    // Calculate distance
	result = Point2.Dot( Normal ) + Distance;
	// Check the position
	if (result > EPSILON ) {
		Infront++;
	} else if (result < -EPSILON) {
		Behind++;
	} else {
		OnPlane++;
		Infront++;
		Behind++;
    } // End if Result tested

    // Check Results
	if ( OnPlane == 2 ) return CLASSIFY_ONPLANE;
	if ( Behind  == 2 ) return CLASSIFY_BEHIND;
	if ( Infront == 2 ) return CLASSIFY_INFRONT;
	return CLASSIFY_SPANNING;
}

//-----------------------------------------------------------------------------
// Name : GetPointOnPlane ()
// Desc : Retrieves a point on the plane
//-----------------------------------------------------------------------------
CVector3 CPlane3::GetPointOnPlane( ) const
{
    return Normal * -Distance;
}

//-----------------------------------------------------------------------------
// Name : GetRayIntersect ()
// Desc : Calculates the Intersection point between a ray and this plane
//-----------------------------------------------------------------------------
bool CPlane3::GetRayIntersect( const CVector3& RayStart,  const CVector3& RayEnd, CVector3& Intersection, float * pPercentage ) const
{
    CVector3 Velocity = RayEnd - RayStart;

    // Get the length of the 'adjacent' side of the virtual triangle formed
    // by the velocity and normal.
    float ProjRayLength = Velocity.Dot( Normal );

    // Calculate distance to plane along it's normal
    float PlaneDistance = RayStart.Dot( Normal ) + Distance;

    // Calculate the actual interval (Distance along the adjacent side / length of adjacent side).
    float t = PlaneDistance / -ProjRayLength;

    // Outside our valid range? If yes, return no collide.
    if ( t < 0.0f || t > 1.0f ) return false;

    // Calculate Intersection
    Intersection = RayStart + (Velocity * t);

    // Store percentage if requested
    if ( pPercentage ) *pPercentage = t;

    // We're intersecting
    return true;
}

//-----------------------------------------------------------------------------
// Name : SameFacing ()
// Desc : Determines if the normal passed is facing in the same direction as
//        this plane.
// Note : Assumes that we already know that the normals are coplanar.
//-----------------------------------------------------------------------------
bool CPlane3::SameFacing( const CVector3& vecNormal ) const
{
    if ( fabsf((Normal.x - vecNormal.x) + 
               (Normal.y - vecNormal.y) + 
               (Normal.z - vecNormal.z)) < 0.1f) return true;
    
    // Opposing Direction
    return false;
}