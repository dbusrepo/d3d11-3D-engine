
//-----------------------------------------------------------------------------
// Main Module Includes
//-----------------------------------------------------------------------------
#include "Main.h"
#include "../Compiler Source/CCompiler.h"
#include "LogOutput.h"
#include <conio.h>

#define INPUT_FILE "Content\\Map\\test.map"
#define OUTPUT_FILE "Content\\Bsp\\test.bsp"

//-----------------------------------------------------------------------------
// Name : WinMain() (Application Entry Point)
// Desc : Entry point for program, App flow starts here.
//-----------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int iCmdShow )
{
    //OPENFILENAME  File;
    TCHAR         FileName[MAX_PATH];
    CLogOutput    LogOutput;
    CCompiler     Compiler;
    bool          bSuccess;
    ULONG         i;

    // Create a console window so that we can use it as our logging output
    AllocConsole();

    // Redirect all standard output to console (i.e. printf, cout etc).
    freopen("CONOUT$", "a", stdout);

    // Create the log output handler and attach it to the compiler
    LogOutput.Create( 20 );
    LogOutput.LogWrite( LOG_GENERAL, 0, false, _T("\nSolid Leaf BSP Tree Compiler v1.0.0\n"));
    LogOutput.LogWrite( LOG_GENERAL, 0, true,  _T("Compiler startup successful, awaiting user input."));
    Compiler.SetLogger( &LogOutput );

	/*
    // Fill out our default file dialog structure
    ZeroMemory( &File, sizeof(OPENFILENAME) );
    ZeroMemory( FileName, MAX_PATH * sizeof(TCHAR));
    File.lStructSize  = sizeof(OPENFILENAME);
    File.hwndOwner    = NULL;
    File.nFilterIndex = 1;
    File.lpstrTitle   = _T("Select Input IWF File for Compilation.");
    File.Flags        = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST;
    File.lpstrFile    = FileName;
    File.nMaxFile     = MAX_PATH - 1;
	File.lpstrFilter = _T("*.map");

    // Retrieve a filename.
    if ( !GetOpenFileName( &File ) ) return 0;
	*/

	strcpy(FileName, INPUT_FILE);
//"C:\\Users\\htcVive\\Google Drive\\D3DGraphicsProgramming\\D3D11Projects\\D3DEngine\\tools\\Map\\test.map");
			
		//"Content\\Map\\test.map");

    // Inform the compiler about the file to compile
    Compiler.SetFile( FileName );
	
    // Compile the scene
    bSuccess = Compiler.CompileScene();

    // Clear the console window and output all of the status text one last time
    LogOutput.ClearConsole();
    for ( i = 0; i < 20; ++i ) 
    {
        // Was there any text in this channel?
        if (LogOutput.GetChannelText( i ) != NULL)
        {
            // Print the channel information out (specifying false to the clear parameter)
            LogOutput.SwitchChannel( i, false );
            printf( "\n\n---------------------------------------------------------------------\n" );

        } // End if anything in channel

    } // Next Channel

    // Save the scene if it compiled succesfully
    if ( bSuccess )
    {
        // Alter structure for any changed options
        //File.nFilterIndex = 1;
        //File.lpstrTitle   = _T("Select Output BSP File.");
        //File.lpstrFilter  = _T("BSP Compiled (*.bsp)\0");
        //ZeroMemory( FileName, MAX_PATH * sizeof(TCHAR));

		strcpy(FileName, OUTPUT_FILE);
		Compiler.SaveScene(FileName);

		/*
        // Retrieve a filename.
        if ( GetSaveFileName( &File ) )
        {
            // Save the scene
            Compiler.SaveScene( FileName );

        } // End if Selected File
        else
        {
            // Output information
            printf( "User chose to cancel save operation." );

        } // End if no file
		*/

    } // End if success

    // Hold application until user presses key
    printf( "\n\nPress any key to exit..." );
    _getch();

    // Clean up console
    FreeConsole();

    // Return
    return 0;
}