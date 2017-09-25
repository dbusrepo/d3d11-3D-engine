#ifndef _CCOMPILER_H_
#define _CCOMPILER_H_

//-----------------------------------------------------------------------------
// CCompiler Specific Includes
//-----------------------------------------------------------------------------
#include "..\\Support Source\\Common.h"
#include "..\\Compiler Source\\CompilerTypes.h"
#include "CLevelFile.h"
#include <vector>

//-----------------------------------------------------------------------------
// Typedefs Structures & Enumerators
//-----------------------------------------------------------------------------
typedef std::vector<CMesh*>      vectorMesh;
//typedef std::vector<iwfTexture*>  vectorTexture;
//typedef std::vector<iwfMaterial*> vectorMaterial;
//typedef std::vector<iwfEntity*>   vectorEntity;
//typedef std::vector<iwfShader*>   vectorShader;

enum COMPILESTATUS
{
    CS_IDLE         = 0,
    CS_INPROGRESS   = 1,
    CS_PAUSED       = 2,
    CS_CANCELLED    = 3
};

//-----------------------------------------------------------------------------
// Main Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CCompiler (Class)
// Desc : Main compiler class, handles the various tasks involved with
//        initialising and starting each compilation process.
//-----------------------------------------------------------------------------
class CCompiler
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class.
    //-------------------------------------------------------------------------
             CCompiler();
    virtual ~CCompiler();

    //-------------------------------------------------------------------------
    // Public Functions for This Class.
    //-------------------------------------------------------------------------
    bool            CompileScene     ( );
    void            Release          ( );
    void            SetFile          ( LPCTSTR FileName );
    void            SetOptions       ( UINT Process, const LPVOID Options );
    void            GetOptions       ( UINT Process, LPVOID Options ) const;
    void            SetLogger        ( ILogger * pLogger ) { m_pLogger = pLogger; }
    CBSPTree       *GetBSPTree       ( ) const { return m_pBSPTree;   }
    bool            SaveScene        ( LPCTSTR FileName );
    
    void            PauseCompiler    ( );
    void            ResumeCompiler   ( );
    void            StopCompiler     ( );
    bool            TestCompilerState( );

    COMPILESTATUS   GetCompileStatus ( ) const { return m_Status; }
    void            SetCompileStatus ( COMPILESTATUS Status ) { m_Status = Status; }
    

    //-------------------------------------------------------------------------
    // Public Variables for This Class.
    //-------------------------------------------------------------------------
    vectorMesh      m_vpMeshList;       // A list of all meshes loaded.
    //vectorTexture   m_vpTextureList;    // A list of all textures loaded.
    //vectorMaterial  m_vpMaterialList;   // A list of all materials loaded.
    //vectorEntity    m_vpEntityList;     // A list of all entities loaded.
    //vectorShader    m_vpShaderList;     // A list of all shaders loaded.

private:
    //-------------------------------------------------------------------------
    // Private Functions for This Class.
    //-------------------------------------------------------------------------
    bool            PerformHSR( );      // Hidden Surface Removal
    bool            PerformBSP( );      // Binary Space Partition Compilation
    bool            PerformPRT( );      // Portal Compilation
    bool            PerformPVS( );      // Potential Visibility Set Compilation
    bool            PerformTJR( );      // T-Junction Repair
	bool			PerformLMP();		// LightMapping
    //-------------------------------------------------------------------------
    // Private Variables for This Class.
    //-------------------------------------------------------------------------
    HSROPTIONS      m_OptionsHSR;       // Hidden Surface Removal Options
    BSPOPTIONS      m_OptionsBSP;       // BSP Compilation Options
    PRTOPTIONS      m_OptionsPRT;       // Portal Compilation Options
    PVSOPTIONS      m_OptionsPVS;       // PVS Compilation Options
    TJROPTIONS      m_OptionsTJR;       // T-Junction Repair Options
	LIGHTMAPOPTIONS m_OptionLightmapping;

    ILogger        *m_pLogger;          // Just our logging interface used to log progress etc.
    LPTSTR          m_strFileName;      // The file used for compilation
    COMPILESTATUS   m_Status;           // The current status of the compile run
    ULONG           m_CurrentLog;       // Current logging channel for messages.

    CBSPTree       *m_pBSPTree;         // Our compiled BSP Tree.

	CLevelFile		m_Level;			// file loader
    

};

#endif // _CCOMPILER_H_