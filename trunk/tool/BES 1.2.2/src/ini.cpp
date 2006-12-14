#include <time.h>
#include "BattleEnc.h"



BOOL g_bSelChange = FALSE;

extern BOOL g_bRealTime;
extern BOOL g_bLogging;
extern BYTE g_Slider[ 4 ];

extern TCHAR g_lpszEnemy[ MAX_PROCESS_CNT ][ MAX_PATH * 2 ];
extern int g_iEnemyIndex;
extern TCHAR g_lpszFriend[ MAX_PROCESS_CNT ][ MAX_PATH * 2 ];
extern int g_iFriendIndex;

#define BUF_SIZE MAX_PATH * 2 * MAX_PROCESS_CNT


BOOL GetIniPath( LPTSTR lpszPath )
{
	if( lpszPath == NULL ) return FALSE;

	static TCHAR lpszIniFilePath[ MAX_PATH * 2 ] = TEXT( "" );

	TCHAR szBuf[ MAX_PATH * 2 ] = TEXT( "" );

	if( lstrlen( lpszIniFilePath ) == 0 )
	{

		GetModuleFileName( NULL, szBuf, MAX_PATH * 2UL );

		int len = lstrlen( szBuf );
		
		for( int i = len - 1; i >= 0; i-- )
		{
			if( szBuf[ i ] == TEXT( '\\' ) )
			{
				szBuf[ i ] = TEXT( '\0' );
				lstrcpy( lpszIniFilePath, szBuf ); // _tcscpy
				break;
			}
		}

		if( lpszIniFilePath[ 0 ] == TEXT( '\0' ) )
		{
			GetCurrentDirectory( MAX_PATH, lpszIniFilePath );
		}

		lstrcat( lpszIniFilePath, TEXT( "\\bes.ini" ) );


	}
	lstrcpy( lpszPath, lpszIniFilePath );
	return TRUE;
}



BOOL ReadIni()
{
	TCHAR lpszPath[ MAX_PATH * 2 ];
	GetIniPath( lpszPath );

	g_bLogging = !! GetPrivateProfileInt( TEXT( "Options" ), TEXT( "Logging" ), FALSE, lpszPath );
	EnableLoggingIni( g_bLogging );


	g_bSelChange = !! GetPrivateProfileInt( TEXT( "Options" ), TEXT( "SelChange" ), FALSE, lpszPath );

//0x04 	LANG_CHINESE
//0x01 	SUBLANG_CHINESE_TRADITIONAL 	Chinese (Traditional)
//0x02 	SUBLANG_CHINESE_SIMPLIFIED 	Chinese (Simplified)
//	WORD wDefLanguage = (WORD) PRIMARYLANGID( GetSystemDefaultLangID() );
	WORD wDefLanguage = (WORD) PRIMARYLANGID( GetUserDefaultLangID() );
	

	if( wDefLanguage == LANG_CHINESE )
	{
		WORD wSubLangId = (WORD) SUBLANGID( GetSystemDefaultLangID() );
		wDefLanguage = ( wSubLangId == SUBLANG_CHINESE_TRADITIONAL )? LANG_CHINESE_T : LANG_CHINESE_S ;
	}
	else if( wDefLanguage != LANG_ENGLISH && wDefLanguage != LANG_FINNISH && wDefLanguage != LANG_JAPANESE
		&& wDefLanguage != LANG_SPANISH )
	{
		wDefLanguage = LANG_ENGLISH;
	}

	WORD wLanguage = (WORD) GetPrivateProfileInt( TEXT( "Options" ), TEXT( "Language" ), (int) wDefLanguage, lpszPath );

	if( wLanguage != LANG_ENGLISH && wLanguage != LANG_FINNISH && wLanguage != LANG_JAPANESE && wLanguage != LANG_JAPANESEo
		&& wLanguage != LANG_CHINESE_T && wLanguage != LANG_CHINESE_S && wDefLanguage != LANG_SPANISH
		&& wLanguage != LANG_FRENCH )
	{
		wLanguage = wDefLanguage;
	}
	
	SetLanguage( wLanguage );

	g_Slider[ 0 ] = (BYTE) 33;
	g_Slider[ 1 ] = (BYTE) 33;
	g_Slider[ 2 ] = (BYTE) 33;
	g_Slider[ 3 ] = (BYTE) 0;

	int i, j;
	TCHAR lpszKey[ 100 ];
	g_iEnemyIndex = 0;
	for( i = 0; i < MAX_PROCESS_CNT; i++ )
	{
		wsprintf( lpszKey, TEXT( "Enemy%d" ), i );
		GetPrivateProfileString(
			TEXT( "Enemy" ),
			lpszKey,
			TEXT( "" ),
			g_lpszEnemy[ g_iEnemyIndex ],
			MAX_PATH + 1,
			lpszPath
		);
		if( lstrlen( g_lpszEnemy[ g_iEnemyIndex ] ) == 0 ) break;
		else if( IsAbsFoe( g_lpszEnemy[ g_iEnemyIndex ] ) ) continue;
		else if( _tcschr( g_lpszEnemy[ g_iEnemyIndex ], TEXT( '\\' ) ) != NULL ) continue;

		PathToExeEx( g_lpszEnemy[ g_iEnemyIndex ], MAX_PATH + 1 );

		for( j = 0; j < g_iEnemyIndex; j++ )
		{
			if( lstrcmpi( g_lpszEnemy[ j ], g_lpszEnemy[ g_iEnemyIndex ] ) == 0 ) goto NEXT_FOE;
		}

		g_iEnemyIndex++;

		NEXT_FOE:
		{
			;
		}
	}

	g_iFriendIndex = 0;
	for( i = 0; i < MAX_PROCESS_CNT; i++ )
	{
		wsprintf( lpszKey, TEXT( "Friend%d" ), i );
		GetPrivateProfileString(
			TEXT( "Friend" ),
			lpszKey,
			TEXT( "" ),
			g_lpszFriend[ g_iFriendIndex ],
			MAX_PATH + 1,
			lpszPath
		);
	
		if( lstrlen( g_lpszFriend[ g_iFriendIndex ] ) == 0 ) break;
		else if( IsAbsFoe( g_lpszEnemy[ g_iFriendIndex ] ) ) continue;
		else if( _tcschr( g_lpszEnemy[ g_iFriendIndex ], TEXT( '\\' ) ) != NULL ) continue;

		PathToExeEx( g_lpszFriend[ g_iFriendIndex ], MAX_PATH + 1 );

		for( j = 0; j < g_iFriendIndex; j++ )
		{
			if( lstrcmpi( g_lpszFriend[ j ], g_lpszFriend[ g_iFriendIndex ] ) == 0 ) goto NEXT_FRIEND;
		}
		for( j = 0; j < g_iEnemyIndex; j++ )
		{
			if( lstrcmpi( g_lpszEnemy[ j ], g_lpszFriend[ g_iFriendIndex ] ) == 0 ) goto NEXT_FRIEND;
		}

		g_iFriendIndex++;

		NEXT_FRIEND:
		{
			;
		}
	}
	return TRUE;
}


BOOL WriteIni()
{
	TCHAR lpBuffer[ BUF_SIZE ];
	TCHAR lpszPath[ MAX_PATH * 2 ];
	GetIniPath( lpszPath );

	// Delete old keys/values
	WritePrivateProfileString( TEXT( "Slider" ), TEXT( "Slider0" ),	NULL, lpszPath );
	WritePrivateProfileString( TEXT( "Slider" ), TEXT( "Slider1" ),	NULL, lpszPath );
	WritePrivateProfileString( TEXT( "Slider" ), TEXT( "Slider2" ),	NULL, lpszPath );
	WritePrivateProfileString( TEXT( "Options" ), TEXT( "RealTime" ), NULL, lpszPath );

	WritePrivateProfileString(
		TEXT( "Options" ), 
		TEXT( "RealTime" ),
		g_bRealTime ? TEXT( "1" ) : TEXT( "0" ),
		lpszPath
	);

	TCHAR lpszLangId[ 100 ];
	wsprintf( lpszLangId, TEXT( "%d" ), (int) GetLanguage() );
	WritePrivateProfileString(
		TEXT( "Options" ), 
		TEXT( "Language" ),
		lpszLangId,
		lpszPath
	);

	ZeroMemory( lpBuffer, sizeof( TCHAR ) * BUF_SIZE );
	LPTSTR ptr = lpBuffer;

	int i;

	for( i = 0; i < g_iEnemyIndex; i++ )
	{
		wsprintf( ptr, TEXT( "Enemy%d=%s" ), i, g_lpszEnemy[ i ] );
		ptr += ( lstrlen( ptr ) + 1 ); 
	}
	WritePrivateProfileSection( TEXT( "Enemy" ), lpBuffer, lpszPath );

	ZeroMemory( lpBuffer, sizeof( TCHAR ) * BUF_SIZE );
	ptr = lpBuffer;
	for( i = 0; i < g_iFriendIndex; i++ )
	{
		wsprintf( ptr, TEXT( "Friend%d=%s" ), i, g_lpszFriend[ i ] );
		ptr += ( lstrlen( ptr ) + 1 ); 
	}
	WritePrivateProfileSection( TEXT( "Friend" ), lpBuffer, lpszPath );

	return TRUE;
}


BOOL SetSliderIni( LPCTSTR lpszString, const int iSlider )
{
	if( iSlider <= 0 || iSlider > 99 || lstrlen( lpszString ) > 1000 ) return FALSE;

	WriteDebugLog( TEXT( "SetSliderIni" ) );
	WriteDebugLog( lpszString );


	TCHAR lpszExeName[ MAX_PATH * 2 ] = _T( "" );
/* - 1.1b7
	int len = lstrlen( lpszTarget );
	int start = -1;
	int end = -1;
	for( int i = len - 1; i >= 5; i-- )
	{
		if(
			lpszTarget[ i ] == TEXT( ' ' ) &&
			( lpszTarget[ i - 1 ] == TEXT( 'e' ) || lpszTarget[ i - 1 ] == TEXT( 'E' ) ) &&
			( lpszTarget[ i - 2 ] == TEXT( 'x' ) || lpszTarget[ i - 2 ] == TEXT( 'X' ) ) &&
			( lpszTarget[ i - 3 ] == TEXT( 'e' ) || lpszTarget[ i - 3 ] == TEXT( 'E' ) ) &&
			lpszTarget[ i - 4 ] == TEXT( '.' )
		)
		{
			end = i;
			break;
		}
	}

	for( ; i >=0; i-- )
	{
		if( lpszTarget[ i ] == TEXT( '\\' ) )
		{
			start = i + 1;
			break;
		}
	}

	if( start == -1 ) start = 0;

	if( end == -1 || end - start <= 0 )
	{
		TCHAR dbg[1000];
		wsprintf( dbg, TEXT("DEBUG: %s %d %d %d"), lpszTarget, start, end, end-start );
		WriteDebugLog( dbg ) ;

		return FALSE;
	}



	lstrcpy( lpszExeName, &lpszTarget[ start ] );
	lpszExeName[ end - start ] = TEXT( '\0' );
*/
// +1.1b7
	PathToExe( lpszString, lpszExeName, MAX_PATH * 2 );


#if !defined( _UNICODE )
	if( lstrlen( lpszExeName ) >= 19 )
	{
		lpszExeName[ 15 ] = '\0';
		PathToExeEx( lpszExeName, MAX_PATH * 2 );
	}
#endif


	TCHAR lpszExeNameLower[ MAX_PATH * 2 ] = _TEXT( "" );
	int len = lstrlen( lpszExeName );
	for( int i = 0; i < len; i++ )
	{
		TCHAR c = lpszExeName[ i ];
		if( _istascii( c ) && _istupper( c ) )
		{
			lpszExeNameLower[ i ] = (TCHAR) _totlower( c );
		}
		else
		{
			lpszExeNameLower[ i ] = c;
		}
	}
	//lpszExeNameLower[ i ] = TEXT( '\0' );



	TCHAR lpszPath[ MAX_PATH * 2 ];
	GetIniPath( lpszPath );

	TCHAR tmpstr[ 100 ];
	wsprintf( tmpstr, TEXT( "%d" ), iSlider );


	WritePrivateProfileString(
		TEXT( "Slider" ), 
		lpszExeNameLower,
		tmpstr,
		lpszPath
	);

	// to flushes the cache
	WritePrivateProfileString(
		NULL,
		NULL,
		NULL,
		lpszPath
	);

	return TRUE;
}

int GetSliderIni( LPCTSTR lpszTargetPath )
{
	TCHAR lpszExeName[ MAX_PATH * 2 ];
	TCHAR lpszExeNameLower[ MAX_PATH * 2 ];

	if( lpszTargetPath == NULL ) return 33;

	const int len = lstrlen( lpszTargetPath );
	int start = 0;
	int i;
	for( i = len - 1; i >= 0; i-- )
	{
		if( lpszTargetPath[ i ] == TEXT( '\\' ) )
		{
			start = i + 1;
			break;
		}
	}

	lstrcpy( lpszExeName, &lpszTargetPath[ start ] );
#if !defined( _UNICODE )
	if( lstrlen( lpszExeName ) >= 19 )
	{
		lpszExeName[ 15 ] = '\0';
		PathToExeEx( lpszExeName, MAX_PATH * 2 );
	}
#endif
	const int len2 = lstrlen( lpszExeName );
	for( i = 0; i < len2; i++ )
	{
		TCHAR c = lpszExeName[ i ];
		if( _istascii( c ) && _istupper( c ) )
		{
			lpszExeNameLower[ i ] = (TCHAR) _totlower( c );
		}
		else
		{
			lpszExeNameLower[ i ] = c;
		}
	}
	lpszExeNameLower[ i ] = TEXT( '\0' );

	TCHAR lpszPath[ MAX_PATH * 2 ];
	GetIniPath( lpszPath );


	// to flushes the cache --- Just in case!
	WritePrivateProfileString(
		NULL,
		NULL,
		NULL,
		lpszPath
	);

	int iSlider = GetPrivateProfileInt( TEXT( "Slider" ), lpszExeNameLower, 33, lpszPath );
	if( iSlider < 0 || iSlider > 99 ) iSlider = 33;
	else if( iSlider == 0 ) iSlider = 1; // for backword compat.
	return iSlider;
}

VOID SetWindowPosIni( const HWND hWnd )
{
	RECT rect;
	if( ! GetWindowRect( hWnd, &rect ) ) return;

	TCHAR lptstrIniPath[ MAX_PATH * 2 ];
	GetIniPath( lptstrIniPath );

	TCHAR lptstrX[ 100 ];
	TCHAR lptstrY[ 100 ];

	int x = (int) rect.left;
	if( x < 0 ) x = 0;
	int y = (int) rect.top;
	if( y < 0 ) y = 0;
	wsprintf( lptstrX, TEXT( "%d" ), x );
	wsprintf( lptstrY, TEXT( "%d" ), y );

	WritePrivateProfileString(
		TEXT( "Window" ), 
		TEXT( "posX" ),
		lptstrX,
		lptstrIniPath
	);

	WritePrivateProfileString(
		TEXT( "Window" ), 
		TEXT( "posY" ),
		lptstrY,
		lptstrIniPath
	);
}

VOID GetWindowPosIni( LPPOINT lppt )
{
	TCHAR lptstrIniPath[ MAX_PATH * 2 ];
	GetIniPath( lptstrIniPath );
	int x = GetPrivateProfileInt( TEXT( "Window" ), TEXT( "PosX" ), CW_USEDEFAULT, lptstrIniPath );
    int y = GetPrivateProfileInt( TEXT( "Window" ), TEXT( "PosY" ), CW_USEDEFAULT, lptstrIniPath );

	int maxX = GetSystemMetrics( SM_CXSCREEN ) - 640;
	int maxY = GetSystemMetrics( SM_CYSCREEN ) - 480;

	if( x != CW_USEDEFAULT && x > maxX ) x = maxX;
	if( y != CW_USEDEFAULT && y > maxY ) y = maxY;

	lppt->x = x, lppt->y = y;
}

BOOL GetTargetTitleIni( LPTSTR lpStr, int iMax )
{
	TCHAR lpIniPath[ MAX_PATH * 2 ];
	GetIniPath( lpIniPath );

	GetPrivateProfileString(
		TEXT( "Options" ),
		TEXT( "GShow" ),
		NULL,
		lpStr,
		iMax,
		lpIniPath
	);

	return ( lpStr != NULL );
}



VOID SaveCmdShow( HWND hWnd, int nCmdShow )
{
	TCHAR lpIniPath[ MAX_PATH * 2 ] = TEXT( "" );
	if( ! GetSWIniPath( lpIniPath ) )
	{
		GetTempPath( MAX_PATH, lpIniPath );
		lstrcat( lpIniPath, TEXT( "bes_sw.ini" ) );
	}

	TCHAR strHwnd[ 100 ];
	wsprintf( strHwnd, TEXT( "%08lX" ), (DWORD) hWnd );
	TCHAR strCmdShow[ 100 ];
	wsprintf( strCmdShow, TEXT( "%d" ), nCmdShow );

	WritePrivateProfileString(
		TEXT( "nCmdShow" ),
		strHwnd,
		( nCmdShow == BES_DELETE_KEY )? NULL : strCmdShow,
		lpIniPath
	);

}

int GetCmdShow( HWND hWnd )
{
	TCHAR lptstrIniPath[ MAX_PATH * 2 ] = TEXT( "" );
	if( ! GetSWIniPath( lptstrIniPath ) )
	{
		GetTempPath( MAX_PATH, lptstrIniPath );
		lstrcat( lptstrIniPath, TEXT( "bes_sw.ini" ) );
	}

	TCHAR strHwnd[ 100 ];
	wsprintf( strHwnd, TEXT( "%08lX" ), (DWORD) hWnd );
	return GetPrivateProfileInt(
		TEXT( "nCmdShow" ),
		strHwnd,
		BES_ERROR, // if not found
		lptstrIniPath
	);
}

VOID InitSWIni( VOID )
{
	TCHAR lpIniPath[ MAX_PATH * 2 ] = TEXT( "" );
	if( ! GetSWIniPath( lpIniPath ) )
	{
		GetTempPath( MAX_PATH, lpIniPath );
		lstrcat( lpIniPath, TEXT( "bes_sw.ini" ) );
	}

	int iPrevBootTime = GetPrivateProfileInt( TEXT( "Signature" ), TEXT( "Boot" ), 0, lpIniPath );

	int iNow = (int) (time_t) time( ( time_t * ) NULL );

	int iBootTime = iNow - (int)( GetTickCount() / 1000UL );

	if( abs( iBootTime - iPrevBootTime ) > 4 )
	{
		TCHAR strBootTime[ 100 ];
		wsprintf( strBootTime, TEXT( "%u" ), (UINT) iBootTime );

		WritePrivateProfileString(
			TEXT( "Signature" ),
			TEXT( "Boot" ),
			strBootTime,
			lpIniPath
		);

		WritePrivateProfileString( TEXT( "nCmdShow" ), NULL, NULL, lpIniPath );
	}
}


