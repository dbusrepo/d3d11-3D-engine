//-----------------------------------------------------------------------------
// File: CMatrix.h
//
// Desc: Matrix class, handles all Matrix mathmatics routines


//-----------------------------------------------------------------------------
// CMatrix Specific Includes
//-----------------------------------------------------------------------------
#include "CMatrix.h"
#include "CVector.h"
#include <memory.h>
#include <math.h>

//-----------------------------------------------------------------------------
// Static Variable Definitions
//-----------------------------------------------------------------------------
CMatrix4 CMatrix4::m_mtxStaticIdentity = CMatrix4( 1.0f, 0.0f, 0.0f, 0.0f, 
                                                   0.0f, 1.0f, 0.0f, 0.0f,
                                                   0.0f, 0.0f, 1.0f, 0.0f,
                                                   0.0f, 0.0f, 0.0f, 1.0f );

//-----------------------------------------------------------------------------
// Name : CMatrix4 () (Default Constructor)
// Desc : CMatrix4 Class Constructor
//-----------------------------------------------------------------------------
CMatrix4::CMatrix4( ) 
{

}

//-----------------------------------------------------------------------------
// Name : CMatrix4 () (Alternate Constructor)
// Desc : CMatrix4 Class Constructor, copies matrix passed
//-----------------------------------------------------------------------------
CMatrix4::CMatrix4( const CMatrix4& mtx )
{
    ::memcpy( m, mtx.m, sizeof(m));
}


//-----------------------------------------------------------------------------
// Name : CMatrix4 () (Alternate Constructor)
// Desc : CMatrix4 Class Constructor, sets values from real values passed
//-----------------------------------------------------------------------------
CMatrix4::CMatrix4( float _m11, float _m12, float _m13, float _m14,
                    float _m21, float _m22, float _m23, float _m24,
                    float _m31, float _m32, float _m33, float _m34,
                    float _m41, float _m42, float _m43, float _m44) 
{
    _11 = _m11; _12 = _m12; _13 = _m13; _14 = _m14;
    _21 = _m21; _22 = _m22; _23 = _m23; _24 = _m24;
    _31 = _m31; _32 = _m32; _33 = _m33; _34 = _m34;
    _41 = _m41; _42 = _m42; _43 = _m43; _44 = _m44;
}

//-----------------------------------------------------------------------------
// Name : operator()
// Desc : Returns the corresponding referenced matrix value
//-----------------------------------------------------------------------------
float& CMatrix4::operator()(int iRow, int iColumn)
{
    return m[iRow][iColumn]; 
}

//-----------------------------------------------------------------------------
// Name : operator()
// Desc : Returns the corresponding copied matrix value
//-----------------------------------------------------------------------------
float CMatrix4::operator()(int iRow, int iColumn) const
{
    return m[iRow][iColumn]; 
}

//-----------------------------------------------------------------------------
// Name : operator float* ()
// Desc : Returns a pointer to the matrix data
//-----------------------------------------------------------------------------
CMatrix4::operator float * ()
{
    return (float*)&_11;
}

//-----------------------------------------------------------------------------
// Name : operator float* ()
// Desc : Returns a const pointer to the matrix data
//-----------------------------------------------------------------------------
CMatrix4::operator const float * () const
{
    return (const float*)&_11;
}

//-----------------------------------------------------------------------------
// Name : operator*= ()
// Desc : Multiplies this matrix by the matrix passed
//-----------------------------------------------------------------------------
CMatrix4& CMatrix4::operator*= (const CMatrix4& mtx ) 
{
    float a1, a2, a3, a4;

     a1 = _11; a2 = _12; a3 = _13; a4 = _14;
    _11 = a1 * mtx._11 + a2 * mtx._21 + a3 * mtx._31 + a4 * mtx._41;
	_12 = a1 * mtx._12 + a2 * mtx._22 + a3 * mtx._32 + a4 * mtx._42;
	_13 = a1 * mtx._13 + a2 * mtx._23 + a3 * mtx._33 + a4 * mtx._43;
    _14 = a1 * mtx._14 + a2 * mtx._24 + a3 * mtx._34 + a4 * mtx._44;

     a1 = _21; a2 = _22; a3 = _23; a4 = _24;
  	_21 = a1 * mtx._11 + a2 * mtx._21 + a3 * mtx._31 + a4 * mtx._41;
	_22 = a1 * mtx._12 + a2 * mtx._22 + a3 * mtx._32 + a4 * mtx._42;
	_23 = a1 * mtx._13 + a2 * mtx._23 + a3 * mtx._33 + a4 * mtx._43;
    _24 = a1 * mtx._14 + a2 * mtx._24 + a3 * mtx._34 + a4 * mtx._44;

     a1 = _31; a2 = _32; a3 = _33; a4 = _34;
    _31 = a1 * mtx._11 + a2 * mtx._21 + a3 * mtx._31 + a4 * mtx._41;
	_32 = a1 * mtx._12 + a2 * mtx._22 + a3 * mtx._32 + a4 * mtx._42;
	_33 = a1 * mtx._13 + a2 * mtx._23 + a3 * mtx._33 + a4 * mtx._43;
    _34 = a1 * mtx._14 + a2 * mtx._24 + a3 * mtx._34 + a4 * mtx._44;

     a1 = _41; a2 = _42; a3 = _43; a4 = _44;
    _41 = a1 * mtx._11 + a2 * mtx._21 + a3 * mtx._31 + a4 * mtx._41;
	_42 = a1 * mtx._12 + a2 * mtx._22 + a3 * mtx._32 + a4 * mtx._42;
	_43 = a1 * mtx._13 + a2 * mtx._23 + a3 * mtx._33 + a4 * mtx._43;
    _44 = a1 * mtx._14 + a2 * mtx._24 + a3 * mtx._34 + a4 * mtx._44;

    return *this;
}

//-----------------------------------------------------------------------------
// Name : operator+= ()
// Desc : Adds the passed matrix onto this
//-----------------------------------------------------------------------------
CMatrix4& CMatrix4::operator+= (const CMatrix4 & mtx ) 
{
    _11 += mtx._11; _12 += mtx._12; _13 += mtx._13; _14 += mtx._14;
    _21 += mtx._11; _22 += mtx._12; _23 += mtx._13; _24 += mtx._14;
    _31 += mtx._11; _32 += mtx._12; _33 += mtx._13; _34 += mtx._14;
    _41 += mtx._11; _42 += mtx._12; _43 += mtx._13; _44 += mtx._14;

    return *this;
}

//-----------------------------------------------------------------------------
// Name : operator-= ()
// Desc : Subtracts the passed matrix from this
//-----------------------------------------------------------------------------
CMatrix4& CMatrix4::operator-= (const CMatrix4 & mtx ) 
{
    _11 -= mtx._11; _12 -= mtx._12; _13 -= mtx._13; _14 -= mtx._14;
    _21 -= mtx._11; _22 -= mtx._12; _23 -= mtx._13; _24 -= mtx._14;
    _31 -= mtx._11; _32 -= mtx._12; _33 -= mtx._13; _34 -= mtx._14;
    _41 -= mtx._11; _42 -= mtx._12; _43 -= mtx._13; _44 -= mtx._14;

    return *this;
}

//-----------------------------------------------------------------------------
// Name : operator*= ()
// Desc : Multiplies this matrix by the value passed 
//-----------------------------------------------------------------------------
CMatrix4& CMatrix4::operator*= ( const float& Value ) 
{
    _11 *= Value; _12 *= Value; _13 *= Value; _14 *= Value;
    _21 *= Value; _22 *= Value; _23 *= Value; _24 *= Value;
    _31 *= Value; _32 *= Value; _33 *= Value; _34 *= Value;
    _41 *= Value; _42 *= Value; _43 *= Value; _44 *= Value;

    return *this;
}

//-----------------------------------------------------------------------------
// Name : operator/= ()
// Desc : Multiplies this matrix by the inverse of the value passed 
//-----------------------------------------------------------------------------
CMatrix4& CMatrix4::operator/= ( const float& Value ) 
{
    float fValue = 1.0f / Value;
    _11 *= fValue; _12 *= fValue; _13 *= fValue; _14 *= fValue;
    _21 *= fValue; _22 *= fValue; _23 *= fValue; _24 *= fValue;
    _31 *= fValue; _32 *= fValue; _33 *= fValue; _34 *= fValue;
    _41 *= fValue; _42 *= fValue; _43 *= fValue; _44 *= fValue;

    return *this;
}

//-----------------------------------------------------------------------------
// Name : operator+ ()
// Desc : Provided for completeness only, returns a copy of this matrix.
//-----------------------------------------------------------------------------
CMatrix4 CMatrix4::operator+ () const
{
    return *this;
}

//-----------------------------------------------------------------------------
// Name : operator- ()
// Desc : Returns a negated copy of this matrix.
//-----------------------------------------------------------------------------
CMatrix4 CMatrix4::operator- () const
{
    return CMatrix4( -_11, -_12, -_13, -_14,
                     -_21, -_22, -_23, -_24,
                     -_31, -_32, -_33, -_34,
                     -_41, -_42, -_43, -_44);
}

//-----------------------------------------------------------------------------
// Name : operator * ()
// Desc : Returns this matrix multiplied by matrix passed 
//-----------------------------------------------------------------------------
CMatrix4 CMatrix4::operator* ( const CMatrix4& mtx ) const
{
    return CMatrix4( _11 * mtx._11 + _12 * mtx._21 + _13 * mtx._31 + _14 * mtx._41,
	                 _11 * mtx._12 + _12 * mtx._22 + _13 * mtx._32 + _14 * mtx._42,
	                 _11 * mtx._13 + _12 * mtx._23 + _13 * mtx._33 + _14 * mtx._43,
                     _11 * mtx._14 + _12 * mtx._24 + _13 * mtx._34 + _14 * mtx._44,

  	                 _21 * mtx._11 + _22 * mtx._21 + _23 * mtx._31 + _24 * mtx._41,
	                 _21 * mtx._12 + _22 * mtx._22 + _23 * mtx._32 + _24 * mtx._42,
	                 _21 * mtx._13 + _22 * mtx._23 + _23 * mtx._33 + _24 * mtx._43,
                     _21 * mtx._14 + _22 * mtx._24 + _23 * mtx._34 + _24 * mtx._44,

                     _31 * mtx._11 + _32 * mtx._21 + _33 * mtx._31 + _34 * mtx._41,
	                 _31 * mtx._12 + _32 * mtx._22 + _33 * mtx._32 + _34 * mtx._42,
	                 _31 * mtx._13 + _32 * mtx._23 + _33 * mtx._33 + _34 * mtx._43,
                     _31 * mtx._14 + _32 * mtx._24 + _33 * mtx._34 + _34 * mtx._44,

                     _41 * mtx._11 + _42 * mtx._21 + _43 * mtx._31 + _44 * mtx._41,
	                 _41 * mtx._12 + _42 * mtx._22 + _43 * mtx._32 + _44 * mtx._42,
	                 _41 * mtx._13 + _42 * mtx._23 + _43 * mtx._33 + _44 * mtx._43,
                     _41 * mtx._14 + _42 * mtx._24 + _43 * mtx._34 + _44 * mtx._44 );
}

//-----------------------------------------------------------------------------
// Name : operator+ ()
// Desc : Returns this matrix after adding the specified matrix
//-----------------------------------------------------------------------------
CMatrix4 CMatrix4::operator+  (const CMatrix4 & mtx ) const
{
    return CMatrix4( _11 + mtx._11, _12 + mtx._12, _13 + mtx._13, _14 + mtx._14,
                     _21 + mtx._21, _22 + mtx._22, _23 + mtx._23, _24 + mtx._24,
                     _31 + mtx._31, _32 + mtx._32, _33 + mtx._33, _34 + mtx._34,
                     _41 + mtx._41, _42 + mtx._42, _43 + mtx._43, _44 + mtx._44);
}

//-----------------------------------------------------------------------------
// Name : operator- ()
// Desc : Returns this matrix after subtracting the specified matrix
//-----------------------------------------------------------------------------
CMatrix4 CMatrix4::operator-  (const CMatrix4 & mtx ) const
{
    return CMatrix4( _11 - mtx._11, _12 - mtx._12, _13 - mtx._13, _14 - mtx._14,
                     _21 - mtx._21, _22 - mtx._22, _23 - mtx._23, _24 - mtx._24,
                     _31 - mtx._31, _32 - mtx._32, _33 - mtx._33, _34 - mtx._34,
                     _41 - mtx._41, _42 - mtx._42, _43 - mtx._43, _44 - mtx._44);
}

//-----------------------------------------------------------------------------
// Name : operator* ()
// Desc : Returns this matrix multiplied by value passed 
//-----------------------------------------------------------------------------
CMatrix4 CMatrix4::operator* ( const float& Value ) const
{
    return CMatrix4( _11 * Value, _12 * Value, _13 * Value, _14 * Value,
                     _21 * Value, _22 * Value, _23 * Value, _24 * Value,
                     _31 * Value, _32 * Value, _33 * Value, _34 * Value,
                     _41 * Value, _42 * Value, _43 * Value, _44 * Value);
}

//-----------------------------------------------------------------------------
// Name : operator/ ()
// Desc : Returns this matrix multiplied by the inverse of the value passed 
//-----------------------------------------------------------------------------
CMatrix4 CMatrix4::operator/ ( const float& Value ) const
{
    float fValue = 1.0f / Value;
    return CMatrix4( _11 * fValue, _12 * fValue, _13 * fValue, _14 * fValue,
                     _21 * fValue, _22 * fValue, _23 * fValue, _24 * fValue,
                     _31 * fValue, _32 * fValue, _33 * fValue, _34 * fValue,
                     _41 * fValue, _42 * fValue, _43 * fValue, _44 * fValue);
}

//-----------------------------------------------------------------------------
// Name : operator== ()
// Desc : Comparison operator, determines if matrices are identical
//-----------------------------------------------------------------------------
bool CMatrix4::operator == ( const CMatrix4& mtx ) const
{
    return ::memcmp(m, mtx.m, sizeof(m)) == 0;
}

//-----------------------------------------------------------------------------
// Name : operator!= ()
// Desc : Comparison operator, determines if matrices are not identical
//-----------------------------------------------------------------------------
bool CMatrix4::operator != ( const CMatrix4& mtx ) const
{
    return ::memcmp(m, mtx.m, sizeof(m)) != 0;
}

//-----------------------------------------------------------------------------
// Name : operator* () (Friend)
// Desc : For multiplication where value is first, and matrix is second.
//-----------------------------------------------------------------------------
CMatrix4 operator * (float Value, const CMatrix4& mtx)
{
    return CMatrix4( mtx._11 * Value, mtx._12 * Value, mtx._13 * Value, mtx._14 * Value,
                     mtx._21 * Value, mtx._22 * Value, mtx._23 * Value, mtx._24 * Value,
                     mtx._31 * Value, mtx._32 * Value, mtx._33 * Value, mtx._34 * Value,
                     mtx._41 * Value, mtx._42 * Value, mtx._43 * Value, mtx._44 * Value);
}

//-----------------------------------------------------------------------------
// Name : Identity()
// Desc : Sets this matrix as an identity matrix
//-----------------------------------------------------------------------------
CMatrix4& CMatrix4::Identity() 
{
    _11 = 1; _12 = 0; _13 = 0; _14 = 0;
    _21 = 0; _22 = 1; _23 = 0; _24 = 0;
    _31 = 0; _32 = 0; _33 = 1; _34 = 0;
    _41 = 0; _42 = 0; _43 = 0; _44 = 1;

    return *this;
}

//-----------------------------------------------------------------------------
// Name : Zero()
// Desc : Clears all matrix values
//-----------------------------------------------------------------------------
CMatrix4& CMatrix4::Zero() 
{
    _11 = _12 = _13 = _14 = 0;
    _21 = _22 = _23 = _24 = 0;
    _31 = _32 = _33 = _34 = 0;
    _41 = _42 = _43 = _44 = 0;

    return *this;
}

//-----------------------------------------------------------------------------
// Name : Determinant ()
// Desc : Calculates the determinant value of this 4x4 matrix
//-----------------------------------------------------------------------------
float CMatrix4::Determinant( ) const
{
    return _11 * CalculateDeterminant3(_22, _32, _42, _23, _33, _43, _24, _34, _44)
         - _12 * CalculateDeterminant3(_21, _31, _41, _23, _33, _43, _24, _34, _44)
         + _13 * CalculateDeterminant3(_21, _31, _41, _22, _32, _42, _24, _34, _44)
         - _14 * CalculateDeterminant3(_21, _31, _41, _22, _32, _42, _23, _33, _43);
}

//-----------------------------------------------------------------------------
// Name : GetInverse ()
// Desc : Returns an inverted copy of the current matrix
//-----------------------------------------------------------------------------
CMatrix4 CMatrix4::GetInverse( float * pDeterminant ) const
{
    CMatrix4 InverseMatrix = *this;
    return InverseMatrix.Invert( pDeterminant );
}

//-----------------------------------------------------------------------------
// Name : Invert
// Desc : Generates the inverse of the current matrix, and stores.
//-----------------------------------------------------------------------------
CMatrix4& CMatrix4::Invert( float * pDeterminant )
{
    float det;
    float a1, b1, c1, d1;
    float a2, b2, c2, d2;
    float a3, b3, c3, d3;
    float a4, b4, c4, d4;

    if (pDeterminant == NULL) pDeterminant = &det;

    *pDeterminant = Determinant( );
    if (fabsf(*pDeterminant) < 1e-5f) return *this;

    det = 1.0f / (*pDeterminant);

    a1 = _11; b1 = _12; c1 = _13; d1 = _14;
    a2 = _21; b2 = _22; c2 = _23; d2 = _24;
    a3 = _31; b3 = _32; c3 = _33; d3 = _34;
    a4 = _41; b4 = _42; c4 = _43; d4 = _44;

    _11 =  CalculateDeterminant3(b2, b3, b4, c2, c3, c4, d2, d3, d4) * det;
    _21 = -CalculateDeterminant3(a2, a3, a4, c2, c3, c4, d2, d3, d4) * det;
    _31 =  CalculateDeterminant3(a2, a3, a4, b2, b3, b4, d2, d3, d4) * det;
    _41 = -CalculateDeterminant3(a2, a3, a4, b2, b3, b4, c2, c3, c4) * det;

    _12 = -CalculateDeterminant3(b1, b3, b4, c1, c3, c4, d1, d3, d4) * det;
    _22 =  CalculateDeterminant3(a1, a3, a4, c1, c3, c4, d1, d3, d4) * det;
    _32 = -CalculateDeterminant3(a1, a3, a4, b1, b3, b4, d1, d3, d4) * det;
    _42 =  CalculateDeterminant3(a1, a3, a4, b1, b3, b4, c1, c3, c4) * det;

    _13 =  CalculateDeterminant3(b1, b2, b4, c1, c2, c4, d1, d2, d4) * det;
    _23 = -CalculateDeterminant3(a1, a2, a4, c1, c2, c4, d1, d2, d4) * det;
    _33 =  CalculateDeterminant3(a1, a2, a4, b1, b2, b4, d1, d2, d4) * det;
    _43 = -CalculateDeterminant3(a1, a2, a4, b1, b2, b4, c1, c2, c4) * det;

    _14 = -CalculateDeterminant3(b1, b2, b3, c1, c2, c3, d1, d2, d3) * det;
    _24 =  CalculateDeterminant3(a1, a2, a3, c1, c2, c3, d1, d2, d3) * det;
    _34 = -CalculateDeterminant3(a1, a2, a3, b1, b2, b3, d1, d2, d3) * det;
    _44 =  CalculateDeterminant3(a1, a2, a3, b1, b2, b3, c1, c2, c3) * det;

    return *this;
}

bool CMatrix4::IsIdentity() const
{
    if ( _11 == 1.0f && _12 == 0.0f && _13 == 0.0f && _14 == 0.0f &&
         _21 == 0.0f && _22 == 1.0f && _23 == 0.0f && _24 == 0.0f &&
         _31 == 0.0f && _32 == 0.0f && _33 == 1.0f && _34 == 0.0f &&
         _41 == 0.0f && _42 == 0.0f && _43 == 0.0f && _44 == 1.0f ) return true;
    return false;
}

CMatrix4& CMatrix4::RotateAxis( const CVector3& vecAxis, const float& Angle, bool Reset )
{
    CMatrix4 * pMatrix, TempMatrix;
    float cosAngle, sinAngle, invCosAngle, x, y, z, mag;

    x = vecAxis.x; y = vecAxis.y; z = vecAxis.z;

    mag = x * x + y * y + z * z;
    if (fabsf(1.0f - mag) > 1e-5f) {
        mag = sqrtf(mag);
        if (fabsf(mag) < 1e-5f) return *this;
        mag = 1.0f / mag;
       
        x *= mag; y *= mag; z *= mag;
    } // End If

    cosAngle    = cosf(Angle);
    sinAngle    = sinf(Angle);
    invCosAngle = 1.0f - cosAngle;

    // If we are to reset
    if ( Reset ) {
        // Just set this to a rotation matrix
        pMatrix = this;
    } else {
        // Build a new rotation matrix
        pMatrix = &TempMatrix;
    } // End If Reset

    pMatrix->_11 = invCosAngle * x * x + cosAngle;
    pMatrix->_12 = invCosAngle * x * y + z * sinAngle;
    pMatrix->_13 = invCosAngle * x * z - y * sinAngle;
    pMatrix->_14 = 0.0f;

    pMatrix->_21 = invCosAngle * x * y - z * sinAngle;
    pMatrix->_22 = invCosAngle * y * y + cosAngle;
    pMatrix->_23 = invCosAngle * y * z + x * sinAngle;
    pMatrix->_24 = 0.0f;

    pMatrix->_31 = invCosAngle * x * z + y * sinAngle;
    pMatrix->_32 = invCosAngle * y * z - x * sinAngle;
    pMatrix->_33 = invCosAngle * z * z + cosAngle;
    pMatrix->_34 = 0.0f;

    pMatrix->_41 = 0.0f; pMatrix->_42 = 0.0f; pMatrix->_43 = 0.0f; pMatrix->_44 = 1.0f;

    // If we are rotating current values, multiply it by the TempMatrix
    if ( !Reset ) { *this *= *pMatrix; }

    return *this;
}

CMatrix4& CMatrix4::RotateX( const float& Angle, bool Reset )
{
    CMatrix4 * pMatrix, TempMatrix;
    float cosAngle, sinAngle;

    cosAngle = cosf(Angle);
    sinAngle = sinf(Angle);
    
    // If we are to reset
    if ( Reset ) {
        // Just set this to a rotation matrix
        pMatrix = this;
    } else {
        // Build a new rotation matrix
        pMatrix = &TempMatrix;
    } // End If Reset

    pMatrix->Identity();
    pMatrix->_22 =  cosAngle; pMatrix->_23 = sinAngle;
    pMatrix->_32 = -sinAngle; pMatrix->_33 = cosAngle;

    // If we are rotating current values, multiply it by the TempMatrix
    if ( !Reset ) { *this *= *pMatrix; }

    return *this;
}

CMatrix4& CMatrix4::RotateY( const float& Angle, bool Reset )
{
    CMatrix4 * pMatrix, TempMatrix;
    float cosAngle, sinAngle;

    cosAngle = cosf(Angle);
    sinAngle = sinf(Angle);
    
    // If we are to reset
    if ( Reset ) {
        // Just set this to a rotation matrix
        pMatrix = this;
    } else {
        // Build a new rotation matrix
        pMatrix = &TempMatrix;
    } // End If Reset

    pMatrix->Identity();
    pMatrix->_11 = cosAngle; pMatrix->_13 =-sinAngle;
    pMatrix->_31 = sinAngle; pMatrix->_33 = cosAngle;

    // If we are rotating current values, multiply it by the TempMatrix
    if ( !Reset ) { *this *= *pMatrix; }

    return *this;
}

CMatrix4& CMatrix4::RotateZ( const float& Angle, bool Reset )
{
    CMatrix4 * pMatrix, TempMatrix;
    float cosAngle, sinAngle;

    cosAngle = cosf(Angle);
    sinAngle = sinf(Angle);
    
    // If we are to reset
    if ( Reset ) {
        // Just set this to a rotation matrix
        pMatrix = this;
    } else {
        // Build a new rotation matrix
        pMatrix = &TempMatrix;
    } // End If Reset

    pMatrix->Identity();
    pMatrix->_11 =  cosAngle; pMatrix->_12 = sinAngle;
    pMatrix->_21 = -sinAngle; pMatrix->_22 = cosAngle;

    // If we are rotating current values, multiply it by the TempMatrix
    if ( !Reset ) { *this *= *pMatrix; }

    return *this;
}


CMatrix4& CMatrix4::Rotate( const float& Yaw, const float& Pitch, const float& Roll, bool Reset )
{
    CMatrix4 * pMatrix, TempMatrix;
    float sinY, cosY, sinP, cosP, sinR, cosR;
    float ux, uy, uz, vx, vy, vz, nx, ny, nz;

    sinY = sinf(Yaw);
    cosY = cosf(Yaw);

    sinP = sinf(Pitch);
    cosP = cosf(Pitch);

    sinR = sinf(Roll);
    cosR = cosf(Roll);

    ux = cosY * cosR + sinY * sinP * sinR;
    uy = sinR * cosP;
    uz = cosY * sinP * sinR - sinY * cosR;

    vx = sinY * sinP * cosR - cosY * sinR;
    vy = cosR * cosP;
    vz = sinR * sinY + cosR * cosY * sinP;

    nx = cosP * sinY;
    ny = -sinP;
    nz = cosP * cosY;

    // If we are to reset
    if ( Reset ) {
        // Just set this to a rotation matrix
        pMatrix = this;
    } else {
        // Build a new rotation matrix
        pMatrix = &TempMatrix;
    } // End If Reset

    pMatrix->_11 =   ux; pMatrix->_12 =   uy; pMatrix->_13 =   uz; pMatrix->_14 = 0.0f; 
    pMatrix->_21 =   vx; pMatrix->_22 =   vy; pMatrix->_23 =   vz; pMatrix->_24 = 0.0f; 
    pMatrix->_31 =   nx; pMatrix->_32 =   ny; pMatrix->_33 =   nz; pMatrix->_34 = 0.0f; 
    pMatrix->_41 = 0.0f; pMatrix->_42 = 0.0f; pMatrix->_43 = 0.0f; pMatrix->_44 = 1.0f; 

    // If we are rotating current values, multiply it by the TempMatrix
    if ( !Reset ) { *this *= *pMatrix; }

    return *this;
}


//-----------------------------------------------------------------------------
// Name : Scale
// Desc : Scales the matrix by values specified
//-----------------------------------------------------------------------------
CMatrix4& CMatrix4::Scale( const float& X, const float& Y, const float& Z, bool Reset )
{
    if (Reset) {
        Identity();
        _11 = X; _22 = Y; _33 = Z;
    } else {
        // Scale matrix values
	    _11 *= X; _21 *= X; _31 *= X; _41 *= X;
        _12 *= Y; _22 *= Y; _32 *= Y; _42 *= Y;
        _13 *= Z; _23 *= Z; _33 *= Z; _43 *= Z;
    } // End If

    return *this;

}

CMatrix4& CMatrix4::Scale( const float& X, const float& Y, const float& Z, const CVector3& Origin, bool Reset )
{
    if (Reset) Identity();

    // Translate from arbitrary origin
	Translate( -Origin.x, -Origin.y, -Origin.z );

	// Scale matrix values
	_11 *= X; _21 *= X; _31 *= X; _41 *= X;
    _12 *= Y; _22 *= Y; _32 *= Y; _42 *= Y;
    _13 *= Z; _23 *= Z; _33 *= Z; _43 *= Z;

    // Translate back to pre-scale origin
    Translate( Origin.x, Origin.y, Origin.z );

    return *this;
}

CMatrix4& CMatrix4::Translate( const float& X, const float& Y, const float& Z, bool Reset )
{
    // Set / Increment / Decrement translation values
    if (Reset) {
        Identity();
        _41 = X; _42 = Y; _43 = Z;
    } else {
        _11 += _14 * X; _21 += _24 * X; _31 += _34 * X; _41 += _44 * X;
        _12 += _14 * Y; _22 += _24 * Y; _32 += _34 * Y; _42 += _44 * Y;
        _13 += _14 * Z; _23 += _24 * Z; _33 += _34 * Z; _43 += _44 * Z;
    } // End If

    return *this;
}
    
CMatrix4& CMatrix4::Transpose( )
{
    float fTemp;

    // Transpose Values
    fTemp = _12; _12 = _21; _21 = fTemp;
    fTemp = _13; _13 = _31; _31 = fTemp;
    fTemp = _14; _14 = _41; _41 = fTemp;

    fTemp = _23; _23 = _32; _32 = fTemp;
    fTemp = _24; _24 = _42; _42 = fTemp;
    fTemp = _34; _34 = _43; _43 = fTemp;

    return *this;
}

bool CMatrix4::SetLookAtLH( const CVector3& vecPos, const CVector3& vecLookAt, const CVector3& vecUp )
{
    CVector3   uAxis, vAxis, nAxis;

    // Obtain look direction
    nAxis = (vecLookAt - vecPos);
    nAxis.Normalize();

    // Obtain best up vector.
    vAxis = vecUp - nAxis * vecUp.Dot(nAxis);

    // Ensure the vAxis is valid.
    if (vAxis.Length() < 1e-5f) {
        vAxis.x =      - nAxis.y * nAxis.x;
        vAxis.y = 1.0f - nAxis.y * nAxis.y;
        vAxis.z =      - nAxis.y * nAxis.z;
        if ( vAxis.Length() < 1e-5f ) {
            vAxis.x =      - nAxis.z * nAxis.x;
            vAxis.y =      - nAxis.z * nAxis.y;
            vAxis.z = 1.0f - nAxis.z * nAxis.z;
            // No Chance, Bail.
            if (vAxis.Length() < 1e-5f) return false;
        } // End If
    } // End If

    // Normalize and obtain v Axis
    vAxis.Normalize();
    uAxis = vAxis.Cross( nAxis );
    
    // Set Matrix Values
    _11 = uAxis.x; _12 = vAxis.x; _13 = nAxis.x; _14 = 0.0f;
    _21 = uAxis.y; _22 = vAxis.y; _23 = nAxis.y; _24 = 0.0f;
    _31 = uAxis.z; _32 = vAxis.z; _33 = nAxis.z; _34 = 0.0f;
    _41 = -vecPos.Dot(uAxis);
    _42 = -vecPos.Dot(vAxis);
    _43 = -vecPos.Dot(nAxis);
    _44 = 1.0f;

    return true;
}

bool CMatrix4::SetLookAtRH( const CVector3& vecPos, const CVector3& vecLookAt, const CVector3& vecUp )
{
    CVector3   uAxis, vAxis, nAxis;

    // Obtain look direction
    nAxis = -(vecLookAt - vecPos);
    nAxis.Normalize();

    // Obtain best up vector.
    vAxis = vecUp - nAxis * vecUp.Dot(nAxis);

    // Ensure the vAxis is valid.
    if (vAxis.Length() < 1e-5f) {
        vAxis.x =      - nAxis.y * nAxis.x;
        vAxis.y = 1.0f - nAxis.y * nAxis.y;
        vAxis.z =      - nAxis.y * nAxis.z;
        if ( vAxis.Length() < 1e-5f ) {
            vAxis.x =      - nAxis.z * nAxis.x;
            vAxis.y =      - nAxis.z * nAxis.y;
            vAxis.z = 1.0f - nAxis.z * nAxis.z;
            // No Chance, Bail.
            if (vAxis.Length() < 1e-5f) return false;
        } // End If
    } // End If

    // Normalize and obtain v Axis
    vAxis.Normalize();
    uAxis = vAxis.Cross( nAxis );
    
    // Set Matrix Values
    _11 = uAxis.x; _12 = vAxis.x; _13 = nAxis.x; _14 = 0.0f;
    _21 = uAxis.y; _22 = vAxis.y; _23 = nAxis.y; _24 = 0.0f;
    _31 = uAxis.z; _32 = vAxis.z; _33 = nAxis.z; _34 = 0.0f;
    _41 = -vecPos.Dot(uAxis);
    _42 = -vecPos.Dot(vAxis);
    _43 = -vecPos.Dot(nAxis);
    _44 = 1.0f;

    return true;
}

bool CMatrix4::SetOrthoLH( const float& Width, const float& Height, const float& NearPlane, const float& FarPlane )
{
    float nfRatio;

    if (fabsf(Width) < 1e-2f) return false;
    if (fabsf(Height) < 1e-2f) return false;

    nfRatio = FarPlane - NearPlane;
    if (fabsf(nfRatio) < 1e-2f) return false;
    nfRatio = 1.0f / nfRatio;

    _11 = 2.0f / Width;
    _22 = 2.0f / Height;
    _33 = nfRatio;
    _43 = -nfRatio * NearPlane;
    _44 = 1.0f;

    _12 = 0.0f; _13 = 0.0f; _14 = 0.0f;
    _21 = 0.0f; _23 = 0.0f; _24 = 0.0f;
    _31 = 0.0f; _32 = 0.0f; _34 = 0.0f;
    _41 = 0.0f; _42 = 0.0f;

    return true;
}

bool CMatrix4::SetOrthoRH( const float& Width, const float& Height, const float& NearPlane, const float& FarPlane )
{
    float nfRatio;

    if (fabsf(Width) < 1e-2f) return false;
    if (fabsf(Height) < 1e-2f) return false;

    nfRatio = FarPlane - NearPlane;
    if (fabsf(nfRatio) < 1e-2f) return false;
    nfRatio = 1.0f / nfRatio;

    _11 = 2.0f / Width;
    _22 = 2.0f / Height;
    _33 = -nfRatio;
    _43 = -nfRatio * NearPlane;
    _44 = 1.0f;

    _12 = 0.0f; _13 = 0.0f; _14 = 0.0f;
    _21 = 0.0f; _23 = 0.0f; _24 = 0.0f;
    _31 = 0.0f; _32 = 0.0f; _34 = 0.0f;
    _41 = 0.0f; _42 = 0.0f;

    return true;
}

bool CMatrix4::SetOrthoOffCenterLH( const float& Left, const float& Right, const float& Bottom, const float& Top, const float& NearPlane, const float& FarPlane )
{
    float nfRatio, width, height;

    width  = Right - Left;
    height = Bottom - Top;

    if (fabsf(width)  < 1e-2f) return false;
    if (fabsf(height) < 1e-2f) return false;

    nfRatio = FarPlane - NearPlane;
    if (fabsf(nfRatio) < 1e-2f) return false;
    nfRatio = 1.0f / nfRatio;

    _11 = 2.0f / width;
    _22 = 2.0f / height;
    _33 = nfRatio;
    _41 = -(Left + Right) / width;
    _42 = -(Top + Bottom) / height;
    _43 = -nfRatio * NearPlane;
    _44 = 1.0f;

    _12 = 0.0f; _13 = 0.0f; _14 = 0.0f;
    _21 = 0.0f; _23 = 0.0f; _24 = 0.0f;
    _31 = 0.0f; _32 = 0.0f; _34 = 0.0f;

    return true;
}
    
bool CMatrix4::SetOrthoOffCenterRH( const float& Left, const float& Right, const float& Bottom, const float& Top, const float& NearPlane, const float& FarPlane )
{
    float nfRatio, width, height;

    width  = Right - Left;
    height = Bottom - Top;

    if (fabsf(width)  < 1e-2f) return false;
    if (fabsf(height) < 1e-2f) return false;

    nfRatio = FarPlane - NearPlane;
    if (fabsf(nfRatio) < 1e-2f) return false;
    nfRatio = 1.0f / nfRatio;

    _11 = 2.0f / width;
    _22 = 2.0f / height;
    _33 = -nfRatio;
    _41 = -(Left + Right) / width;
    _42 = -(Top + Bottom) / height;
    _43 = -nfRatio * NearPlane;
    _44 = 1.0f;

    _12 = 0.0f; _13 = 0.0f; _14 = 0.0f;
    _21 = 0.0f; _23 = 0.0f; _24 = 0.0f;
    _31 = 0.0f; _32 = 0.0f; _34 = 0.0f;

    return true;
}

//-----------------------------------------------------------------------------
// Name : SetProjectionFovLH()
// Desc : Fills this matrix with a left handed perspective projection matrix
//-----------------------------------------------------------------------------
bool CMatrix4::SetPerspectiveFovLH( const float& FovY, const float& Aspect, const float& NearPlane, const float& FarPlane ) 
{
    float nfRatio, cotFovY, fFOV;

    nfRatio = FarPlane - NearPlane;
    if (fabsf(nfRatio) < 1e-2f) return false;
    nfRatio = FarPlane / nfRatio;

    fFOV = FovY * 0.5f;
    cotFovY = sinf(fFOV);
    if (fabsf(cotFovY) < 1e-5f) return false;
    cotFovY = cosf(fFOV) / cotFovY;

    _11 = cotFovY / Aspect;
    _22 = cotFovY;
    _33 = nfRatio;
    _34 = 1.0f;
    _43 = -nfRatio * NearPlane;

    _12 = 0.0f; _13 = 0.0f; _14 = 0.0f;
    _21 = 0.0f; _23 = 0.0f; _24 = 0.0f;
    _31 = 0.0f; _32 = 0.0f;
    _41 = 0.0f; _42 = 0.0f; _44 = 0.0f;

    return true;
}

//-----------------------------------------------------------------------------
// Name : SetProjectionFovRH()
// Desc : Fills this matrix with a right handed perspective projection matrix
//-----------------------------------------------------------------------------
bool CMatrix4::SetPerspectiveFovRH( const float& FovY, const float& Aspect, const float& NearPlane, const float& FarPlane )
{
    float nfRatio, cotFovY, fFOV;

    nfRatio = FarPlane - NearPlane;
    if (fabsf(nfRatio) < 1e-2f) return false;
    nfRatio = FarPlane / nfRatio;

    fFOV = FovY * 0.5f;
    cotFovY = sinf(fFOV);
    if (fabsf(cotFovY) < 1e-5f) return false;
    cotFovY = cosf(fFOV) / cotFovY;

    _11 = cotFovY / Aspect;
    _22 = cotFovY;
    _33 = -nfRatio;
    _34 = -1.0f;
    _43 = -nfRatio * NearPlane;

    _12 = 0.0f; _13 = 0.0f; _14 = 0.0f;
    _21 = 0.0f; _23 = 0.0f; _24 = 0.0f;
    _31 = 0.0f; _32 = 0.0f;
    _41 = 0.0f; _42 = 0.0f; _44 = 0.0f;

    return true;
}

bool CMatrix4::SetPerspectiveLH( const float& Width, const float& Height, const float& NearPlane, const float& FarPlane )
{
    return true;
}

bool CMatrix4::SetPerspectiveRH( const float& Width, const float& Height, const float& NearPlane, const float& FarPlane )
{
    return true;
}

bool CMatrix4::SetPerspectiveOffCenterLH( const float& Left, const float& Right, const float& Bottom, const float& Top, const float& NearPlane, const float& FarPlane )
{
    return true;
}

bool CMatrix4::SetPerspectiveOffCenterRH( const float& Left, const float& Right, const float& Bottom, const float& Top, const float& NearPlane, const float& FarPlane )
{
    return true;
}

bool CMatrix4::SetReflect( const CPlane3& Plane )
{
    return true;
}

bool CMatrix4::SetShadow( const CVector4& Light, const CPlane3& Plane )
{
    return true;
}

//-----------------------------------------------------------------------------
// Name : SetViewLH()
// Desc : Fills this matrix with a view matrix
//-----------------------------------------------------------------------------
bool CMatrix4::SetViewLH( const CVector3& vecPos, const CVector3& vecLookDir, const CVector3& vecUpDir, const CVector3& vecRightDir ) 
{

    // Identity this matrix before update
    Identity();

    // Update View Matrix
    _11 = vecRightDir.x; _12 = vecUpDir.x; _13 = vecLookDir.x;
	_21 = vecRightDir.y; _22 = vecUpDir.y; _23 = vecLookDir.y;
	_31 = vecRightDir.z; _32 = vecUpDir.z; _33 = vecLookDir.z;
	_41 =- vecPos.Dot( vecRightDir );
	_42 =- vecPos.Dot( vecUpDir );
	_43 =- vecPos.Dot( vecLookDir );

    return true;
}

//-----------------------------------------------------------------------------
// Name : CalculateDeterminant3 (Private Static)
// Desc : Calculates the determinant value of this 3x3 matrix
//-----------------------------------------------------------------------------
float CMatrix4::CalculateDeterminant3( float _11, float _12, float _13, float _21, float _22, float _23, float _31, float _32, float _33)
{
    return _11 * (_22 * _33 - _23 * _32) - _12 * (_21 * _33 - _23 * _31) + _13 * (_21 * _32 - _22 * _31);
}

//-----------------------------------------------------------------------------
// Name : GetIdentity() (Static)
// Desc : Static function which returns a base identity matrix.
//-----------------------------------------------------------------------------
const CMatrix4& CMatrix4::GetIdentity()
{
    return m_mtxStaticIdentity;
}