
#ifndef _APPERROR_H_
#define _APPERROR_H_

//-----------------------------------------------------------------------------
// HRESULT typedef
//-----------------------------------------------------------------------------
typedef LONG HRESULT;

//-----------------------------------------------------------------------------
// Miscellaneous Error Defines
//-----------------------------------------------------------------------------
#define FACILITY_BC  0x216              // BSP Compiler Facility Code

//-----------------------------------------------------------------------------
// Miscellaneous Error Macros
//-----------------------------------------------------------------------------
#define BCHRESULT(sev,fac,code) ((HRESULT) (((unsigned long)(sev)<<31) | ((unsigned long)(fac)<<16) | ((unsigned long)(code))) )

#undef SUCCEEDED
#undef FAILED

#define SUCCEEDED(Status) ((HRESULT)(Status) >= 0)
#define FAILED(Status) ((HRESULT)(Status)<0)

//-----------------------------------------------------------------------------
// Error Code Definitions
//-----------------------------------------------------------------------------
#define BC_OK                                   ((HRESULT)0x00000000L)
#define BC_CANCELLED                            BCHRESULT(0, FACILITY_BC, 0001)

// Miscellaneous Usage Errors
#define BCERR_GENERIC                           BCHRESULT(1, FACILITY_BC, 1000)
#define BCERR_UNKNOWN                           BCHRESULT(1, FACILITY_BC, 1001)
#define BCERR_INVALIDPARAMS                     BCHRESULT(1, FACILITY_BC, 1002)

// Memory Related Errors
#define BCERR_OUTOFMEMORY                       BCHRESULT(1, FACILITY_BC, 2000)
#define BCERR_INVALIDREADPOINTER                BCHRESULT(1, FACILITY_BC, 2001)
#define BCERR_INVALIDWRITEPOINTER               BCHRESULT(1, FACILITY_BC, 2002)

// File and Stream IO Related Errors
#define BCERR_FILENOTFOUND                      BCHRESULT(1, FACILITY_BC, 3000)
#define BCERR_FILENOTOPEN                       BCHRESULT(1, FACILITY_BC, 3001)
#define BCERR_FILEALREADYOPEN                   BCHRESULT(1, FACILITY_BC, 3002)
#define BCERR_ACCESSDENIED                      BCHRESULT(1, FACILITY_BC, 3003)
#define BCERR_INVALIDSEEK                       BCHRESULT(1, FACILITY_BC, 3004)
#define BCERR_PASSEDEOF                         BCHRESULT(1, FACILITY_BC, 3005)
#define BCERR_LOADFAILURE                       BCHRESULT(1, FACILITY_BC, 3006)
#define BCERR_SAVEFAILURE                       BCHRESULT(1, FACILITY_BC, 3007)

// Directory Service Related Errors
#define BCERR_DIRECTORYNOTFOUND                 BCHRESULT(1, FACILITY_BC, 4001)

// Display Device Related Errors
#define BCERR_NORENDERDEVICES                   BCHRESULT(1, FACILITY_BC, 6000)
#define BCERR_RENDERDEVICENOTFOUND              BCHRESULT(1, FACILITY_BC, 6001)

// BSP Compiler related errors.
#define BCERR_BSP_INVALIDGEOMETRY               BCHRESULT(1, FACILITY_BC, 7000)
#define BCERR_BSP_INVALIDTREEDATA               BCHRESULT(1, FACILITY_BC, 7001)

#endif // _APPERROR_H_

