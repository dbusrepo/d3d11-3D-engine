#ifndef _PROCESSHSR_H_
#define _PROCESSHSR_H_

//-----------------------------------------------------------------------------
// CProcessHSR Specific Includes
//-----------------------------------------------------------------------------
#include <vector>
#include "CompilerTypes.h"
#include "..\\Support Source\\Common.h"

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class CBSPTree;
class CMesh;
class CCompiler;

//-----------------------------------------------------------------------------
// Miscellaneous Definitions
//-----------------------------------------------------------------------------
#define HSR_ARRAY_THRESHOLD     100

//-----------------------------------------------------------------------------
// Typedefs utilised for shorthand versions of our STL types.
//-----------------------------------------------------------------------------
typedef std::vector<CMesh*>     vectorMesh;
typedef std::vector<CBSPTree*>  vectorBSPTree;

//-----------------------------------------------------------------------------
// Main Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CProcessHSR (Class)
// Desc : This is the primary hidden surface removal process class which takes 
//        the information stored in one or more meshes, and build a mesh result 
//        with all hidden, or intersecting, geometry removed.
//-----------------------------------------------------------------------------
class CProcessHSR
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class.
    //-------------------------------------------------------------------------
             CProcessHSR();
    virtual ~CProcessHSR();

    //-------------------------------------------------------------------------
    // Public Functions for This Class.
    //-------------------------------------------------------------------------
    void            SetOptions( const HSROPTIONS& Options ) { m_OptionSet = Options; }
    void            SetLogger ( ILogger * pLogger )         { m_pLogger = pLogger; }
    void            SetParent ( CCompiler * pParent )       { m_pParent = pParent; }

    bool            AddMesh( CMesh * Mesh );
    CMesh          *GetResultMesh() const;
    HRESULT         Process();
    void            Release();

private:
    //-------------------------------------------------------------------------
    // Private Variables for This Class.
    //-------------------------------------------------------------------------
    vectorMesh      m_vpMeshList;       // A list of all meshes for HSR Processing.
    CMesh          *m_pResultMesh;      // The single mesh result after HSR processing

    HSROPTIONS      m_OptionSet;        // The option set for HSR Processing
    ILogger        *m_pLogger;          // Just our logging interface used to log progress etc.
    CCompiler      *m_pParent;          // Parent Compiler Pointer

};

#endif // _PROCESSHSR_H_