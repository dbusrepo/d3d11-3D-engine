//-----------------------------------------------------------------------------
// File: CBounds.h
//
// Desc: Vector class, handles all vector mathmatics routines
//


//-----------------------------------------------------------------------------
// CBounds Specific Includes
//-----------------------------------------------------------------------------
#include "CBounds.h"
#include "CPlane.h"

//-----------------------------------------------------------------------------
// Name : CBounds3 () (CONSTRUCTOR)
// Desc : CBounds3 Class Constructor
//-----------------------------------------------------------------------------
CBounds3::CBounds3( ) 
{
	// Initialise values
    Reset();
}

//-----------------------------------------------------------------------------
// Name : CBounds3 () (ALTERNATE CONSTRUCTOR)
// Desc : CBounds3 Class Constructor, sets values from vector values passed
//-----------------------------------------------------------------------------
CBounds3::CBounds3( const CVector3& vecMin, const CVector3& vecMax ) 
{
	// Copy vector values
	Min = vecMin;
	Max = vecMax;
}

//-----------------------------------------------------------------------------
// Name : CBounds3 () (ALTERNATE CONSTRUCTOR)
// Desc : CBounds3 Class Constructor, sets values from float values passed
//-----------------------------------------------------------------------------
CBounds3::CBounds3( float xMin, float yMin, float zMin, float xMax, float yMax, float zMax )
{
    // Copy coordinate values
    Min = CVector3( xMin, yMin, zMin );
    Max = CVector3( xMax, yMax, zMax );
}

//-----------------------------------------------------------------------------
// Name : Reset ()
// Desc : Resets the bounding box values.
//-----------------------------------------------------------------------------
void CBounds3::Reset()
{
    Min = CVector3(  9999999,  9999999,  9999999 );
	Max = CVector3( -9999999, -9999999, -9999999 );
}

//-----------------------------------------------------------------------------
// Name : GetDimensions ()
// Desc : Returns a vector containing the dimensions of the bounding box
//-----------------------------------------------------------------------------
CVector3 CBounds3::GetDimensions() const
{
    return Max - Min;
}

//-----------------------------------------------------------------------------
// Name : GetCentre ()
// Desc : Returns a vector containing the exact centre point of the box
//-----------------------------------------------------------------------------
CVector3 CBounds3::GetCentre() const
{
    return (Max + Min) / 2;
}

//-----------------------------------------------------------------------------
// Name : GetPlane ()
// Desc : Retrieves the plane for the specified side of the bounding box
//-----------------------------------------------------------------------------
CPlane3 CBounds3::GetPlane( unsigned long Side ) const
{
    CPlane3 BoundsPlane;
    
    // Select the requested side
    switch ( Side ) {
        case BOUNDS_PLANE_TOP:
            BoundsPlane.Normal.y = 1;
            BoundsPlane.Distance = -Max.Dot(BoundsPlane.Normal);
            break;
        case BOUNDS_PLANE_RIGHT:
            BoundsPlane.Normal.x = 1;
            BoundsPlane.Distance = -Max.Dot(BoundsPlane.Normal);
            break;
        case BOUNDS_PLANE_BACK:
            BoundsPlane.Normal.z = 1;
            BoundsPlane.Distance = -Max.Dot(BoundsPlane.Normal);
            break;
        case BOUNDS_PLANE_BOTTOM:
            BoundsPlane.Normal.y = -1;
            BoundsPlane.Distance = -Min.Dot(BoundsPlane.Normal);
            break;
        case BOUNDS_PLANE_LEFT:
            BoundsPlane.Normal.x = -1;
            BoundsPlane.Distance = -Min.Dot(BoundsPlane.Normal);
            break;
        case BOUNDS_PLANE_FRONT:
            BoundsPlane.Normal.z = -1;
            BoundsPlane.Distance = -Min.Dot(BoundsPlane.Normal);
            break;
    } // End Side Switch

    // Return the plane
    return BoundsPlane;
}

//-----------------------------------------------------------------------------
// Name : CalculateFromPolygon ()
// Desc : Calculates the bounding box based on the Face passed.
//-----------------------------------------------------------------------------
CBounds3& CBounds3::CalculateFromPolygon( const CVector3 pVertices[], unsigned long VertexCount, unsigned long VertexStride, bool bReset )
{
    CVector3      * vtx;
    unsigned char * pVerts  = (unsigned char*)pVertices;

    // Check for invalid params
    if ( !pVertices ) return *this;

    // Reset the box if requested
    if ( bReset ) Reset();

    // Loop round all the Vertices in the face
    for ( unsigned long v = 0; v < VertexCount; v++, pVerts += VertexStride ) 
    {
        vtx = (CVector3*)pVerts;
        if ( vtx->x < Min.x ) Min.x = vtx->x;
        if ( vtx->y < Min.y ) Min.y = vtx->y;
        if ( vtx->z < Min.z ) Min.z = vtx->z;
        if ( vtx->x > Max.x ) Max.x = vtx->x;
        if ( vtx->y > Max.y ) Max.y = vtx->y;
        if ( vtx->z > Max.z ) Max.z = vtx->z;
    } // Next Vertex

    return *this;
}

//-----------------------------------------------------------------------------
// Name : Validate ()
// Desc : Ensures that the values placed in the Min / Max values never make the
//        bounding box itself inverted.
//-----------------------------------------------------------------------------
void CBounds3::Validate()
{
    float rTemp;
    if ( Max.x < Min.x ) { rTemp = Max.x; Max.x = Min.x; Min.x = rTemp; }
    if ( Max.y < Min.y ) { rTemp = Max.y; Max.y = Min.y; Min.y = rTemp; }
    if ( Max.z < Min.z ) { rTemp = Max.z; Max.z = Min.z; Min.z = rTemp; }
}

//-----------------------------------------------------------------------------
// Name : IntersectedByBounds()
// Desc : Tests to see if this AABB is intersected by another AABB
//-----------------------------------------------------------------------------
bool CBounds3::IntersectedByBounds( const CBounds3& Bounds ) const
{
    return (Min.x <= Bounds.Max.x) && (Min.y <= Bounds.Max.y) &&
           (Min.z <= Bounds.Max.z) && (Max.x >= Bounds.Min.x) &&
           (Max.y >= Bounds.Min.y) && (Max.z >= Bounds.Min.z);
}

//-----------------------------------------------------------------------------
// Name : IntersectedByBounds()
// Desc : Tests to see if this AABB is intersected by another AABB, includes
//        a tolerance for checking.
//-----------------------------------------------------------------------------
bool CBounds3::IntersectedByBounds( const CBounds3& Bounds, const CVector3& Tolerance ) const
{
	return ((Min.x - Tolerance.x) <= (Bounds.Max.x + Tolerance.x)) &&
           ((Min.y - Tolerance.y) <= (Bounds.Max.y + Tolerance.y)) &&
           ((Min.z - Tolerance.z) <= (Bounds.Max.z + Tolerance.z)) &&
           ((Max.x + Tolerance.x) >= (Bounds.Min.x - Tolerance.x)) &&
           ((Max.y + Tolerance.y) >= (Bounds.Min.y - Tolerance.y)) &&
           ((Max.z + Tolerance.z) >= (Bounds.Min.z - Tolerance.z));
}

//-----------------------------------------------------------------------------
// Name : PointInBounds()
// Desc : Tests to see if a point falls within this bounding box or not.
//-----------------------------------------------------------------------------
bool CBounds3::PointInBounds( const CVector3& Point ) const
{
    if (Point.x < Min.x || Point.x > Max.x) return false;
    if (Point.y < Min.y || Point.y > Max.y) return false;
    if (Point.z < Min.z || Point.z > Max.z) return false;
    return true;
}

//-----------------------------------------------------------------------------
// Name : PointInBounds()
// Desc : Tests to see if a point falls within this bounding box or not.
//-----------------------------------------------------------------------------
bool CBounds3::PointInBounds( const CVector3& Point, const CVector3& Tolerance ) const
{
    if (Point.x < Min.x - Tolerance.x || Point.x > Max.x + Tolerance.x) return false;
    if (Point.y < Min.y - Tolerance.y || Point.y > Max.y + Tolerance.y) return false;
    if (Point.z < Min.z - Tolerance.z || Point.z > Max.z + Tolerance.z) return false;
    return true;
}

//-----------------------------------------------------------------------------
// Name : operator+=()
// Desc : Moves the bounding box by the vector passed.
//-----------------------------------------------------------------------------
CBounds3& CBounds3::operator+= ( const CVector3& vecShift )
{
    Min += vecShift;
    Max += vecShift;
    return *this;
}