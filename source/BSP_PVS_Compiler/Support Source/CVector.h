#ifndef _CVECTOR_H_
#define _CVECTOR_H_

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class CMatrix4;
class CPlane3;

//-----------------------------------------------------------------------------
// Main class definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CVector4 (Class)
// Desc : 4D Vector class. Storage for vector values and wraps up vector math.
//-----------------------------------------------------------------------------
class CVector4
{
public:

    //------------------------------------------------------------
	// Constructors / Destructors for this Class
	//------------------------------------------------------------
    CVector4( );
    CVector4( const CVector4 & vec );
    CVector4( float _x, float _y, float _z, float _w);
    
	//------------------------------------------------------------
	// Public Variables For This Class
	//------------------------------------------------------------
    union {
        struct {
            float    x;              // Vector X Component
            float    y;              // Vector Y Component
            float    z;              // Vector Z Component
            float    w;              // Vector W Component
        }; // End struct
        float v[4];
    }; // End union
	
	//------------------------------------------------------------
	// Public Functions For This Class
	//------------------------------------------------------------
    bool                IsEmpty() const;
    bool                SetBaryCentric( const CVector4& V1, const CVector4& V2, const CVector4& V3, const float& f, const float& g );
    bool                SetCatmullRom( const CVector4& V1, const CVector4& V2, const CVector4& V3, const CVector4& V4, const float& s );
    bool                SetHermite( const CVector4& V1, const CVector4& T1, const CVector4& V2, const CVector4& T2, const float& s );
    CVector4            Lerp( const CVector4& V1, const float& s ) const;
    CVector4            Maximize( const CVector4& V1 ) const;
    CVector4            Minimize( const CVector4& V1 ) const;
    CVector4&           Scale( const float &Scale );
    CVector4            Cross( const CVector4& V1, const CVector4& V2 ) const;
    float               Dot( const CVector4& vec ) const;
    float               Length() const;
    float               SquareLength() const;
    CVector4            Transform( const CMatrix4& mtx ) const;
    bool                FuzzyCompare( const CVector4& vecCompare,  const float& Tolerance) const;
    CVector4&           Normalize();

    //------------------------------------------------------------
	// Public Operators For This Class
	//------------------------------------------------------------
    CVector4  operator+  ( const CVector4& vec ) const;
    CVector4  operator-  ( const CVector4& vec ) const;
    CVector4  operator*  ( const float& Value  ) const;
    CVector4  operator/  ( const float& Value  ) const;
    
    CVector4& operator+= ( const CVector4& vec );
    CVector4& operator-= ( const CVector4& vec );
    CVector4& operator/= ( const float& Value  );
    CVector4& operator*= ( const float& Value  );

    CVector4  operator+  () const;
    CVector4  operator-  () const;
    CVector4& operator=  ( const CVector4& vec );
    bool      operator== ( const CVector4& vec ) const;
    bool      operator!= ( const CVector4& vec ) const;

    operator float * ();
    operator const float * () const;

    //------------------------------------------------------------
	// Public Friend Operators For This Class
	//------------------------------------------------------------
    friend CVector4 operator * (float Value, const CVector4& vec );
};

//-----------------------------------------------------------------------------
// Name : CVector3 (Class)
// Desc : 3D Vector class. Storage for vector values and wraps up vector math.
//-----------------------------------------------------------------------------
class CVector3
{
public:

    //------------------------------------------------------------
	// Constructors / Destructors for this Class
	//------------------------------------------------------------
    CVector3( );
    CVector3( const CVector3 & vec );
    CVector3( float _x, float _y, float _z );
    
	//------------------------------------------------------------
	// Public Variables For This Class
	//------------------------------------------------------------
    union {
        struct {
            float    x;              // Vector X Component
            float    y;              // Vector Y Component
            float    z;              // Vector Z Component
        }; // End struct
        float v[3];
    }; // End union
	
	//------------------------------------------------------------
	// Public Functions For This Class
	//------------------------------------------------------------
    bool                IsEmpty() const;
    bool                SetBaryCentric( const CVector3& V1, const CVector3& V2, const CVector3& V3, const float& f, const float& g );
    bool                SetCatmullRom( const CVector3& V1, const CVector3& V2, const CVector3& V3, const CVector3& V4, const float& s );
    bool                SetHermite( const CVector3& V1, const CVector3& T1, const CVector3& V2, const CVector3& T2, const float& s );
    CVector3            Lerp( const CVector3& V1, const float& s) const;
    CVector3            Maximize( const CVector3& V1 ) const;
    CVector3            Minimize( const CVector3& V1 ) const;
    CVector3&           Scale( const float &Scale );
    CVector3            Cross( const CVector3& V1 ) const;
    float               Dot( const CVector3& vec ) const;
    float               Length() const;
    float               SquareLength() const;
    CVector4            Transform( const CMatrix4& mtx ) const;
    CVector3            TransformCoord( const CMatrix4& mtx ) const;
    CVector3            TransformNormal( const CMatrix4& mtx ) const;
    bool                FuzzyCompare( const CVector3& vecCompare,  const float& Tolerance) const;
    CVector3&           Normalize();
    float               DistanceToPlane( const CPlane3& Plane ) const;
    float               DistanceToPlane( const CPlane3& Plane, const CVector3& Direction ) const;
    float               DistanceToLine( const CVector3 &vecStart, const CVector3& vecEnd ) const;

    //------------------------------------------------------------
	// Public Operators For This Class
	//------------------------------------------------------------
    CVector3  operator+  ( const CVector3& vec ) const;
    CVector3  operator-  ( const CVector3& vec ) const;
    CVector3  operator*  ( const CVector3& vec ) const;
    CVector3  operator*  ( const CMatrix4& mtx ) const;
    CVector3  operator*  ( const float& Value  ) const;
    CVector3  operator/  ( const float& Value  ) const;
    
    CVector3& operator+= ( const CVector3& vec );
    CVector3& operator-= ( const CVector3& vec );
    CVector3& operator/= ( const float& Value  );
    CVector3& operator*= ( const CMatrix4& mtx );
    CVector3& operator*= ( const float& Value  );

    CVector3  operator+  () const;
    CVector3  operator-  () const;
    CVector3& operator=  ( const CVector3& vec );
    bool      operator== ( const CVector3& vec ) const;
    bool      operator!= ( const CVector3& vec ) const;

    operator float * ();
    operator const float * () const;

    //------------------------------------------------------------
	// Public Friend Operators For This Class
	//------------------------------------------------------------
    friend CVector3 operator * (float Value, const CVector3& vec );
};

//-----------------------------------------------------------------------------
// Name : CVector2 (Class)
// Desc : 2D Vector class. Storage for vector values and wraps up vector math.
//-----------------------------------------------------------------------------
class CVector2
{
public:

    //------------------------------------------------------------
	// Constructors / Destructors for this Class
	//------------------------------------------------------------
    CVector2( ) { }
    CVector2( const CVector2 & vec ) {x = vec.x; y = vec.y; }
    CVector2( float _x, float _y ) { x = _x; y = _y; }
    
	//------------------------------------------------------------
	// Public Variables For This Class
	//------------------------------------------------------------
    union {
        struct {
            float    x;              // Vector X Component
            float    y;              // Vector Y Component
        }; // End struct
        float v[2];
    }; // End union
};


#endif // _CVECTOR_H_
