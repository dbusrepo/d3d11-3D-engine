#ifndef _CPLANE_H_
#define _CPLANE_H_

//-----------------------------------------------------------------------------
// Class specific includes
//-----------------------------------------------------------------------------
#include "CVector.h"
#include "Common.h"
//-----------------------------------------------------------------------------
// Miscellaneous Defines
//-----------------------------------------------------------------------------
#ifndef NULL
#define NULL 0
#endif

//-----------------------------------------------------------------------------
// Typedefs, structures & Enumerators 
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Main class definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CPlane3 (Class)
// Desc : 3D Plane class, handles all plane classification / calculations
//-----------------------------------------------------------------------------
class CPlane3
{
public:
    //------------------------------------------------------------
	// Constructors / Destructors for this Class
	//------------------------------------------------------------
     CPlane3( );
     CPlane3( const CVector3& vecNormal, float fDistance );
     CPlane3( const CVector3& vecNormal, const CVector3& vecPointOnPlane );

    //------------------------------------------------------------
	// Public Functions for This Class
	//------------------------------------------------------------
    CLASSIFYTYPE    ClassifyPoly( const CVector3 pVertices[], unsigned long VertexCount, unsigned long VertexStride ) const;
    CLASSIFYTYPE    ClassifyPoint( const CVector3& vecPoint, float * Dist = NULL ) const;
    CLASSIFYTYPE    ClassifyLine( const CVector3& Point1, const CVector3& Point2 ) const;
    CVector3        GetPointOnPlane() const;
    bool            GetRayIntersect( const CVector3& RayStart, const CVector3& RayEnd, CVector3& Intersection, float * pPercentage = NULL ) const;
    bool            SameFacing( const CVector3& vecNormal ) const;

    //------------------------------------------------------------
	// Public Variables for This Class
	//------------------------------------------------------------
    CVector3        Normal;             // Plane Normal
    float           Distance;           // PlaneDIstance
};

#endif // _CPLANE_H_