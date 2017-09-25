//-----------------------------------------------------------------------------
// File: CVector.cpp
//
// Desc: Vector class, handles all vector mathmatics routines


//------------------------------------------------------------
// CVector specific includes
//------------------------------------------------------------
#include <math.h>
#include "CVector.h"
#include "CMatrix.h"
#include "CPlane.h"

CVector4::CVector4( )
{
    // Reset required variables.
    x = 0.0f; y = 0.0f; z = 0.0f; w = 0.0f;
}

CVector4::CVector4( const CVector4 & vec )
{
    x = vec.x; y = vec.y; z = vec.z; w = vec.w;
}

CVector4::CVector4( float _x, float _y, float _z, float _w)
{
    x = _x; y = _y; z = _z; w = _w;
}


CVector4::operator float * ()
{
    return (float*)&x;
}

CVector4::operator const float * () const
{
    return (const float*)&x;
}

CVector4 CVector4::operator+  () const
{
    return *this;
}

CVector4 CVector4::operator-  () const
{
    return CVector4( -x, -y, -z, -w );
}

CVector4& CVector4::operator+= ( const CVector4& vec )
{
    x += vec.x; y += vec.y; z += vec.z; w += vec.w;
    return *this;
}

CVector4& CVector4::operator-= ( const CVector4& vec )
{
    x -= vec.x; y -= vec.y; z -= vec.z; w -= vec.w;
    return *this;
}

CVector4& CVector4::operator*= ( const float& Value  )
{
    x *= Value; y *= Value; z *= Value; w *= Value;
    return *this;
}

CVector4& CVector4::operator/= ( const float& Value  )
{
    float fValue = 1.0f / Value;
    x *= fValue; y *= fValue; z *= fValue; w *= fValue;
    return *this;
}

CVector4 CVector4::operator+  ( const CVector4& vec ) const
{
    return CVector4( x + vec.x, y + vec.y, z + vec.z, w + vec.w );
}

CVector4 CVector4::operator-  ( const CVector4& vec ) const
{
    return CVector4( x - vec.x, y - vec.y, z - vec.z, w - vec.w );
}

CVector4 CVector4::operator*  ( const float& Value  ) const
{
    return CVector4( x * Value, y * Value, z * Value, w * Value );
}

CVector4 CVector4::operator/  ( const float& Value  ) const
{
    float fValue = 1.0f / Value;
    return CVector4( x * fValue, y * fValue, z * fValue, w * fValue );
}

CVector4& CVector4::operator=  ( const CVector4& vec )
{
    x = vec.x; y = vec.y; z = vec.z; w = vec.w;
    return *this;
}

bool CVector4::operator== ( const CVector4& vec ) const
{
    return (x == vec.x) && (y == vec.y) && (z == vec.z) && (w == vec.w);
}

bool CVector4::operator!= ( const CVector4& vec ) const
{
    return (x != vec.x) || (y != vec.y) || (z != vec.z) || (w != vec.w);
}
    
CVector4 operator * (float Value, const CVector4& vec )
{
    return CVector4( vec.x * Value, vec.y * Value, vec.z * Value, vec.w * Value );
}

bool CVector4::IsEmpty() const
{
    return (x == 0.0f) && (y == 0.0f) && (z == 0.0f) && (w == 0.0f);
}

bool CVector4::SetBaryCentric( const CVector4& V1, const CVector4& V2, const CVector4& V3, const float& f, const float& g )
{
    x = V1.x + f * (V2.x - V1.x) + g * (V3.x - V1.x);
    y = V1.y + f * (V2.y - V1.y) + g * (V3.y - V1.y);
    z = V1.z + f * (V2.z - V1.z) + g * (V3.z - V1.z);
    w = V1.w + f * (V2.w - V1.w) + g * (V3.w - V1.w);

    return true;
}

bool CVector4::SetCatmullRom( const CVector4& V1, const CVector4& V2, const CVector4& V3, const CVector4& V4, const float& s )
{
    float   ss, sss, a, b, c, d;

    ss  = s * s;
    sss = s * ss;

    a = -0.5f * sss + ss - 0.5f * s;
    b =  1.5f * sss - 2.5f * ss + 1.0f;
    c = -1.5f * sss + 2.0f * ss + 0.5f * s;
    d =  0.5f * sss - 0.5f * ss;

    x = a * V1.x + b * V2.x + c * V3.x + d * V4.x;
    y = a * V1.y + b * V2.y + c * V3.y + d * V4.y;
    z = a * V1.z + b * V2.z + c * V3.z + d * V4.z;
    w = a * V1.w + b * V2.w + c * V3.w + d * V4.w;

    return true;
}

bool CVector4::SetHermite( const CVector4& V1, const CVector4& T1, const CVector4& V2, const CVector4& T2, const float& s )
{
    float   ss, sss, a, b, c, d;

    ss  = s * s;
    sss = s * ss;

    a =  2.0f * sss - 3.0f * ss + 1.0f;
    b = -2.0f * sss + 3.0f * ss;
    c =  sss - 2.0f * ss + s;
    d =  sss - ss;

    x = a * V1.x + b * V2.x + c * T1.x + d * T2.x;
    y = a * V1.y + b * V2.y + c * T1.y + d * T2.y;
    z = a * V1.z + b * V2.z + c * T1.z + d * T2.z;
    w = a * V1.w + b * V2.w + c * T1.w + d * T2.w;

    return true;
}
CVector4 CVector4::Lerp( const CVector4& V1, const float& s ) const
{
    return CVector4( x + s * (V1.x - x), y + s * (V1.y - y), z + s * (V1.z - z), w + s * (V1.w - w));
}

CVector4 CVector4::Maximize( const CVector4& V1 ) const
{
    return CVector4( (x > V1.x) ? x : V1.x, (y > V1.y) ? y : V1.y, (z > V1.z) ? z : V1.z, (w > V1.w) ? w : V1.w );
}

CVector4 CVector4::Minimize( const CVector4& V1 ) const
{
    return CVector4( (x < V1.x) ? x : V1.x, (y < V1.y) ? y : V1.y, (z < V1.z) ? z : V1.z, (w < V1.w) ? w : V1.w );
}

CVector4& CVector4::Scale( const float &Scale )
{
    x *= Scale; y *= Scale; z *= Scale; w *= Scale;
    return *this;
}

CVector4 CVector4::Cross( const CVector4& V1, const CVector4& V2 ) const
{
    float   a, b, c, d, e, f;

    a = V1.x * V2.y - V1.y * V2.x;
    b = V1.x * V2.z - V1.z * V2.x;
    c = V1.x * V2.w - V1.w * V2.x;
    d = V1.y * V2.z - V1.z * V2.y;
    e = V1.y * V2.w - V1.w * V2.y;
    f = V1.z * V2.w - V1.w * V2.z;

    return CVector4( ( f * y - e * z + d * w), 
                     (-f * x - c * z + b * w),
                     ( e * x - c * y + a * w),
                     (-d * x - b * y + a * z));
}

float CVector4::Dot( const CVector4& vec ) const
{
    return x * vec.x + y * vec.y + z * vec.z + w * vec.w;
}

float CVector4::Length() const
{
    return sqrtf(x * x + y * y + z * z + w * w);
}

float CVector4::SquareLength() const
{
    return x * x + y * y + z * z + w * w;
}

CVector4 CVector4::Transform( const CMatrix4& mtx ) const
{
    return CVector4( x * mtx._11 + y * mtx._21 + z * mtx._31 + w * mtx._41,
                     x * mtx._12 + y * mtx._22 + z * mtx._32 + w * mtx._42,
                     x * mtx._13 + y * mtx._23 + z * mtx._33 + w * mtx._43,
                     x * mtx._14 + y * mtx._24 + z * mtx._34 + w * mtx._44 );
}

bool CVector4::FuzzyCompare( const CVector4& vecCompare,  const float& Tolerance) const
{
    if ( fabsf(x) > Tolerance ) return false;
    if ( fabsf(y) > Tolerance ) return false;
    if ( fabsf(z) > Tolerance ) return false;
    if ( fabsf(w) > Tolerance ) return false;
    return true;
}

CVector4& CVector4::Normalize()
{
    float   denom;

    denom = sqrtf(x * x + y * y + z * z + w * w);
    if (fabsf(denom) < 1e-5f) return *this;

    denom = 1.0f / denom;

    x *= denom;
    y *= denom;
    z *= denom;
    w *= denom;

    return *this;
}

CVector3::CVector3( )
{
    // Reset required variables.
    x = 0.0f; y = 0.0f; z = 0.0f;
}

CVector3::CVector3( const CVector3 & vec )
{
    x = vec.x; y = vec.y; z = vec.z;
}

CVector3::CVector3( float _x, float _y, float _z)
{
    x = _x; y = _y; z = _z;
}


CVector3::operator float * ()
{
    return (float*)&x;
}

CVector3::operator const float * () const
{
    return (const float*)&x;
}

CVector3 CVector3::operator+  () const
{
    return *this;
}

CVector3 CVector3::operator-  () const
{
    return CVector3( -x, -y, -z );
}

CVector3& CVector3::operator+= ( const CVector3& vec )
{
    x += vec.x; y += vec.y; z += vec.z;
    return *this;
}

CVector3& CVector3::operator-= ( const CVector3& vec )
{
    x -= vec.x; y -= vec.y; z -= vec.z;
    return *this;
}

CVector3& CVector3::operator*= ( const CMatrix4& mtx )
{
    float   rhw, _x, _y, _z;

    rhw = (x * mtx._14 + y * mtx._24 + z * mtx._34 + mtx._44);
    if (fabsf(rhw) < 1e-5f) { x = 0; y = 0; z = 0; return *this; }

    rhw = 1.0f / rhw;

    _x = rhw * (x * mtx._11 + y * mtx._21 + z * mtx._31 + mtx._41);
    _y = rhw * (x * mtx._12 + y * mtx._22 + z * mtx._32 + mtx._42);
    _z = rhw * (x * mtx._13 + y * mtx._23 + z * mtx._33 + mtx._43);

    x = _x; y = _y; z = _z;

    return *this;
}

CVector3& CVector3::operator*= ( const float& Value  )
{
    x *= Value; y *= Value; z *= Value;
    return *this;
}

CVector3& CVector3::operator/= ( const float& Value  )
{
    float fValue = 1.0f / Value;
    x *= fValue; y *= fValue; z *= fValue;
    return *this;
}

CVector3 CVector3::operator+  ( const CVector3& vec ) const
{
    return CVector3( x + vec.x, y + vec.y, z + vec.z );
}

CVector3 CVector3::operator-  ( const CVector3& vec ) const
{
    return CVector3( x - vec.x, y - vec.y, z - vec.z );
}

CVector3 CVector3::operator*  ( const float& Value  ) const
{
    return CVector3( x * Value, y * Value, z * Value );
}

CVector3 CVector3::operator*  ( const CMatrix4& mtx  ) const
{
    float   rhw;

    rhw = (x * mtx._14 + y * mtx._24 + z * mtx._34 + mtx._44);
    if (fabsf(rhw) < 1e-5f) return CVector3( 0, 0, 0 );

    rhw = 1.0f / rhw;

    return CVector3( rhw * (x * mtx._11 + y * mtx._21 + z * mtx._31 + mtx._41),
                     rhw * (x * mtx._12 + y * mtx._22 + z * mtx._32 + mtx._42),
                     rhw * (x * mtx._13 + y * mtx._23 + z * mtx._33 + mtx._43));
}

CVector3 CVector3::operator*  ( const CVector3& vec  ) const
{
    return CVector3( x * vec.x, y * vec.y, z * vec.z );
}

CVector3 CVector3::operator/  ( const float& Value  ) const
{
    float fValue = 1.0f / Value;
    return CVector3( x * fValue, y * fValue, z * fValue );
}

CVector3& CVector3::operator=  ( const CVector3& vec )
{
    x = vec.x; y = vec.y; z = vec.z;
    return *this;
}

bool CVector3::operator== ( const CVector3& vec ) const
{
    return (x == vec.x) && (y == vec.y) && (z == vec.z);
}

bool CVector3::operator!= ( const CVector3& vec ) const
{
    return (x != vec.x) || (y != vec.y) || (z != vec.z);
}
    
CVector3 operator * (float Value, const CVector3& vec )
{
    return CVector3( vec.x * Value, vec.y * Value, vec.z * Value );
}

bool CVector3::IsEmpty() const
{
    return (x == 0.0f) && (y == 0.0f) && (z == 0.0f);
}

bool CVector3::SetBaryCentric( const CVector3& V1, const CVector3& V2, const CVector3& V3, const float& f, const float& g )
{
    x = V1.x + f * (V2.x - V1.x) + g * (V3.x - V1.x);
    y = V1.y + f * (V2.y - V1.y) + g * (V3.y - V1.y);
    z = V1.z + f * (V2.z - V1.z) + g * (V3.z - V1.z);
    
    return true;
}

bool CVector3::SetCatmullRom( const CVector3& V1, const CVector3& V2, const CVector3& V3, const CVector3& V4, const float& s )
{
    float   ss, sss, a, b, c, d;

    ss  = s * s;
    sss = s * ss;

    a = -0.5f * sss + ss - 0.5f * s;
    b =  1.5f * sss - 2.5f * ss + 1.0f;
    c = -1.5f * sss + 2.0f * ss + 0.5f * s;
    d =  0.5f * sss - 0.5f * ss;

    x = a * V1.x + b * V2.x + c * V3.x + d * V4.x;
    y = a * V1.y + b * V2.y + c * V3.y + d * V4.y;
    z = a * V1.z + b * V2.z + c * V3.z + d * V4.z;

    return true;
}

bool CVector3::SetHermite( const CVector3& V1, const CVector3& T1, const CVector3& V2, const CVector3& T2, const float& s )
{
    float   ss, sss, a, b, c, d;

    ss  = s * s;
    sss = s * ss;

    a =  2.0f * sss - 3.0f * ss + 1.0f;
    b = -2.0f * sss + 3.0f * ss;
    c =  sss - 2.0f * ss + s;
    d =  sss - ss;

    x = a * V1.x + b * V2.x + c * T1.x + d * T2.x;
    y = a * V1.y + b * V2.y + c * T1.y + d * T2.y;
    z = a * V1.z + b * V2.z + c * T1.z + d * T2.z;

    return true;
}
CVector3 CVector3::Lerp( const CVector3& V1, const float& s ) const
{
    return CVector3( x + s * (V1.x - x), y + s * (V1.y - y), z + s * (V1.z - z) );
}

CVector3 CVector3::Maximize( const CVector3& V1 ) const
{
    return CVector3( (x > V1.x) ? x : V1.x, (y > V1.y) ? y : V1.y, (z > V1.z) ? z : V1.z );
}

CVector3 CVector3::Minimize( const CVector3& V1 ) const
{
    return CVector3( (x < V1.x) ? x : V1.x, (y < V1.y) ? y : V1.y, (z < V1.z) ? z : V1.z );
}

CVector3& CVector3::Scale( const float &Scale )
{
    x *= Scale; y *= Scale; z *= Scale;
    return *this;
}

CVector3 CVector3::Cross( const CVector3& V1 ) const
{
    return CVector3( y * V1.z - z * V1.y, z * V1.x - x * V1.z, x * V1.y - y * V1.x );
}

float CVector3::Dot( const CVector3& vec ) const
{
    return x * vec.x + y * vec.y + z * vec.z;
}

float CVector3::Length() const
{
    return sqrtf(x * x + y * y + z * z);
}

float CVector3::SquareLength() const
{
    return x * x + y * y + z * z;
}

CVector4 CVector3::Transform( const CMatrix4& mtx ) const
{
    return CVector4( x * mtx._11 + y * mtx._21 + z * mtx._31 + mtx._41,
                     x * mtx._12 + y * mtx._22 + z * mtx._32 + mtx._42,
                     x * mtx._13 + y * mtx._23 + z * mtx._33 + mtx._43,
                     x * mtx._14 + y * mtx._24 + z * mtx._34 + mtx._44 );
}

CVector3 CVector3::TransformCoord( const CMatrix4& mtx ) const
{
    float   rhw;

    rhw = (x * mtx._14 + y * mtx._24 + z * mtx._34 + mtx._44);
    if (fabsf(rhw) < 1e-5f) return CVector3( 0, 0, 0 );

    rhw = 1.0f / rhw;

    return CVector3( rhw * (x * mtx._11 + y * mtx._21 + z * mtx._31 + mtx._41),
                     rhw * (x * mtx._12 + y * mtx._22 + z * mtx._32 + mtx._42),
                     rhw * (x * mtx._13 + y * mtx._23 + z * mtx._33 + mtx._43));
}

CVector3 CVector3::TransformNormal( const CMatrix4& mtx ) const
{
    return CVector3( x * mtx._11 + y * mtx._21 + z * mtx._31,
                     x * mtx._12 + y * mtx._22 + z * mtx._32,
                     x * mtx._13 + y * mtx._23 + z * mtx._33 );
}


bool CVector3::FuzzyCompare( const CVector3& vecCompare,  const float& Tolerance) const
{
    if ( fabsf(x - vecCompare.x) >= Tolerance ) return false;
    if ( fabsf(y - vecCompare.y) >= Tolerance ) return false;
    if ( fabsf(z - vecCompare.z) >= Tolerance ) return false;
    return true;
}

CVector3& CVector3::Normalize()
{
    float   denom;

    denom = sqrtf(x * x + y * y + z * z);
    if (fabsf(denom) < 1e-5f) return *this;

    denom = 1.0f / denom;

    x *= denom;
    y *= denom;
    z *= denom;

    return *this;
}

//-----------------------------------------------------------------------------
// Name : DistanceToPlane ()
// Desc : Calculates the Distance from the position stored in this vector, and 
//        the plane passed to this function.
//-----------------------------------------------------------------------------
float CVector3::DistanceToPlane( const CPlane3& Plane ) const
{
    return Dot( Plane.Normal ) + Plane.Distance;
}

//-----------------------------------------------------------------------------
// Name : DistanceToPlane ()
// Desc : Calculates the Distance from the position stored in this vector, and 
//        the plane passed to this function, along a ray direction.
//-----------------------------------------------------------------------------
float CVector3::DistanceToPlane( const CPlane3& Plane, const CVector3& Direction ) const
{
    float PlaneDistance = Dot( -Plane.Normal ) + Plane.Distance;
    return PlaneDistance * (1 / (-Plane.Normal).Dot( Direction ));
}

//-----------------------------------------------------------------------------
// Name : DistanceToLine ()
// Desc : Calculates the distance from the position stored in this vector, and
//        the line segment passed into this function.
// Note : Returns a value that is out of range if the point is outside of the
//        extents of vecStart & vecEnd.
//-----------------------------------------------------------------------------
float CVector3::DistanceToLine( const CVector3 &vecStart, const CVector3& vecEnd ) const
{
    CVector3 c, v;
    float    d, t;

    // Determine t 
    // (the length of the vector from ‘vecStart’ to ‘this’)
    c = *this  - vecStart;
    v = vecEnd - vecStart;   
    d = v.Length();
    
    v.Normalize();
    
    t = v.Dot( c );
   
    // Check to see if ‘t’ is beyond the extents of the line segment
    if (t < 0.01f)     return 99999.0f;
    if (t > d - 0.01f) return 99999.0f;
  
    // set length of v to t. v is normalized so this is easy
    v.x = vecStart.x + (v.x * t);
    v.y = vecStart.y + (v.y * t);
    v.z = vecStart.z + (v.z * t);
  
    // Return the distance
    return ((*this) - v).Length();
}

