//-----------------------------------------------------------------------------
// File: CCompiler.cpp
//
// Desc: The CCompiler class is an all encompassing class which manages all the
//       different processes involved in the scene pre-process.
//
//-----------------------------------------------------------------------------
// CCompiler Specific Includes
//-----------------------------------------------------------------------------
#include <algorithm>
#include <memory>
#include <cstdint>
#include <cstdio>
#include <cwchar>
#include "..\\Support Source\\Common.h"
#include "CCompiler.h"
#include "CompilerTypes.h"
#include "ProcessHSR.h"
#include "ProcessPRT.h"
#include "ProcessPVS.h"
#include "ProcessTJR.h"
#include "CBSPTree.h"
//lightmapping
#include "..\\LightMapper Source\\Vector.h"
#include "..\\LightMapper Source\\TexturePacker.h"
#include "..\\LightMapper Source\\BSP.h"
#include "..\\LightMapper Source\\Imaging\\Image.h"

#define LIGHT_CLUSTER_MAPS_FOLDER "Content\\LightClusterMaps"

float clamp(const float v, const float c0, const float c1) {
	return min(max(v, c0), c1);
}
#define saturate(x) clamp(x, 0, 1)

//-----------------------------------------------------------------------------
// Name : CCompiler () (Constructor)
// Desc : CCompiler Class Constructor
//-----------------------------------------------------------------------------
CCompiler::CCompiler()
{
    // Set up default HSR options
    m_OptionsHSR.Enabled            = true;

    // Set up default BSP options
    m_OptionsBSP.Enabled            = true;
    m_OptionsBSP.TreeType           = BSP_TYPE_SPLIT;
    m_OptionsBSP.SplitHeuristic     = 3.0f;
    m_OptionsBSP.SplitterSample     = 60;
    m_OptionsBSP.RemoveBackLeaves   = true;
    m_OptionsBSP.AddBoundingPolys   = false;

    // Set up default Portal Compile Options
	m_OptionsPRT.Enabled			= true; //true;

    // Set up default PVS Options
	m_OptionsPVS.Enabled			= true;//true;
    m_OptionsPVS.FullCompile        = true;//true
    m_OptionsPVS.ClipTestCount      = 3; // 3 //1(loose PVS,fast),4(tightest PVS,slooow)

    // Set up default TJR Options
    m_OptionsTJR.Enabled            = true;

	// Lightmapping options
	m_OptionLightmapping.Enabled = true;
	m_OptionLightmapping.lightmap_res_factor = 16;
	m_OptionLightmapping.clustermap_lightmap_factor = 2;
	m_OptionLightmapping.lm_width = 1024;// 1024;
	m_OptionLightmapping.lm_height = 1024; // 1024;
	m_OptionLightmapping.sampleCount = 150; // 250
	m_OptionLightmapping.sampleDistanceFactor = 150.f;// 150.f;// 50.f; // 50.f;

    // Reset Vars
    m_strFileName = NULL;
    m_pBSPTree    = NULL;
    m_pLogger     = NULL;
    m_Status      = CS_IDLE;
    
}

//-----------------------------------------------------------------------------
// Name : ~CCompiler () (Destructor)
// Desc : CCompiler Class Destructor
//-----------------------------------------------------------------------------
CCompiler::~CCompiler()
{
    // Clean up after ourselves
    Release();
}

//-----------------------------------------------------------------------------
// Name : PauseCompiler( )
// Desc : Called by the application to instruct the compiler to enter a paused
//        state, during which no compilation should take place.
// Note : This could be done at the thread level, but because the compiler is
//        not physically aware that it is part of any particular thread (or 
//        which type of thread), we must implement internal pause ability.
//-----------------------------------------------------------------------------
void CCompiler::PauseCompiler( )
{
    if ( m_Status != CS_INPROGRESS ) return;
    m_Status = CS_PAUSED;
    
    // Write Log Info
    if ( m_pLogger ) 
    {
        // Rewind and write 'pausing' message
        m_pLogger->Rewind( m_CurrentLog );
        m_pLogger->LogWrite( m_CurrentLog, LOGF_WARNING | LOGF_ITALIC, false, _T("Pausing..."));
    
    } // End if Logger Available
}

//-----------------------------------------------------------------------------
// Name : ResumeCompiler( )
// Desc : Called by the application to bring the compiler out of a paused state.
//-----------------------------------------------------------------------------
void CCompiler::ResumeCompiler( )
{
    if ( m_Status != CS_PAUSED ) return;
    m_Status = CS_INPROGRESS;

    // Write Log Info
    if ( m_pLogger ) 
    {
        // Rewind and write 'resuming' message
        m_pLogger->Rewind( m_CurrentLog );
        m_pLogger->LogWrite( m_CurrentLog, LOGF_WARNING | LOGF_ITALIC, false, _T("Please Wait..."));

    } // End if Logger Available
}

//-----------------------------------------------------------------------------
// Name : StopCompiler( )
// Desc : Called by the application to stop the  compilation process
//-----------------------------------------------------------------------------
void CCompiler::StopCompiler( )
{
    if ( m_Status == CS_IDLE ) return;
    m_Status = CS_CANCELLED;

    // Write Log Info
    if ( m_pLogger ) 
    {
        // Auto switch to progress channel
        m_pLogger->UpdateProgress();
    
        // Rewind and write 'pausing' message
        m_pLogger->Rewind( m_CurrentLog );
        m_pLogger->LogWrite( m_CurrentLog, LOGF_WARNING | LOGF_ITALIC, false, _T("Cancelling..."));

    } // End if Logger Available
}

//-----------------------------------------------------------------------------
// Name : TestCompilerState( )
// Desc : Called by the individual compiler processes to enter / exit paused
//        state, and also returns false if the process should terminate.
//-----------------------------------------------------------------------------
bool CCompiler::TestCompilerState( )
{
    if ( m_Status == CS_PAUSED )
    {
        // Write Log Info
        if ( m_pLogger ) 
        {
            // Rewind and write 'pausing' message
            m_pLogger->Rewind( m_CurrentLog );
            m_pLogger->LogWrite( m_CurrentLog, LOGF_WARNING | LOGF_ITALIC, false, _T("Paused..."));
    
        } // End if Logger Available

        // Sleep the thread until compiler is resumed
        while (m_Status == CS_PAUSED) Sleep(500);

    } // End if Paused

    // Should we cancel ?
    if ( m_Status == CS_CANCELLED ) return false;

    // Continue running.
    return true;
}

//-----------------------------------------------------------------------------
// Name : CompileScene ()
// Desc : Compiles the scene loaded from the specified filename
//-----------------------------------------------------------------------------
bool CCompiler::CompileScene( )
{
	m_Level.ClearObjects();

    // Validate Data
    if (!m_strFileName) return false;

    // Retrieve the filename portion only of the string
   
	LPTSTR FileName = m_strFileName; //NULL;
    FileName = _tcsrchr( m_strFileName, _T('\\') );
    if (!FileName) FileName = _tcsrchr( m_strFileName, _T('/') );
    if (!FileName) FileName = m_strFileName;
    if (FileName[0] == _T('\\') || FileName[0] == _T('/')) FileName++;
	

    try
    {
        // We Are starting the process
        m_Status = CS_INPROGRESS;

        // Load the specified file
		m_Level.Load( m_strFileName );

        // Obtain ownership of the objects loaded
        m_vpMeshList     = m_Level.m_vpMeshList;
        //m_vpMaterialList = MAPFile.m_vpMaterialList;
        //m_vpTextureList  = MAPFile.m_vpTextureList;
        //m_vpEntityList   = MAPFile.m_vpEntityList;
        //m_vpShaderList   = MAPFile.m_vpShaderList;

        // Clear out the MAP's object vectors (don't destroy)
		m_Level.m_vpMeshList.clear();
        //MAPFile.m_vpMaterialList.clear();
        //MAPFile.m_vpTextureList.clear();
        //MAPFile.m_vpEntityList.clear();
        //MAPFile.m_vpShaderList.clear();

        // Write load success
        if ( m_pLogger ) 
        {
            m_pLogger->LogWrite( LOG_GENERAL, 0, true, _T("Successfully loaded geometry from file '%s'"), FileName );
        
        } // End if Logger


    } // End try Block

    catch (HRESULT & e)
    {
        // Write Load Failure
        if ( m_pLogger ) 
        {
            m_pLogger->LogWrite( LOG_GENERAL, LOGF_ERROR, true, _T("Failed to load geometry from file '%s' with error code '0x%x'"), FileName, e );

        } // End if Logger

		m_Level.ClearObjects();
        Release();
        return false;

    } // End Catch Block
    catch (...)
    {
        // Write Load Failure
        if ( m_pLogger ) 
        {
            m_pLogger->LogWrite( LOG_GENERAL, LOGF_ERROR, true, _T("Failed to load geometry from file '%s'"), FileName );
        
        } // End if Logger

		m_Level.ClearObjects();
        Release();
        return false;
    
    } // End Catch Block

    // Write Log Info
    if ( m_pLogger ) 
    {
        m_pLogger->LogWrite( LOG_GENERAL, 0, true, _T("Beginning compilation run \t\t\t- ") );

    } // End if Logger

    // Start compiling by removing all hidden surfaces
	// note here: we have world mesh and a brushes world mesh. don't perform hsr (don't want only one mesh)
    //m_CurrentLog = LOG_HSR;
    //if ( m_OptionsHSR.Enabled && m_Status != CS_CANCELLED) PerformHSR();
    
    // Build the BSP Tree if requested
    m_CurrentLog = LOG_BSP;
    if ( m_OptionsBSP.Enabled && m_Status != CS_CANCELLED) PerformBSP();
    
    // Build the portals if requested
    m_CurrentLog = LOG_PRT;
    if ( m_OptionsPRT.Enabled && m_Status != CS_CANCELLED) PerformPRT();

    // Build the PVS if requested
    m_CurrentLog = LOG_PVS;
    if ( m_OptionsPVS.Enabled && m_Status != CS_CANCELLED) PerformPVS();

    // Repair any T-Juncs if requested
    m_CurrentLog = LOG_TJR;
    if ( m_OptionsTJR.Enabled && m_Status != CS_CANCELLED) PerformTJR();
    
	m_CurrentLog = LOG_LMP;
	if (m_OptionLightmapping.Enabled && m_Status != CS_CANCELLED) PerformLMP();

    // Clean up if required
    if ( m_Status == CS_CANCELLED ) Release();

    // Processing Run Completed
    m_Status = CS_IDLE;

    // Write end of compilation message (We use warning just to make it blue ;)
    if ( m_pLogger ) m_pLogger->LogWrite( LOG_GENERAL, LOGF_WARNING | LOGF_ITALIC, false, _T("Success") );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : SaveScene
// Desc : 
//-----------------------------------------------------------------------------
bool CCompiler::SaveScene( LPCTSTR FileName )
{
    ULONG       i;
    CMesh      *pTreeMesh = NULL;

    try
    {
        // Fill up the internal structures

        //MAPFile.m_vpTextureList 
        //MAPFile.m_vpMaterialList
        //MAPFile.m_vpEntityList
        //MAPFile.m_vpShaderList   ARE ALL ALREADY OK

    
        // If there is a tree, build a mesh from it
        if ( m_pBSPTree )
        {
            // Allocate a new tree mesh
            pTreeMesh = new CMesh;
            if (!pTreeMesh) throw std::bad_alloc();

            // Build the mesh from the bsp tree
            if (!pTreeMesh->BuildFromBSPTree( m_pBSPTree )) throw BCERR_OUTOFMEMORY;

			pTreeMesh->Name = _strdup("world");

            // Add the mesh to the file export list
			m_Level.m_vpMeshList.push_back( pTreeMesh );

        }
        else
        {
            // Add any conventional meshes
            for ( i = 0; i < m_vpMeshList.size(); i++ )
            {
                CMesh * pMesh = m_vpMeshList[ i ];
                if ( !pMesh ) continue;

                // Add this if it's not a detail mesh
                if ( !(pMesh->Flags & MESH_DETAIL) ) m_Level.m_vpMeshList.push_back( pMesh );
        
            } // Next Mesh
    
        } // End if No Tree

        // Loop through and add any detail meshes
        for ( i = 0; i < m_vpMeshList.size(); i++ )
        {
            CMesh * pMesh = m_vpMeshList[ i ];
            if ( !pMesh ) continue;

            // Add this if it's a detail mesh
            if ( pMesh->Flags & MESH_DETAIL ) m_Level.m_vpMeshList.push_back( pMesh );
        
        } // Next Mesh

        // Export file
		m_Level.Save( FileName, m_pBSPTree );

        // Clean up
        if ( pTreeMesh ) delete pTreeMesh;
		m_Level.m_vpMeshList.clear();
		//MAPFile.m_vpTextureList.clear();
        //MAPFile.m_vpEntityList.clear();
        //MAPFile.m_vpMaterialList.clear();
        //MAPFile.m_vpShaderList.clear();

    } // End Try Block

    catch (...)
    {
        if ( pTreeMesh ) delete pTreeMesh;
		m_Level.m_vpMeshList.clear();
        //MAPFile.m_vpTextureList.clear();
        //MAPFile.m_vpEntityList.clear();
        //MAPFile.m_vpMaterialList.clear();
        //MAPFile.m_vpShaderList.clear();
        return false;
    
    } // End Catch Block

    // Success!!
    return true;
}

//-----------------------------------------------------------------------------
// Name : PerformHSR () (Private)
// Desc : Perform the hidden surface removal tasks.
//-----------------------------------------------------------------------------
bool CCompiler::PerformHSR()
{
    // One time compile process
    CProcessHSR ProcessHSR;
    ULONG       i, MeshCount = 0;

    // Set our processor options
    ProcessHSR.SetOptions( m_OptionsHSR );
    ProcessHSR.SetLogger( m_pLogger );
    ProcessHSR.SetParent( this );

    // Write Log Information
    if ( m_pLogger )
    {
        m_pLogger->Clear( LOG_HSR );
        m_pLogger->LogWrite( LOG_HSR, LOGF_WARNING | LOGF_BOLD  , false, _T("\r\nHSR Processor v1.0.0\r\n"));
        m_pLogger->LogWrite( LOG_HSR, 0, true, _T("Beginning hidden-surface removal process."));

    } // End if Logger Available

    // Count the number of scene meshes
    for ( i = 0; i < m_vpMeshList.size(); i++ )
    {
        CMesh * pMesh = m_vpMeshList[i];
        if ( !pMesh ) continue;

        // Tally Mesh ?
        if ( !(pMesh->Flags & MESH_DETAIL) ) MeshCount++;

    } // Next Mesh 

    // How many meshes ?
    if ( MeshCount <= 1 )
    {
        m_pLogger->LogWrite( LOG_HSR, LOGF_WARNING, true, _T("Zero or Single mesh scene found. Skipping HSR process."));

    } // End if single mesh
    else
    {

        // Add all of our meshes to the HSR Processor
        for ( i = 0; i < m_vpMeshList.size(); i++ )
        {
            CMesh *pMesh = m_vpMeshList[i];
            if (!pMesh) continue;
            if ( pMesh->Flags & MESH_DETAIL ) continue;
            ProcessHSR.AddMesh( pMesh );

        } // Next Mesh

        // Begin the HSR Process
        ProcessHSR.Process();

        // Make way for our result if there is one
        if ( ProcessHSR.GetResultMesh() != NULL )
        {
            // Destroy any loaded meshes
            for ( i = 0; i < m_vpMeshList.size(); i++ )
            {
                CMesh * pMesh = m_vpMeshList[i];
                if ( !pMesh ) continue;
                if ( pMesh->Flags & MESH_DETAIL ) continue;
                delete pMesh;
                m_vpMeshList[i] = NULL;

            } // Next Mesh
    
            // Store our single result mesh in the first empty slot
            long MeshIndex = -1;
            for ( i = 0; i < m_vpMeshList.size(); i++ )
            {
                if ( !m_vpMeshList[i] ) { MeshIndex = i; break; }
            
            } // Next Mesh Slot

            if ( MeshIndex == -1 ) 
            {
                m_vpMeshList.push_back( ProcessHSR.GetResultMesh() );
            
            } // End if No Slot
            else
            {
                m_vpMeshList[ MeshIndex ] = ProcessHSR.GetResultMesh();
            
            } // End if Free Slot

        } // End if

    } // End if multiple meshes

    // Write Log Information
    if ( m_pLogger )
    {
        if ( m_Status != CS_CANCELLED )
            m_pLogger->LogWrite( LOG_HSR, 0, true, _T("Hidden-surface removal completed successfully."));
        else
            m_pLogger->LogWrite( LOG_HSR, 0, true, _T("Hidden-surface removal cancelled."));
    } // End if Logger Available

    // Success!!
    return true;
}

//-----------------------------------------------------------------------------
// Name : PerformBSP () (Private)
// Desc : Perform the BSP compilation tasks.
//-----------------------------------------------------------------------------
bool CCompiler::PerformBSP()
{
    ULONG       i;

    // Destroy any old BSP Tree
    if ( m_pBSPTree ) delete m_pBSPTree;

    // Allocate a new BSP Tree
    m_pBSPTree = new CBSPTree;

    // Set our compiler options
    m_pBSPTree->SetOptions( m_OptionsBSP );
    m_pBSPTree->SetLogger( m_pLogger );
    m_pBSPTree->SetParent( this );

    // Write Log Information
    if ( m_pLogger )
    {
        m_pLogger->Clear( LOG_BSP );
        m_pLogger->LogWrite( LOG_BSP, LOGF_WARNING | LOGF_BOLD  , false, _T("\r\nBSP Compiler v1.0.0\r\n"));
        m_pLogger->LogWrite( LOG_BSP, 0, true, _T("Beginning BSP compilation process."));

    } // End if Logger Available

    // Add all remaining meshes polys
    for ( i = 0; i < m_vpMeshList.size(); i++ )
    {
        CMesh * pMesh = m_vpMeshList[i];
        if ( !pMesh ) continue;
        //if ( pMesh->Flags & MESH_DETAIL ) continue;
        m_pBSPTree->AddFaces( pMesh->Faces, pMesh->FaceCount );

    } // Next Mesh

    // Add bounding polygons if requested
    if ( m_OptionsBSP.AddBoundingPolys )  m_pBSPTree->AddBoundingPolys( true );

    // Compile the BSP Tree
    m_pBSPTree->CompileTree();

    // Destroy any loaded scene meshes
    for ( i = 0; i < m_vpMeshList.size(); i++ )
    {
        CMesh * pMesh = m_vpMeshList[i];
        if ( !pMesh ) continue;
        //if ( pMesh->Flags & MESH_DETAIL ) continue;
        delete pMesh;
        m_vpMeshList[i] = NULL;

    } // Next Mesh

    // Prevent future logging
    m_pBSPTree->SetLogger( NULL );

    // Write Log Information
    if ( m_pLogger )
    {
        if ( m_Status != CS_CANCELLED )
            m_pLogger->LogWrite( LOG_BSP, 0, true, _T("BSP compilation completed successfully."));
        else
            m_pLogger->LogWrite( LOG_BSP, 0, true, _T("BSP compilation cancelled."));
    } // End if Logger Available
        
    // Success!!
    return true;
}

//-----------------------------------------------------------------------------
// Name : PerformPRT () (Private)
// Desc : Perform the portal compilation tasks.
//-----------------------------------------------------------------------------
bool CCompiler::PerformPRT()
{
    // One time compile process
    CProcessPRT ProcessPRT;

    // Set our process options
    ProcessPRT.SetOptions( m_OptionsPRT );
    ProcessPRT.SetLogger( m_pLogger );
    ProcessPRT.SetParent( this );

    // Write Log Information
    if ( m_pLogger )
    {
        m_pLogger->Clear( LOG_PRT );
        m_pLogger->LogWrite( LOG_PRT, LOGF_WARNING | LOGF_BOLD  , false, _T("\r\nPortal Processor v1.0.0\r\n"));
        m_pLogger->LogWrite( LOG_PRT, 0, true, _T("Beginning portal compilation process."));

    } // End if Logger Available

    // Compile the Portal set
    ProcessPRT.Process( m_pBSPTree );

    // Write Log Information
    if ( m_pLogger )
    {
        if ( m_Status != CS_CANCELLED )
            m_pLogger->LogWrite( LOG_PRT, 0, true, _T("Portal compilation completed successfully."));
        else
            m_pLogger->LogWrite( LOG_PRT, 0, true, _T("Portal compilation cancelled."));
    } // End if Logger Available

    // Success!!
    return true;
}

//-----------------------------------------------------------------------------
// Name : PerformPVS () (Private)
// Desc : Perform the potential visibility set compilation tasks.
//-----------------------------------------------------------------------------
bool CCompiler::PerformPVS()
{
    // One time compile process
    CProcessPVS ProcessPVS;

    // Set our processor options
    ProcessPVS.SetOptions( m_OptionsPVS );
    ProcessPVS.SetLogger( m_pLogger );
    ProcessPVS.SetParent( this );

    // Write Log Information
    if ( m_pLogger )
    {
        m_pLogger->Clear( LOG_PVS );
        m_pLogger->LogWrite( LOG_PVS, LOGF_WARNING | LOGF_BOLD  , false, _T("\r\nPVS Processor v1.0.0\r\n"));
        m_pLogger->LogWrite( LOG_PVS, 0, true, _T("Beginning visibility determination process."));

    } // End if Logger Available

    // Begin the PVS Process
    ProcessPVS.Process( m_pBSPTree );

    // Write Log Information
    if ( m_pLogger )
    {
        if ( m_Status != CS_CANCELLED )
            m_pLogger->LogWrite( LOG_PVS, 0, true, _T("Visibility determination completed successfully."));
        else
            m_pLogger->LogWrite( LOG_PVS, 0, true, _T("Visibility determination cancelled."));
    } // End if Logger Available

    // Success!!
    return true;

}

//-----------------------------------------------------------------------------
// Name : PerformTJR () (Private)
// Desc : Perform the T-Junction repair tasks.
//-----------------------------------------------------------------------------
bool CCompiler::PerformTJR()
{
    ULONG i, k;

    // One time compile process
    CProcessTJR ProcessTJR;

    // Set our process options
    ProcessTJR.SetOptions( m_OptionsTJR );
    ProcessTJR.SetLogger( m_pLogger );
    ProcessTJR.SetParent( this );

    // The TJunction processor simply accepts a bunch of separate polys
    // We can use this to compile BSP Tree or Mesh polys as we see fit
    ULONG       PolyCount = 0, Counter = 0;
    CPolygon ** ppPolys   = NULL;

    // Write Log Information
    if ( m_pLogger )
    {
        m_pLogger->Clear( LOG_TJR );
        m_pLogger->LogWrite( LOG_TJR, LOGF_WARNING | LOGF_BOLD  , false, _T("\r\nT-Junction Repair Processor v1.0.0\r\n"));
        m_pLogger->LogWrite( LOG_TJR, 0, true, _T("Beginning T-Junction repair process."));

    } // End if Logger Available

    if ( !m_pBSPTree )
    {    
        // Count all polys in all active meshes
        for ( i = 0; i < m_vpMeshList.size(); i++ )
        {
            CMesh * pMesh = m_vpMeshList[i];
            if ( !pMesh ) continue;
            if ( pMesh->Flags & MESH_DETAIL ) continue;
            PolyCount += pMesh->FaceCount;
        
        } // Next Mesh

        // If no polys exist, bail (no error)
        if (!PolyCount) return true;

        // Allocate our polygon pointer array
        if (!(ppPolys = new CPolygon*[PolyCount])) return false;

        // Add all mesh face pointers here
        for ( i = 0, Counter = 0; i < m_vpMeshList.size(); i++ )
        {
            CMesh * pMesh = m_vpMeshList[i];
            if ( !pMesh ) continue;
            if ( pMesh->Flags & MESH_DETAIL ) continue;
            
            // Loop through faces
            for ( k = 0; k < pMesh->FaceCount; k++ ) ppPolys[ Counter++ ] = pMesh->Faces[ k ];
        
        } // Next Mesh
        
    } // End if Meshes
    else
    {
        // Retrieve poly count
        PolyCount = m_pBSPTree->GetFaceCount();

        // If no polys exist, bail (no error)
        if (!PolyCount) return true;

        // Allocate our polygon pointer array
        if (!(ppPolys = new CPolygon*[PolyCount])) return false;

        // Loop through faces and add
        for ( k = 0; k < PolyCount; k++ ) ppPolys[ k ] = m_pBSPTree->GetFace( k );

    } // End if BSP Tree

    // Repair any T-Junctions
    ProcessTJR.Process( ppPolys, PolyCount );

    // Release the poly pointer array
    if (ppPolys) delete []ppPolys;

    // Write Log Information
    if ( m_pLogger )
    {
        if ( m_Status != CS_CANCELLED )
            m_pLogger->LogWrite( LOG_TJR, 0, true, _T("T-Junction repair completed successfully."));
        else
            m_pLogger->LogWrite( LOG_TJR, 0, true, _T("T-Junction repair cancelled."));
    } // End if Logger Available

    // Success!!
    return true;
}

bool CCompiler::PerformLMP()
{
	// Write Log Information
	if (m_pLogger)
	{
		m_pLogger->Clear(LOG_LMP);
		m_pLogger->LogWrite(LOG_LMP, LOGF_WARNING | LOGF_BOLD, false, _T("\r\nLight/Cluster Maps Compiler v1.0.0\r\n"));
		m_pLogger->LogWrite(LOG_LMP, 0, true, _T("Beginning Light/Cluster Maps generation process."));
	}

	/*************************************/

	struct LightData {
		int levelLightIdx;
		CBSPLeaf *leaf;
	};

	std::vector<LightData> lightsDataVec;
	int numLevelLights = (int)m_Level.m_lightsVec.size();
	for (int i = 0; i != numLevelLights; ++i)
	{
		Light &l = m_Level.m_lightsVec[i];
		
		CVector3 position = CVector3(l.origin[0], l.origin[1], l.origin[2]);

		CBSPLeaf *lightLeaf = m_pBSPTree->GetLeaf(m_pBSPTree->FindLeaf(position));

		if (lightLeaf != nullptr) {
			l.isValid = true;
			LightData lightData = { i, lightLeaf };
			lightsDataVec.push_back(lightData);
		}
	}

	struct BoundingRect {
		LightMapper::vec3 center;
		LightMapper::vec3 origin;
		LightMapper::vec3 uDir, vDir;
		float width, height;
	};

	struct PolygonData {
		CFace *polygon;
		std::vector<int> indices;
		BoundingRect polyBoundingRect;
		CBounds3 polyBounds;
	};

	LightMapper::TexturePacker texPacker;
	std::vector<std::unique_ptr<PolygonData>> polygonDataVec;
	//LightMapper::BSP bsp;

	unsigned long numPolygons = m_pBSPTree->GetFaceCount();
	for (unsigned long i = 0; i != numPolygons; ++i) 
	{
		CFace *polygon = m_pBSPTree->GetFace(i);
		if (polygon->VertexCount < 3) continue;

		PolygonData *polygonData = new PolygonData;
		polygonData->polygon = polygon;

		// compute poly bounds
		CVector3 & Min = polygonData->polyBounds.Min;
		CVector3 & Max = polygonData->polyBounds.Max;
	
		// Reset bounding box
		Min = CVector3(FLT_MAX, FLT_MAX, FLT_MAX);
		Max = CVector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

		// compute indices
		int FirstVertexIdx = 0;
		int PreviousVertexIdx = 0;

		unsigned long numVertices = polygon->VertexCount;
		for (unsigned long j = 0; j != numVertices; ++j)
		{		
			const CVertex &vtx = polygon->Vertices[j];
			if (vtx.x < Min.x) Min.x = vtx.x;
			if (vtx.y < Min.y) Min.y = vtx.y;
			if (vtx.z < Min.z) Min.z = vtx.z;
			if (vtx.x > Max.x) Max.x = vtx.x;
			if (vtx.y > Max.y) Max.y = vtx.y;
			if (vtx.z > Max.z) Max.z = vtx.z;

			if (j >= 2)
			{
				polygonData->indices.push_back(FirstVertexIdx);
				polygonData->indices.push_back(PreviousVertexIdx);
				polygonData->indices.push_back(j);

				LightMapper::vec3 v0 = LightMapper::vec3(polygon->Vertices[FirstVertexIdx].x, polygon->Vertices[FirstVertexIdx].y, polygon->Vertices[FirstVertexIdx].z);
				LightMapper::vec3 v1 = LightMapper::vec3(polygon->Vertices[PreviousVertexIdx].x, polygon->Vertices[PreviousVertexIdx].y, polygon->Vertices[PreviousVertexIdx].z);
				LightMapper::vec3 v2 = LightMapper::vec3(polygon->Vertices[j].x, polygon->Vertices[j].y, polygon->Vertices[j].z);
				//bsp.addTriangle(v0, v1, v2);
			}

			PreviousVertexIdx = j;
		}

		// Increase bounds slightly to relieve this operation during intersection testing
		Min.x -= 0.1f; Min.y -= 0.1f; Min.z -= 0.1f;
		Max.x += 0.1f; Max.y += 0.1f; Max.z += 0.1f;

		// setup texture rectangle

		/*******************************************************************************************/

		CVector3 v0, v1, v2;
		CVector3 uv0, uv1, uv2;
		CVector3 edge1, edge2;
		float area;
		int index = 0;

		// select v0,v1,v2 vertices non collinear
		do {
			if (polygonData->indices.size() == index) {
				m_Status = CS_CANCELLED;
				LightMapper::ErrorMsg("tangent space setup failed.\n");
				return false;
				//return 
				//break;
			}

			int i0 = polygonData->indices[index];
			int i1 = polygonData->indices[index+1];
			int i2 = polygonData->indices[index+2];
			v0 = CVector3(polygon->Vertices[i0].x, polygon->Vertices[i0].y, polygon->Vertices[i0].z);
			v1 = CVector3(polygon->Vertices[i1].x, polygon->Vertices[i1].y, polygon->Vertices[i1].z);
			v2 = CVector3(polygon->Vertices[i2].x, polygon->Vertices[i2].y, polygon->Vertices[i2].z);
			uv0 = CVector3(polygon->Vertices[i0].tu, polygon->Vertices[i0].tv, 0.f);
			uv1 = CVector3(polygon->Vertices[i1].tu, polygon->Vertices[i1].tv, 0.f);
			uv2 = CVector3(polygon->Vertices[i2].tu, polygon->Vertices[i2].tv, 0.f);

			edge1 = v1 - v0;
			edge2 = v2 - v0;

			area = LightMapper::length(
				LightMapper::cross(LightMapper::vec3(edge1.x, edge1.y, edge1.z),
					LightMapper::vec3(edge2.x, edge2.y, edge2.z)));
			index += 3;
			
		} while (area <= 0.01f);

		
		CVector3 deltaUV0 = uv1 - uv0;
		CVector3 deltaUV1 = uv2 - uv0;

		float f = 1.0f / (deltaUV0.x * deltaUV1.y - deltaUV1.x * deltaUV0.y);

		CVector3 tangent, bitangent;

		tangent.x = f * (deltaUV1.y * edge1.x - deltaUV0.y * edge2.x);
		tangent.y = f * (deltaUV1.y * edge1.y - deltaUV0.y * edge2.y);
		tangent.z = f * (deltaUV1.y * edge1.z - deltaUV0.y * edge2.z);
		tangent = tangent.Normalize();

		bitangent.x = f * (-deltaUV1.x * edge1.x + deltaUV0.x * edge2.x);
		bitangent.y = f * (-deltaUV1.x * edge1.y + deltaUV0.x * edge2.y);
		bitangent.z = f * (-deltaUV1.x * edge1.z + deltaUV0.x * edge2.z);
		bitangent = bitangent.Normalize();

		CVector3 u = tangent;
		CVector3 v = bitangent;
		CVector3 d;

		float uMin = 0.f;
		float uMax = 0.f;
		float vMin = 0.f;
		float vMax = 0.f;
		CVector3 originVertex = v0;
		CVector3 origin = originVertex;
		//CVector3 center(0.f, 0.f, 0.f);

		for (unsigned long j = 0; j != numVertices; ++j)
		{
			CVector3 &curVertex = polygon->Vertices[j];
			//center += curVertex;
			d = curVertex - origin;
			float uLength = d.Dot(u);
			float vLength = d.Dot(v);
			uMin = min(uLength, uMin);
			uMax = max(uLength, uMax);
			vMin = min(vLength, vMin);
			vMax = max(vLength, vMax);
		}

		//center *= 1.0f/numVertices;

		//uMin -= 0.5f;// 0.5f;
		//vMin -= 0.5f;// 0.5f;
		//uMax += 0.5f;// 0.5f;
		//vMax += 0.5f;// 0.5f;

		d = u * uMin;
		origin += d;
		d = v * vMin;
		origin += d;

		float width = uMax - uMin;
		float height = vMax - vMin;

		//polygonData->polyBoundingRect.center = LightMapper::vec3(center.x, center.y, center.z);
		polygonData->polyBoundingRect.origin = LightMapper::vec3(origin.x, origin.y, origin.z);
		polygonData->polyBoundingRect.uDir = LightMapper::vec3(tangent.x, tangent.y, tangent.z);
		polygonData->polyBoundingRect.vDir = LightMapper::vec3(bitangent.x, bitangent.y, bitangent.z);
		polygonData->polyBoundingRect.width = width;
		polygonData->polyBoundingRect.height = height;

		float w = width / m_OptionLightmapping.lightmap_res_factor;//28;
		float h = height / m_OptionLightmapping.lightmap_res_factor;//28;

		if (w < 8) w = 8;
		if (h < 8) h = 8;

		int tw = (int)w;
		int th = (int)h;

		tw = (tw + 4) & ~7;
		th = (th + 4) & ~7;

		texPacker.addRectangle(tw, th);

		// build extended poly bound
		/* not used 
		uMin -= 0.1f * width;// 0.5f;
		vMin -= 0.1f * height;// 0.5f;
		uMax += 0.1f * width;// 0.5f;
		vMax += 0.1f * height;// 0.5f;
		origin = originVertex;
		d = u * uMin;
		origin += d;
		d = v * vMin;
		origin += d;
		width = uMax - uMin;
		height = vMax - vMin;
		polygonData->polyBoundingRectExt.origin = LightMapper::vec3(origin.x, origin.y, origin.z);
		polygonData->polyBoundingRectExt.uDir = LightMapper::vec3(tangent.x, tangent.y, tangent.z);
		polygonData->polyBoundingRectExt.vDir = LightMapper::vec3(bitangent.x, bitangent.y, bitangent.z);
		polygonData->polyBoundingRectExt.width = width;
		polygonData->polyBoundingRectExt.height = height;
		*/


		// finally save polygonData
		polygonDataVec.push_back(std::unique_ptr<PolygonData>(polygonData));
		
	} // end polygon setup loop
	 

	//bsp.build();
	
	unsigned int lm_width = m_OptionLightmapping.lm_width;//512;
	unsigned int lm_height = m_OptionLightmapping.lm_height;//512;

	if (!texPacker.assignCoords(&lm_width, &lm_height, LightMapper::widthComp))
	{
		LightMapper::ErrorMsg("Lightmap too small");
		return false;
	}


	for (unsigned long i = 0; i != numPolygons; ++i)
	{
		LightMapper::TextureRectangle* rect = texPacker.getRectangle(i);
		//polygonDataVec[i]
		//const LightMapper::vec3 &center = polygonDataVec[i]->polyBoundingRect.center;
		const LightMapper::vec3 &origin = polygonDataVec[i]->polyBoundingRect.origin;
		const LightMapper::vec3 &u = polygonDataVec[i]->polyBoundingRect.uDir;
		const LightMapper::vec3 &v = polygonDataVec[i]->polyBoundingRect.vDir;

		unsigned long numVertices = polygonDataVec[i]->polygon->VertexCount;
		for (unsigned long j = 0; j != numVertices; ++j)
		{
			CVertex &curVertex = polygonDataVec[i]->polygon->Vertices[j];
			LightMapper::vec3 src(curVertex.x, curVertex.y, curVertex.z);
			//LightMapper::vec3 t = src - center;
			//LightMapper::vec3 vert = (t * 0.90f) + center;
			LightMapper::vec3 d = src - origin;

			float uLength = dot(d,u) / polygonDataVec[i]->polyBoundingRect.width;
			float vLength = dot(d,v) / polygonDataVec[i]->polyBoundingRect.height;

			float uOffset = 0.0f;
			float vOffset = 0.0f;

			if (uLength < 0.001f) uOffset = 0.5f;
			else if (uLength > 1.f - 0.001f) uOffset = -0.5f;
	
			if (vLength < 0.001f) vOffset = 0.5f;
			else if (vLength > 1.f - 0.001f) vOffset = -0.5f;

			float xl = float(rect->x + uLength * rect->width + uOffset) / lm_width;
			float yl = float(rect->y + vLength * rect->height + vOffset) / lm_height;
		
			curVertex.lu = xl;
			curVertex.lv = yl;
		}

	}

	//const unsigned int SAMPLE_COUNT = 250;
	//LightMapper::vec3 p_samples[SAMPLE_COUNT];
	//LightMapper::vec2 t_samples[SAMPLE_COUNT];
	unsigned int numSamples = m_OptionLightmapping.sampleCount;
	std::vector<LightMapper::vec3> p_samples(numSamples);
	std::vector<LightMapper::vec2> t_samples(numSamples);

	for (unsigned int s = 0; s < numSamples; s++)
	{
		LightMapper::vec3 p;
		do
		{
			p = LightMapper::vec3(float(rand()), float(rand()), float(rand())) * (2.0f / RAND_MAX) - 1.0f;
		} while (LightMapper::dot(p,p) > 1.0f);
		p_samples[s] = p * m_OptionLightmapping.sampleDistanceFactor;
		
		t_samples[s] = LightMapper::vec2(float(rand()), float(rand())) * (1.0f / RAND_MAX) - 0.5f; //LightMapper::vec2(0, 0);//
	}

	/* Cluster Map setup up */
	const unsigned int cluster_divisor = m_OptionLightmapping.clustermap_lightmap_factor;
	const unsigned int cm_width = lm_width / cluster_divisor;
	const unsigned int cm_height = lm_height / cluster_divisor;
	//uint16_t* clusters = new uint16_t[cm_width * cm_height];
	//memset(clusters, 0, cm_width * cm_height * sizeof(uint16_t));
	/* End Cluster Map setup up */


	if (m_pLogger)
	{
		m_pLogger->LogWrite(LOG_LMP, 0, true, _T("Light/Cluster Maps generation progress \t\t\t- "));
		m_pLogger->SetRewindMarker(LOG_LMP);
		m_pLogger->LogWrite(LOG_LMP, 0, false, _T("0%%"));
		m_pLogger->SetProgressRange(lightsDataVec.size());
		m_pLogger->SetProgressValue(0);
	}

	// Lightmapping.
	int numLightsToProcess = (int)lightsDataVec.size();
	for (int light_index = 0; light_index != numLightsToProcess; ++light_index)
	{
		LightData &lightData = lightsDataVec[light_index];
		CVector3 lightPosition(m_Level.m_lightsVec[lightData.levelLightIdx].origin[0],
								m_Level.m_lightsVec[lightData.levelLightIdx].origin[1],
								m_Level.m_lightsVec[lightData.levelLightIdx].origin[2]);
		LightMapper::vec3 lightPos(lightPosition.x, lightPosition.y, lightPosition.z);
		float lightRadius = m_Level.m_lightsVec[lightData.levelLightIdx].radius;
		float lightRadiusSqr = lightRadius * lightRadius;
		uint8_t* lMap = new uint8_t[lm_width * lm_height];
		memset(lMap, 0, lm_width * lm_height);

		std::vector<unsigned long> &pvsLightLeafIndices = m_pBSPTree->FindPVSLeafIndices(lightData.leaf);
		// sort leaves by light distance ? to avoid excluding lights (when #leaves is >16) from near leaves then	
		CBSPTree *bspTree = m_pBSPTree;
		///*
		std::sort(std::begin(pvsLightLeafIndices), std::end(pvsLightLeafIndices),
			[&lightPosition, bspTree](unsigned long aLeafIdx, unsigned long bLeafIdx) {
			CBSPLeaf *a = bspTree->GetLeaf(aLeafIdx);
			CBSPLeaf *b = bspTree->GetLeaf(bLeafIdx);
			double aDist = SquaredDistPointAABB(lightPosition, a->Bounds);
			double bDist = SquaredDistPointAABB(lightPosition, b->Bounds);
			return aDist < bDist;
		});
		//*/
		int numLeaves = (int)pvsLightLeafIndices.size();
		for (int iLeaf = 0; iLeaf != numLeaves; ++iLeaf) 
		{
			
			CBSPLeaf *pLeaf = m_pBSPTree->GetLeaf(pvsLightLeafIndices[iLeaf]);
			
			//double squaredDistance = SquaredDistPointAABB(lightPosition, pLeaf->Bounds);
			//if (squaredDistance > lightRadiusSqr)
			//	continue;

			if (pLeaf->numLights >= 16)
				continue; // ignore light for this leaf

			if (pLeaf->lightClusters == nullptr) {
				pLeaf->lightClusters = new uint16_t[cm_width * cm_height];
				memset(pLeaf->lightClusters, 0, cm_width * cm_height * sizeof(uint16_t));
			}

			//pLeaf->lightsIndicesArr[pLeaf->numLights] = lightData.levelLightIdx;
			m_Level.m_lightsVec[lightData.levelLightIdx].leaves.push_back({ (uint16_t)pvsLightLeafIndices[iLeaf], (uint8_t)pLeaf->numLights });

			unsigned long numLeafPolygons = pLeaf->FaceIndices.size();
			for (unsigned long j = 0; j != numLeafPolygons; ++j)
			{
				long leafPolyIdx = pLeaf->FaceIndices[j];

				//double squaredDistance = SquaredDistPointAABB(lightPosition, polygonDataVec[leafPolyIdx]->polyBounds);
				//if (squaredDistance > lightRadiusSqr) continue;

				LightMapper::TextureRectangle& rect = *texPacker.getRectangle(leafPolyIdx);
				LightMapper::vec3 pos = polygonDataVec[leafPolyIdx]->polyBoundingRect.origin;
				LightMapper::vec3 dirS = polygonDataVec[leafPolyIdx]->polyBoundingRect.uDir * polygonDataVec[leafPolyIdx]->polyBoundingRect.width;
				LightMapper::vec3 dirT = polygonDataVec[leafPolyIdx]->polyBoundingRect.vDir * polygonDataVec[leafPolyIdx]->polyBoundingRect.height;

				LightMapper::vec3 normal(polygonDataVec[leafPolyIdx]->polygon->Normal.x, polygonDataVec[leafPolyIdx]->polygon->Normal.y, polygonDataVec[leafPolyIdx]->polygon->Normal.z);
				float d = -dot(pos, normal);
				if (dot(lightPos, normal) + d < 0.0f)
					continue;

				// Compute light contribution on this rectangle
				for (unsigned int t = 0; t < rect.height; t++)
				{
					for (unsigned int s = 0; s < rect.width; s++)
					{						
						float light = 0.0f;
						for (unsigned int k = 0; k < numSamples; k++)
						{
							LightMapper::vec3 sample_x = saturate((float(s)) / (rect.width-1) + t_samples[k].x / (rect.width - 1)) * dirS; //saturate((float(s) / (rect.width - 1)) + t_samples[k].x / (rect.width - 1)) * dirS;//((float(s)) / rect.width) * dirS; //  + .5f
							LightMapper::vec3 sample_y = saturate((float(t)) / (rect.height-1) + t_samples[k].y / (rect.height - 1)) * dirT; //saturate((float(t) / (rect.height - 1)) + t_samples[k].y / (rect.height - 1)) * dirT;
							LightMapper::vec3 lumelSamplePos = pos + sample_x + sample_y;
							
							LightMapper::vec3 lightSample = lightPos + p_samples[k];

							//bsp.pushSphere(lumelSamplePos, 0.3f);
							//bsp.pushSphere(lumelSamplePos, 0.2f);
							//bsp.pushSphere(lumelSamplePos, 0.1f);
							
							CVector3 origin = { lightSample.x, lightSample.y, lightSample.z };
							CVector3 dir = CVector3(lumelSamplePos.x, lumelSamplePos.y, lumelSamplePos.z) - origin;

							if (!m_pBSPTree->RayIntersect(origin, dir)) 
							{
								LightMapper::vec3 lightLumelVec = lightSample - lumelSamplePos;

								float distSqr = LightMapper::dot(lightLumelVec, lightLumelVec);
								if (distSqr > lightRadiusSqr) continue;

								light += 1.0f / numSamples;
							}
						}

						uint8_t light_val = (uint8_t)(255.0f * sqrtf(light) + 0.5f); // sqrtf() for poor man's gamma
						lMap[(rect.y + t) * lm_width + (rect.x + s)] = light_val;
					}
				}


				// Assign light clusters for this rectangle
				const unsigned int s0 = rect.x / cluster_divisor;
				const unsigned int t0 = rect.y / cluster_divisor;
				const unsigned int s1 = s0 + rect.width / cluster_divisor;
				const unsigned int t1 = t0 + rect.height / cluster_divisor;
				for (unsigned int t = t0; t < t1; t++)
				{
					for (unsigned int s = s0; s < s1; s++)
					{
						// Sample the light within the rect, plus one extra pixel border to cover filtering, but clamp to stay within the rect
						unsigned int ls0 = max(int(s * cluster_divisor - 1), (int)rect.x);
						unsigned int ls1 = min((s + 1) * cluster_divisor + 1, rect.x + rect.width);
						unsigned int lt0 = max(int(t * cluster_divisor - 1), (int)rect.y);
						unsigned int lt1 = min((t + 1) * cluster_divisor + 1, rect.y + rect.height);

						for (unsigned int lt = lt0; lt < lt1; lt++)
						{
							for (unsigned int ls = ls0; ls < ls1; ls++)
							{
								const uint8_t light_val = lMap[lt * lm_width + ls];
								if (light_val > 0)
								{
									pLeaf->lightClusters[t * cm_width + s] |= (1 << pLeaf->numLights);//(1 << light_index);
									goto double_break;
								}
							}
						}
					double_break:
						;
					}
				}

			} // end leaf polygon loop
				
			++pLeaf->numLights;
						
		} // end light pvs leaves loop

		LightMapper::Image image;
		image.loadFromMemory(lMap, LightMapper::FORMAT_I8, lm_width, lm_height, 1, 1, true);
		char fileName[256];
		sprintf(fileName, "%s//LightMap%d.dds", LIGHT_CLUSTER_MAPS_FOLDER, light_index);
		image.saveImage(fileName);

		if (m_pLogger) m_pLogger->UpdateProgress();

	} // end lights loop

	unsigned long numLeaves = m_pBSPTree->GetLeafCount();
	for (unsigned long i = 0; i != numLeaves; ++i)
	{
		CBSPLeaf *leaf = m_pBSPTree->GetLeaf(i);

		if (leaf->lightClusters == nullptr) {
			leaf->lightClusters = new uint16_t[cm_width * cm_height];
			memset(leaf->lightClusters, 0, cm_width * cm_height * sizeof(uint16_t));
		}

		LightMapper::Image image;
		image.loadFromMemory(leaf->lightClusters, LightMapper::FORMAT_R16UI, cm_width, cm_height, 1, 1, true);
		char fileName[256];
		sprintf(fileName, "%s//ClustersLeaf%d.dds", LIGHT_CLUSTER_MAPS_FOLDER, i);
		image.saveImage(fileName);
		
		//leaf->lightClusters = nullptr;
	}

	/*************************************/

	if (m_pLogger) m_pLogger->ProgressSuccess(LOG_PVS);

	// Write Log Information
	if (m_pLogger)
	{
		if (m_Status != CS_CANCELLED) {
			m_pLogger->LogWrite(LOG_LMP, 0, true, _T("Light/Cluster Maps generation completed successfully."));
			m_Level.m_useLightmaps = true;
		}
		else 
		{
			m_pLogger->LogWrite(LOG_LMP, 0, true, _T("Light/Cluster Maps generation cancelled."));
			m_Level.m_useLightmaps = false;
		}
	} // End if Logger Available


	return true;
}

//-----------------------------------------------------------------------------
// Name : Release ()
// Desc : Releases any memory used by this class.
//-----------------------------------------------------------------------------
void CCompiler::Release()
{
    unsigned long i;

    // Destroy any loaded meshes
    for ( i = 0; i < m_vpMeshList.size(); i++ )
    {
        if ( m_vpMeshList[i] ) delete m_vpMeshList[i];

    } // Next Mesh

	/*
    // Destroy any loaded textures
    for ( i = 0; i < m_vpTextureList.size(); i++ )
    {
        if ( m_vpTextureList[i] ) delete m_vpTextureList[i];

    } // Next Texture

    // Destroy any loaded maerials
    for ( i = 0; i < m_vpMaterialList.size(); i++ )
    {
        if ( m_vpMaterialList[i] ) delete m_vpMaterialList[i];

    } // End If

    // Destroy any loaded shaders
    for ( i = 0; i < m_vpShaderList.size(); i++ )
    {
        if ( m_vpShaderList[i] ) delete m_vpShaderList[i];

    } // End If

    // Destroy any loaded entities
    for ( i = 0; i < m_vpEntityList.size(); i++ )
    {
        if ( m_vpEntityList[i] ) delete m_vpEntityList[i];

    } // End If
	*/

    // Clear the vectors
    m_vpMeshList.clear();
    //m_vpTextureList.clear();
    //m_vpMaterialList.clear();
    //m_vpEntityList.clear();
    //m_vpShaderList.clear();

    // Release strings
    if ( m_strFileName ) free( m_strFileName );
    m_strFileName = NULL;

    // Destroy any compiled BSP Tree
    if (m_pBSPTree) delete m_pBSPTree;
    m_pBSPTree = NULL;
}

//-----------------------------------------------------------------------------
// Name : SetFile ()
// Desc : Set the file that will be used for compilation
//-----------------------------------------------------------------------------
void CCompiler::SetFile( LPCTSTR FileName )
{
    // Release any old filename
    if ( m_strFileName ) free( m_strFileName );
    m_strFileName = NULL;

    // Duplicate the filename
    m_strFileName = _tcsdup( FileName );

}

//-----------------------------------------------------------------------------
// Name : SetOptions ()
// Desc : Set the options relevant to the various processes
//-----------------------------------------------------------------------------
void CCompiler::SetOptions( UINT Process, const LPVOID Options )
{
    switch (Process)
    {
        case PROCESS_HSR:
            m_OptionsHSR = *((HSROPTIONS*)Options);
            break;

        case PROCESS_BSP:
            m_OptionsBSP = *((BSPOPTIONS*)Options);
            break;

        case PROCESS_PRT:
            m_OptionsPRT = *((PRTOPTIONS*)Options);
            break;

        case PROCESS_PVS:
            m_OptionsPVS = *((PVSOPTIONS*)Options);
            break;

        case PROCESS_TJR:
            m_OptionsTJR = *((TJROPTIONS*)Options);
            break;

    } // End Switch
}

//-----------------------------------------------------------------------------
// Name : GetOptions ()
// Desc : Get the options relevant to the various processes
//-----------------------------------------------------------------------------
void CCompiler::GetOptions( UINT Process, LPVOID Options ) const
{
    switch (Process)
    {
        case PROCESS_HSR:
            *((HSROPTIONS*)Options) = m_OptionsHSR;
            break;

        case PROCESS_BSP:
            *((BSPOPTIONS*)Options) = m_OptionsBSP;
            break;

        case PROCESS_PRT:
            *((PRTOPTIONS*)Options) = m_OptionsPRT;
            break;

        case PROCESS_PVS:
            *((PVSOPTIONS*)Options) = m_OptionsPVS;
            break;

        case PROCESS_TJR:
            *((TJROPTIONS*)Options) = m_OptionsTJR;
            break;

    } // End Switch
}