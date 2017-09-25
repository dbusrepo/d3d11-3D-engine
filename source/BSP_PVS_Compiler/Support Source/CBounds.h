
#ifndef _CBOUNDS_H_
#define _CBOUNDS_H_

//-----------------------------------------------------------------------------
// CBounds Specific Includes
//-----------------------------------------------------------------------------
#include "CVector.h"

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Miscellaneous Defines
//-----------------------------------------------------------------------------
#ifndef NULL
#define NULL 0
#endif

//-----------------------------------------------------------------------------
// Class Specific Definitions, Constants & Macros
//-----------------------------------------------------------------------------
#define BOUNDS_PLANE_TOP        0
#define BOUNDS_PLANE_BOTTOM     1
#define BOUNDS_PLANE_LEFT       2
#define BOUNDS_PLANE_RIGHT      3
#define BOUNDS_PLANE_FRONT      4
#define BOUNDS_PLANE_BACK       5

//-----------------------------------------------------------------------------
// Main class definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CBounds3 (Class)
// Desc : Storage for box vector values and wraps up common functionality
//-----------------------------------------------------------------------------
class CBounds3
{
public:
	
	//------------------------------------------------------------
	// Constructors / Destructors for this Class
	//------------------------------------------------------------
    CBounds3( );
    CBounds3( const CVector3& vecMin, const CVector3& vecMax );
    CBounds3( float xMin, float yMin, float zMin, float xMax, float yMax, float zMax );

	//------------------------------------------------------------
	// Public Variables For This Class
	//------------------------------------------------------------
    CVector3        Min;
	CVector3        Max;

	//------------------------------------------------------------
	// Public Functions For This Class
	//------------------------------------------------------------
    CVector3        GetDimensions() const;
    CVector3        GetCentre() const;
    CPlane3         GetPlane( unsigned long Side ) const;
    CBounds3&       CalculateFromPolygon( const CVector3 pVertices[], unsigned long VertexCount, unsigned long VertexStride, bool bReset = true );
    bool            IntersectedByBounds( const CBounds3& Bounds ) const;
    bool            IntersectedByBounds( const CBounds3& Bounds, const CVector3& Tolerance ) const;
    bool            PointInBounds( const CVector3& Point ) const;
    bool            PointInBounds( const CVector3& Point, const CVector3& Tolerance ) const;
    void            Validate();
    void            Reset();

    //------------------------------------------------------------
	// Public Operators For This Class
	//------------------------------------------------------------
    CBounds3&        operator += ( const CVector3& vecShift );
    CBounds3&        operator -= ( const CVector3& vecShift );

};

#endif // _CBOUNDS_H_
