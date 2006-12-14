#if !defined(BATTLEENC_H)

#define BATTLEENC_H

#if _MSC_VER > 1000
	#pragma once
#endif

#undef _WIN32_IE
#define _WIN32_IE 0x0501 // _WIN32_IE_IE501 = _WIN32_IE_WIN2KSP4

#define APP_CLASS TEXT( "BATTLEENC" )

#define MB_CUTE 0x0UL
// -- This can foobars non-English Win2K --
//#define MB_CUTE MB_ERR_INVALID_CHARS //0x00000008

#ifdef _UNICODE
	#undef  _WIN32_WINNT
	#define _WIN32_WINNT 0x0500
	#define APP_NAME L"BES - Battle Encoder Shirase 1.2.2"
#else
	#define APP_NAME "BES - Battle Encoder Shirase 1.2.2"
#endif

#define APP_HOME_URL TEXT( "http://www.faireal.net/mion/" )

#ifdef _MBCS
	#undef _MBCS
#endif

#ifdef _UNICODE
	#ifndef UNICODE
		#define UNICODE
	#endif
	#define APP_COPYRIGHT L"Copyright \251 2004" L"\x2013" L"2006 mion"
#else
	#ifdef UNICODE
		#undef UNICODE
	#endif
	#define APP_COPYRIGHT "Copyright (c) 2004-2006 mion"
#endif

#include <windows.h>
#include <tchar.h>
#include <tlhelp32.h>
#include <shellapi.h>
#include <Windowsx.h> // DeleteFont
#include <commctrl.h> // TRACKBAR_CLASS
#include <stdio.h> // fputc
#include <stdlib.h> // abs
#include <shlwapi.h> // DLLVERSIONINFO
#include <math.h> // floor
#include <olectl.h> // Jpeg
#include <shlobj.h> // SHGetFolderPath

#include "resource.h"
#include "strings.utf8.h"
#include "sstp.sjis.h"

#define WATCHING_IDLE      ( (DWORD) -1 )
#define TARGET_PID_NOT_SET          ( 0UL )
#define TARGET_UNDEF ( TEXT( "<target not specified>" ) )

#define JUST_UPDATE_STATUS          ( 3U )


#define MAX_THREAD_CNT  128
#define MAX_PROCESS_CNT  256
#define MAX_WINDOWTEXT 1024
#define CP_SJIS 932U

#ifndef CP_UTF8
	#define CP_UTF8 65001U
#endif

#ifndef CREARTYPE_QUALITY
	#define CLEARTYPE_QUALITY 5
#endif


#define SURROGATE_LO( wc ) ( ( (WCHAR) wc ) >= 0xDC00 && ( (WCHAR) wc ) <= 0xDFFF )

#ifndef UNREFERENCED_PARAMETER
	#define UNREFERENCED_PARAMETER(P)((P))
#endif

#define LANG_JAPANESEo ((WORD) -2)
#define LANG_CHINESE_T ((WORD) 2807 )
#define LANG_CHINESE_S ((WORD) 2803 )

#define IS_ENGLISH  ( GetLangauge() == LANG_ENGLISH )
#define IS_FINNISH  ( GetLanguage() == LANG_FINNISH )
#define IS_JAPANESE ( GetLanguage() == LANG_JAPANESE || GetLanguage() == LANG_JAPANESEo )
#define IS_JAPANESEo ( GetLanguage() == LANG_JAPANESEo )
#define IS_CHINESE   ( GetLanguage() == LANG_CHINESE_T || GetLanguage() == LANG_CHINESE_S )
#define IS_CHINESE_T ( GetLanguage() == LANG_CHINESE_T )
#define IS_CHINESE_S ( GetLanguage() == LANG_CHINESE_S )
#define IS_SPANISH   ( GetLanguage() == LANG_SPANISH )
#define IS_FRENCH   ( GetLanguage() == LANG_FRENCH )
#define REPEATED_KEYDOWN( lParam ) ( ( lParam ) & 0x40000000 )

#define IFF_SYSTEM                 -2
#define IFF_FRIEND                 -1
#define IFF_UNKNOWN                 0
#define IFF_FOE                     1
#define IFF_ENEMY                   1
#define IFF_ABS_FOE                 2

#define XLIST_WATCH_THIS           -1
#define XLIST_RESTART_0             0
#define XLIST_RESTART_1             1
#define XLIST_RESTART_2             2
#define XLIST_CANCELED           1024
#define XLIST_NEW_TARGET         2048
#define XLIST_UNFREEZE           4096

#define STOP_FROM_TRAY     0x000A
#define NORMAL_TERMINATION 0x1000
#define NOT_WATCHING       0x1001
#define THREAD_NOT_OPENED  0xEEEE
#define TARGET_MISSING     0xF000

#define WM_USER_HACK       ( WM_APP +   1 )
#define WM_USER_STOP       ( WM_APP +   2 )
#define WM_USER_RESTART    ( WM_APP +   3 )
#define WM_USER_NOTIFYICON ( WM_APP +  10 )
#define WM_USER_REFRESH    ( WM_APP +  20 )

#define WM_USER_BUTTON     ( WM_APP +  30 )

typedef struct {
	WORD wIndex;
	TCHAR szTitle[ MAX_PROCESS_CNT ][ MAX_WINDOWTEXT ];
	DWORD dwThreadId[ MAX_PROCESS_CNT ];
	HWND hwnd[ MAX_PROCESS_CNT ];
} WININFO, *LPWININFO;

typedef struct {
	TCHAR szExe[ MAX_PATH * 2 ];
	TCHAR szPath[ MAX_PATH * 2 ];
	TCHAR szText[ MAX_WINDOWTEXT ];
	DWORD dwProcessId;
	DWORD dwThreadId[ MAX_THREAD_CNT ];
	WORD wThreadCount;
	int iIFF;
	int bWatch;
} TARGETINFO, *LPTARGETINFO;

typedef struct {
	HWND myHwnd;
	int iMyId;
	LPTARGETINFO lpTarget;
	LPTSTR lpszStatus[ 16 ];

} HACK_PARAMS, *LPHACK_PARAMS;

DWORD WINAPI Hack( LPVOID dwTargetProcessId );

BOOL SetTargetPlus( const HWND hWnd, HANDLE * phChildThread, LPHACK_PARAMS lphp );
BOOL SetTargetPlus( const HWND hWnd, HANDLE * phChildThread, LPHACK_PARAMS lphp, LPCTSTR lpszTargetPath, LPCTSTR lpszTargetExe );

BOOL Unfreeze( HWND hWnd, LPTARGETINFO lpTarget );

VOID CALLBACK DummyCallback( HWND hWnd, UINT uMsg, DWORD dwData, LRESULT lResult );

LRESULT CALLBACK xList(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK Question( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );

int UpdateProcessSnap( LPTARGETINFO target, UINT uMaxTargets );

DWORD PathToProcess( LPCTSTR lpszPath );

BOOL PathToExe( LPCTSTR lpszPath, LPTSTR lpszExe, const int iBufferSize );
BOOL PathToExe( LPTSTR lpszPath );
BOOL PathToExeEx( LPTSTR lpszPath, int iBufferSize );

BOOL GetProcessInfoMsg( LPTARGETINFO lpTarget, LPTSTR msg, int iBufferSize );


BOOL OpenDebugLog();
BOOL PrintFileHeader( FILE * fp );
BOOL WriteDebugLog( LPCTSTR str );
BOOL CloseDebugLog();

BOOL GetIniPath( LPTSTR lpszPath );
BOOL ReadIni( VOID );

WORD GetLanguage( VOID );
BOOL CheckLanguageMenuRadio( const HWND hWnd );
VOID InitMenuEng( const HWND hWnd );
VOID InitToolTipsEng( TCHAR str[ 4 ][ 256 ] );

#ifdef _UNICODE
VOID InitMenuFin( const HWND hWnd );
VOID InitToolTipsFin( WCHAR str[ 4 ][ 256 ] );
VOID InitMenuJpn( const HWND hWnd );
VOID InitToolTipsJpn( WCHAR str[ 4 ][ 256 ] );
VOID InitMenuSpa( const HWND hWnd );
VOID InitToolTipsSpa( WCHAR str[ 4 ][ 256 ] );

VOID InitToolTipsChiT( WCHAR str[ 4 ][ 256 ] );
VOID InitToolTipsChiS( WCHAR str[ 4 ][ 256 ] );
VOID InitMenuChiT( const HWND hWnd );
VOID InitMenuChiS( const HWND hWnd );



VOID InitToolTipsFre( WCHAR str[ 4 ][ 256 ] );
VOID InitMenuFre( const HWND hWnd );
#endif


BOOL WriteIni( VOID );

BOOL SetSliderIni( LPCTSTR lpszTarget, const int iSlider );
int GetSliderIni( LPCTSTR lpszTarget );
VOID SetWindowPosIni( const HWND hWnd );
VOID GetWindowPosIni( LPPOINT lppt );




VOID InitSWIni( VOID );
VOID SaveCmdShow( HWND hWnd, int nCmdShow );
int GetCmdShow( HWND hWnd );

VOID ShowProcessWindow( HWND hCurWnd, LPTARGETINFO lpTarget, int iShow );
BOOL BES_ShowWindow( HWND hCurWnd, HWND hwnd, int iShow );
#define BES_ERROR                -600

#define BES_DELETE_KEY           -100
#define BES_HIDE                    0
#define BES_SHOW                    1
#define BES_SHOW_MANUALLY           2

VOID GShow( HWND hWnd );
BOOL GetTargetTitleIni( LPTSTR lpStr, int iMax );


HFONT UpdateFont( HFONT hFont );

int MainWindowUrlHit( LPARAM lParam );
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK xList(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK Settings( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );

VOID AboutShortcuts( HWND hWnd );


BOOL CALLBACK WndEnumProc( HWND hwnd, LPARAM lpwi );
int ListProcessThreads( DWORD dwOwnerPID, DWORD *dwThreadIdTable );
VOID GetProcessDetails( DWORD dwProcessId, LPTARGETINFO lpTargetInfo );


BOOL SaveSnap( const HWND hWnd );
int SaveSnap( LPCTSTR lpszSavePath );

VOID AdjustLength( LPTSTR lptstr );


BOOL SetSliderText( HWND hDlg, int iControlId, LONG lPercentage );
BOOL SliderMoved( WPARAM wParam );
BYTE GetSliderParam( LONG lSlider );

int GetArgument( LPCTSTR lpszCmdLine, int iBufferSize, LPTSTR lpszMyPath, LPTSTR lpszTargetPath, LPTSTR lpszTargetExe );


VOID SetRealTimeMode( const HWND hWnd, const BOOL bRealTime, const HANDLE * phChildThread, LPTSTR lpszWindowText );


VOID Exit_CommandFromTaskbar( const HWND hWnd );

int InitNotifyIconData( HWND hWnd, NOTIFYICONDATA * lpni, TARGETINFO * ti );


HWND CreateTooltip ( const HINSTANCE hInst, const HWND hwndBase, LPCTSTR str );
VOID UpdateTooltip( const HINSTANCE hInst, const HWND hwndBase, LPCTSTR str, const HWND hwndToolTip );


VOID Unwatch( HANDLE& hSema, HANDLE& hThread, BOOL * lpbWatch );









BOOL SSTP_Aviutl( HWND hWnd, LPCTSTR lpTitle );

BOOL DirectSSTP( HWND hWnd, char * hcScript );
BOOL DirectSSTP( HWND hWnd, char * hcSakura, char * hcKero );
BOOL DirectSSTP( HWND hWnd, char * hcSakura, char * hcKero, char * hcCharset );
VOID DirectSSTP( LPCTSTR str );


#define BTN_X1     480
#define BTN_WIDTH  130
#define BTN_X2 ( BTN_X1 + BTN_WIDTH )

#define BTN_HEIGHT_LARGE  75
#define BTN_HEIGHT_SMALL  50

#define BTN0_Y1  30
#define BTN0_Y2 ( BTN0_Y1 + BTN_HEIGHT_LARGE )
#define BTN1_Y1 125
#define BTN1_Y2 ( BTN1_Y1 + BTN_HEIGHT_SMALL )
#define BTN2_Y1 195
#define BTN2_Y2 ( BTN2_Y1 + BTN_HEIGHT_LARGE )
#define BTN3_Y1 330
#define BTN3_Y2 ( BTN3_Y1 + BTN_HEIGHT_SMALL )



int GetButtonId( POINT pt );


#ifndef HIMETRIC_INCH
	#define HIMETRIC_INCH 2540
#endif

HFONT MyCreateFont( const HDC hDC, LPCTSTR lpszFace, int iPoint, BOOL bBold, BOOL bItalic );
HFONT MyCreateFont( LPCTSTR lpszFace, int iSize, BOOL bBold, BOOL bItalic );

#define IGNORE_ARGV 200

inline BOOL EnableLoggingIni( BOOL bEnabled )
{
	TCHAR lpszPath[ MAX_PATH * 2 ];
	GetIniPath( lpszPath );

	BOOL bSuccessful = WritePrivateProfileString(
		TEXT( "Options" ), 
		TEXT( "Logging" ),
		bEnabled ? TEXT( "1" ) : TEXT( "0" ),
		lpszPath
	);

	if( ! bSuccessful )
	{
		MessageBox( NULL,
			TEXT("Failed to write .ini for the logging settings."),
			APP_NAME, MB_OK | MB_ICONEXCLAMATION );
	}
	return bSuccessful;
}

inline WORD SetLanguage( const WORD wLID )
{
	static WORD wLanguageId;
	
	if( wLID == (WORD) 0 )
	{
		if( wLanguageId == (WORD) 0 ) wLanguageId = LANG_ENGLISH;
	}
	else
	{
		wLanguageId = wLID;
		if( wLanguageId != LANG_ENGLISH && wLanguageId != LANG_FINNISH
			&& wLanguageId != LANG_FRENCH
			&& wLanguageId != LANG_JAPANESE && wLanguageId != LANG_JAPANESEo
			&& wLanguageId != LANG_CHINESE_T && wLanguageId != LANG_CHINESE_S 
			&& wLanguageId != LANG_SPANISH )
		{
			wLanguageId = LANG_ENGLISH;
		}
	}
	return wLanguageId;		
}

inline WORD GetLanguage()
{
	return SetLanguage( (WORD) 0 );
}

inline BOOL IsAbsFoe( LPCTSTR lpExe, LPCTSTR lpPath )
{
	return
		(
			lstrcmpi( TEXT( "lame.exe" ), lpExe ) == 0
				||
			lstrcmpi( TEXT( "aviutl.exe" ), lpExe ) == 0
				||
			lstrcmpi( TEXT( "virtualdubmod.e" ), lpExe ) == 0
				||
			lstrcmpi( TEXT( "virtualdub.exe" ), lpExe ) == 0
				||
			lstrcmpi( TEXT( "virtualdubmod.exe" ), lpExe ) == 0
				||
			lstrcmpi( TEXT( "ffmpeg.exe" ), lpExe ) == 0
				||
			_tcsstr( lpPath, TEXT( "VirtualDub" )  ) != NULL
	);
}

inline BOOL IsAbsFoe( LPCTSTR lpPath )
{
	if( lpPath == NULL ) return FALSE;
	int len = lstrlen( lpPath );
	for( int i = len - 1; i >= 1; i-- )
	{
		if( lpPath[ i - 1 ] == TEXT( '\\' ) ) break;
	}
	return IsAbsFoe( &lpPath[ i ], lpPath );
}

inline BOOL GetSWIniPath( LPTSTR lpszPath )
{
	if( ! GetIniPath( lpszPath ) ) return FALSE;

	int len = lstrlen( lpszPath );
	for( int i = len - 1; i >= 0; i-- )
	{
		if( lpszPath[ i ] == TEXT( '\\' ) )
		{
			lpszPath[ i + 1 ] = TEXT( '\0' );
			break;
		}
	}

	if( i == 0 || i == len - 1 ) return FALSE;

	lstrcat( lpszPath, TEXT( "bes_sw.ini" ) );
	return TRUE;
}


inline BOOL IsWindowsXPOrLater()
{
	static BOOL bResult = -1;
	if( bResult == -1 )
	{
		OSVERSIONINFO osvi;
		osvi.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
		if( ! GetVersionEx( &osvi ) ) bResult = FALSE;
		else
		{
			bResult = (BOOL)
				( 
					osvi.dwMajorVersion == 5UL && osvi.dwMinorVersion >= 1UL
					||
					osvi.dwMajorVersion > 5UL
				);
		}
	}

	return bResult;
}

inline HFONT MyCreateFont( const HDC hDC, LPCTSTR lpszFace, int iPoint, BOOL bBold, BOOL bItalic )
{
	return CreateFont(
				-MulDiv( iPoint, GetDeviceCaps( hDC, LOGPIXELSY ), 72 ),
				0, 0, 0,
				bBold ? FW_BOLD : FW_NORMAL,
				(DWORD) (!! bItalic ),
				0UL, 0UL,
				DEFAULT_CHARSET,
				//ANSI_CHARSET,
				OUT_OUTLINE_PRECIS,
				CLIP_DEFAULT_PRECIS,
				IsWindowsXPOrLater()? CLEARTYPE_QUALITY : ANTIALIASED_QUALITY,
				FF_SWISS | VARIABLE_PITCH,
				lpszFace
			);
}

inline HFONT MyCreateFont( LPCTSTR lpszFace, int iSize, BOOL bBold, BOOL bItalic )
{
	return CreateFont( iSize,
				0, 0, 0,
				bBold ? FW_BOLD : FW_NORMAL,
				(DWORD) (!! bItalic ),
				0UL, 0UL,
				DEFAULT_CHARSET,
				//ANSI_CHARSET,
				OUT_OUTLINE_PRECIS,
				CLIP_DEFAULT_PRECIS,
				IsWindowsXPOrLater()? CLEARTYPE_QUALITY : ANTIALIASED_QUALITY,
				FF_SWISS | VARIABLE_PITCH,
				lpszFace
			);
}

inline int OpenBrowser( LPCTSTR lpUrl )
{
	return (int) ShellExecute(
			(HWND) NULL,
			TEXT( "open" ),
			lpUrl,
			NULL,
			NULL, // lpszCurrentDir,
			SW_SHOWNORMAL
		);
}

inline HFONT GetFontForURL( HDC hdc )
{
	return
		CreateFont(
			-MulDiv( 10, GetDeviceCaps( hdc, LOGPIXELSY ), 72 ),
			0, 0, 0,
			FW_NORMAL,
			0UL, // bItalic
			1UL, // Underline
			0UL, ANSI_CHARSET, OUT_OUTLINE_PRECIS,
			CLIP_DEFAULT_PRECIS,
			ANTIALIASED_QUALITY,
			FF_SWISS | VARIABLE_PITCH,
			TEXT( "Verdana" )
		);
}


inline int GetShell32Version( VOID )
{
	static int iRet = 0;
	if( iRet > 0 ) return iRet;

	TCHAR lpPath[ MAX_PATH * 2 ];
	GetSystemDirectory( lpPath, MAX_PATH );
	lstrcat( lpPath, TEXT( "\\Shell32.dll" ) );
	HINSTANCE hinst = LoadLibrary( lpPath );
	if( ! hinst ) return 0;

	DLLGETVERSIONPROC p = (DLLGETVERSIONPROC) GetProcAddress( hinst, "DllGetVersion" );

	if( p )
	{
		DLLVERSIONINFO dvi;
		ZeroMemory( &dvi, sizeof( dvi ) );
		dvi.cbSize = sizeof( dvi );
		HRESULT hr = ( *p ) ( &dvi );

		if( SUCCEEDED( hr ) )
		{
			iRet = (int) dvi.dwMajorVersion;
		}
	}

	FreeLibrary( (HMODULE) hinst );

	return iRet;
}

#define IsActive() \
	( g_bHack[ 0 ] || g_bHack[ 1 ] || \
	 g_bHack[ 2 ] && g_dwTargetProcessId[ 2 ] != WATCHING_IDLE )

// update the Status messages and enable/disable buttons/menus
inline VOID UpdateStatus( const HWND hWnd )
{
	SendMessage( hWnd, WM_USER_STOP, JUST_UPDATE_STATUS, 0L );
}


//*-------------------------------
#endif // !defined(BATTLEENC_H)
