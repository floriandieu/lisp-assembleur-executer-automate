#include "BattleEnc.h"


TCHAR g_lpszLogFileName[ MAX_PATH * 2 ];

extern g_bLogging;

BOOL OpenDebugLog()
{
	if( !g_bLogging ) return FALSE;

	
	GetModuleFileName( NULL, g_lpszLogFileName, MAX_PATH * 2UL - 64UL );
	const int len = lstrlen( g_lpszLogFileName );
	for( int i = len - 1; i >= 0; i-- )
	{
		if( g_lpszLogFileName[ i ] == TEXT( '\\' ) )
		{
			g_lpszLogFileName[ i ] = TEXT( '\0' );
			break;
		}
	}

	if( g_lpszLogFileName[ 0 ] == TEXT( '\0' ) )
	{
		GetCurrentDirectory( MAX_PATH * 2 - 64, g_lpszLogFileName );
	}

	TCHAR tmpstr[ 64 ] = TEXT( "" );
	SYSTEMTIME st;
	GetLocalTime( &st );
	wsprintf( tmpstr, TEXT( "\\bes-%04d%02d%02d.log" ), st.wYear, st.wMonth, st.wDay );


	lstrcat( g_lpszLogFileName, tmpstr );

	FILE * fdebug = _tfopen( g_lpszLogFileName, TEXT( "ab" ) );
	if( fdebug == NULL ) return FALSE;

#ifdef _UNICODE
	fputc( 0xFF, fdebug );
	fputc( 0xFE, fdebug );
#endif
	_fputts( TEXT( "-------- START --------\r\n" ), fdebug );

	PrintFileHeader( fdebug );

	fclose( fdebug );
	return TRUE;
}

BOOL PrintFileHeader( FILE * fp )
{
	TCHAR str[ 1024 ];
	_fputts( APP_NAME, fp );

#ifdef _UNICODE
	_fputts( TEXT( " (Unicode)\r\n" ), fp );
#else
	_fputts( TEXT( " (ANSI)\r\n" ), fp );
#endif

	SYSTEMTIME st;
	GetLocalTime( &st );

	TIME_ZONE_INFORMATION tzi;
	GetTimeZoneInformation( &tzi );
	int tz = abs( (int) tzi.Bias );
	int tz_sign = ( tzi.Bias < 0 ) ? ( +1 ) : ( -1 ) ; // if Bias is negative, TZ is positive
	int tz_h = tz / 60;
	int tz_m = tz - tz_h * 60;

	wsprintf( str, TEXT( "  %4d-%02d-%02d %02d:%02d:%02d.%03d%s%02d:%02d\r\n\r\n" ),
		st.wYear, st.wMonth, st.wDay,
		st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
		tz_sign > 0 ? TEXT( "+" ) : TEXT( "-" ), tz_h, tz_m );
	_fputts( str, fp );

	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
	GetVersionEx( &osvi );
	TCHAR tmpstr[ 256 ] = TEXT( "Unknown OS" );

	if( osvi.dwMajorVersion == 4UL )
	{
		if( osvi.dwMinorVersion == 0UL )
			lstrcpy( tmpstr, TEXT( "Windows NT 4.0" ) );
		else if( osvi.dwMinorVersion == 90UL )
			lstrcpy( tmpstr, TEXT( "Windows Me" ) );
	}
	else if( osvi.dwMajorVersion == 5UL )
	{
		if( osvi.dwMinorVersion == 0UL )
			lstrcpy( tmpstr, TEXT( "Windows 2000" ) );
		else if( osvi.dwMinorVersion == 1UL )
			lstrcpy( tmpstr, TEXT( "Windows XP" ) );
		else if( osvi.dwMinorVersion == 2UL )
			lstrcpy( tmpstr, TEXT( "Windows Server 2003 R2, Windows Server 2003, or Windows XP Professional x64 Edition" ) );
	}
	else if( osvi.dwMajorVersion == 6UL )
	{
		if( osvi.dwMinorVersion == 0UL )
			lstrcpy( tmpstr, _T( "Windows Vista or Windows Server \"Longhorn\"" ) );
	}


	wsprintf( str, TEXT( "%s ( OS Version: %lu.%lu Build %lu, %s )\r\n" ),
		tmpstr,
		osvi.dwMajorVersion,
		osvi.dwMinorVersion,
		osvi.dwBuildNumber,
		osvi.szCSDVersion
	);
	_fputts( str, fp );

	LANGID lang = GetSystemDefaultLangID();
	VerLanguageName( (DWORD) lang, tmpstr, 255UL );
	wsprintf( str, TEXT( "  System Language: %u (%s)\r\n" ), lang, tmpstr );
	_fputts( str, fp );
	LCID lcid = GetSystemDefaultLCID();
	GetLocaleInfo( lcid, LOCALE_SNATIVELANGNAME, tmpstr, 255UL );
	wsprintf( str, TEXT( "  System Locale:   %lu (%s)\r\n" ), lcid, tmpstr );
	_fputts( str, fp );

	lang = GetUserDefaultLangID();
	VerLanguageName( (DWORD) lang, tmpstr, 255UL );
	wsprintf( str, TEXT( "  User Language:   %u (%s)\r\n" ), lang, tmpstr );
	_fputts( str, fp );

	lcid = GetUserDefaultLCID();
	GetLocaleInfo( lcid, LOCALE_SNATIVELANGNAME, tmpstr, 255UL );
	wsprintf( str, TEXT( "  User Locale:     %lu (%s)\r\n" ), lcid, tmpstr );
	_fputts( str, fp );
	_fputts( TEXT( "\r\n" ), fp );

	return TRUE;
}



BOOL CloseDebugLog()
{
	if( !g_bLogging ) return FALSE;

	FILE * fdebug = _tfopen( g_lpszLogFileName, TEXT( "ab" ) );
	if( fdebug == NULL )
	{
		Sleep( 3000UL );
		fdebug = _tfopen( g_lpszLogFileName, TEXT( "ab" ) );
	}

	if( fdebug == NULL )
	{
		MessageBox( NULL, TEXT( "CloseDebugLog() failed." ),
			APP_NAME, MB_ICONEXCLAMATION | MB_OK );
		return FALSE;
	}

	_fputts( TEXT( "\r\n\r\n" ), fdebug );
	SYSTEMTIME st;
	GetLocalTime( &st );
	TCHAR lpszTime[ 1024 ];
	wsprintf( lpszTime, TEXT( "[ %04d-%02d-%02d %02d:%02d:%02d.%03d ] " ),
		st.wYear, st.wMonth, st.wDay,
		st.wHour, st.wMinute, st.wSecond, st.wMilliseconds );
	_fputts( lpszTime, fdebug );
	_fputts( TEXT( "-------- END --------\r\n\r\n\r\n" ), fdebug );
	return fclose( fdebug );
}


BOOL WriteDebugLog( LPCTSTR str )
{
	if( !g_bLogging ) return FALSE;
	FILE * fdebug = _tfopen( g_lpszLogFileName, TEXT( "ab" ) );
	if( fdebug == NULL ) return FALSE;
	if( lstrlen( str ) > 0 )
	{
		SYSTEMTIME st;
		GetLocalTime( &st );
		TCHAR lpszTime[ 1024 ];
		wsprintf( lpszTime, TEXT( "[ %04d-%02d-%02d %02d:%02d:%02d.%03d ] " ),
			st.wYear, st.wMonth, st.wDay,
			st.wHour, st.wMinute, st.wSecond, st.wMilliseconds 
		);
		_fputts( lpszTime, fdebug );
		_fputts( str, fdebug );
	}
	_fputts( TEXT( "\r\n" ), fdebug );
	fclose( fdebug );
	return TRUE;

}


