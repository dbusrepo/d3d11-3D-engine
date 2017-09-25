#ifndef _CMATRIX_H_
#define _CMATRIX_H_

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class CVector4;
class CVector3;
class CPlane3;

//-----------------------------------------------------------------------------
// Miscellaneous Defines
//-----------------------------------------------------------------------------
#ifndef NULL
#define NULL 0
#endif

//-----------------------------------------------------------------------------
// Main class definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CMatrix4 (Class)
// Desc : Matrix class. Storage for matrix values and wraps up matrix math.
//-----------------------------------------------------------------------------
class CMatrix4
{
public:
	
    //------------------------------------------------------------
	// Constructors / Destructors for this Class
	//------------------------------------------------------------
    CMatrix4( );
    CMatrix4( const CMatrix4& mtx );
    CMatrix4( float _m11, float _m12, float _m13, float _m14,
              float _m21, float _m22, float _m23, float _m24,
              float _m31, float _m32, float _m33, float _m34,
              float _m41, float _m42, float _m43, float _m44);

	//------------------------------------------------------------
	// Public Functions For This Class
	//------------------------------------------------------------
    float       Determinant() const;
    CMatrix4&   Zero();
    CMatrix4&   Identity();
    CMatrix4    GetInverse( float * pDeterminant = NULL ) const;
    CMatrix4&   Invert( float * pDeterminant = NULL );
    bool        IsIdentity() const;
    CMatrix4&   RotateAxis( const CVector3& vecAxis, const float& Angle, bool Reset = false );
    CMatrix4&   RotateX( const float& Angle, bool Reset = false );
    CMatrix4&   RotateY( const float& Angle, bool Reset = false );
    CMatrix4&   RotateZ( const float& Angle, bool Reset = false );
    CMatrix4&   Rotate( const float& Yaw, const float& Pitch, const float& Roll, bool Reset = false );
    CMatrix4&   Scale( const float& X, const float& Y, const float& Z, bool Reset = false );
    CMatrix4&   Scale( const float& X, const float& Y, const float& Z, const CVector3& Origin, bool Reset = false );
    CMatrix4&   Translate( const float& X, const float& Y, const float& Z, bool Reset = false );
    CMatrix4&   Transpose( );

    bool        SetViewLH( const CVector3& vecPos, const CVector3& vecLookDir, const CVector3& vecUpDir, const CVector3& vecRightDir );
    bool        SetLookAtLH( const CVector3& vecPos, const CVector3& vecLookAt, const CVector3& vecUp );
    bool        SetLookAtRH( const CVector3& vecPos, const CVector3& vecLookAt, const CVector3& vecUp );
    bool        SetOrthoLH( const float& Width, const float& Height, const float& NearPlane, const float& FarPlane );
    bool        SetOrthoRH( const float& Width, const float& Height, const float& NearPlane, const float& FarPlane );
    bool        SetOrthoOffCenterLH( const float& Left, const float& Right, const float& Bottom, const float& Top, const float& NearPlane, const float& FarPlane );
    bool        SetOrthoOffCenterRH( const float& Left, const float& Right, const float& Bottom, const float& Top, const float& NearPlane, const float& FarPlane );
    bool        SetPerspectiveFovLH( const float& FovY, const float& Aspect, const float& NearPlane, const float& FarPlane );
    bool        SetPerspectiveFovRH( const float& FovY, const float& Aspect, const float& NearPlane, const float& FarPlane );
    bool        SetPerspectiveLH( const float& Width, const float& Height, const float& NearPlane, const float& FarPlane );
    bool        SetPerspectiveRH( const float& Width, const float& Height, const float& NearPlane, const float& FarPlane );
    bool        SetPerspectiveOffCenterLH( const float& Left, const float& Right, const float& Bottom, const float& Top, const float& NearPlane, const float& FarPlane );
    bool        SetPerspectiveOffCenterRH( const float& Left, const float& Right, const float& Bottom, const float& Top, const float& NearPlane, const float& FarPlane );
    bool        SetReflect( const CPlane3& Plane );
    bool        SetShadow( const CVector4& Light, const CPlane3& Plane );

    //------------------------------------------------------------
	// Public Static Functions For This Class
    //------------------------------------------------------------
    static const CMatrix4& GetIdentity();

    //------------------------------------------------------------
	// Public Operators For This Class
    //------------------------------------------------------------
    float&          operator() (int iRow, int iColumn) ;
    float           operator() (int iRow, int iColumn) const;
    CMatrix4        operator*  (const CMatrix4 & mtx ) const;
    CMatrix4        operator+  (const CMatrix4 & mtx ) const;
    CMatrix4        operator-  (const CMatrix4 & mtx ) const;
    CMatrix4        operator/  (const float& Value   ) const;
    CMatrix4        operator*  (const float& Value   ) const;
    CMatrix4&       operator*= (const CMatrix4 & mtx );
    CMatrix4&       operator+= (const CMatrix4 & mtx );
    CMatrix4&       operator-= (const CMatrix4 & mtx );
    CMatrix4&       operator/= (const float & Value  );
    CMatrix4&       operator*= (const float & Value  );
    CMatrix4        operator-  () const;
    CMatrix4        operator+  () const;
    bool            operator== (const CMatrix4 & mtx ) const;
    bool            operator!= (const CMatrix4 & mtx ) const;
                    operator const float* () const;
                    operator       float* ();

    //------------------------------------------------------------
	// Public Friend Operators For This Class
    //------------------------------------------------------------
    friend CMatrix4 operator* (float Value, const CMatrix4& mtx);

    //------------------------------------------------------------
	// Public Variables For This Class
	//------------------------------------------------------------
    union  {
        struct {
            float _11, _12, _13, _14;
            float _21, _22, _23, _24;
            float _31, _32, _33, _34;
            float _41, _42, _43, _44;
        }; // End Struct
        float m[4][4];
    }; // End Union

private:
	//------------------------------------------------------------
	// Private Static Functions for This Class
	//------------------------------------------------------------
    static float CalculateDeterminant3( float _11, float _12, float _13, float _21, float _22, float _23, float _31, float _32, float _33);

    //------------------------------------------------------------
	// Private Static Variables For This Class
	//------------------------------------------------------------
    static CMatrix4     m_mtxStaticIdentity;

};

#endif // _CMATRIX_H_
