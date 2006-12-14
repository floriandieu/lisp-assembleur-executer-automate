#include "BattleEnc.h"

extern DWORD g_dwTargetProcessId[ 4 ];
extern BYTE g_Slider[ 4 ];
extern HANDLE hSemaphore[ 4 ];
extern TCHAR g_szTarget[ 4 ][ 1024 ];
extern BOOL g_bHack[ 4 ];
extern BOOL g_bRealTime;


DWORD WINAPI LimitPlus( LPVOID lpVoid )
{
	int i, j;
	LPHACK_PARAMS lphp = (LPHACK_PARAMS) lpVoid;
	const HWND hWnd = lphp->myHwnd;
	const LPTARGETINFO lpTarget = lphp->lpTarget;

	EnableMenuItem( GetMenu( hWnd ), IDM_WATCH, MF_BYCOMMAND | MF_GRAYED );
	EnableMenuItem( GetMenu( hWnd ), IDM_UNWATCH, MF_BYCOMMAND | MF_ENABLED );

	g_dwTargetProcessId[ 2 ] = WATCHING_IDLE;
	UpdateStatus( hWnd );

	TCHAR lpszTargetPath[ MAX_PATH * 2 ];
	TCHAR lpszTargetExe[ MAX_PATH * 2 ];
	lstrcpy( lpszTargetPath, lpTarget->szPath );
	lstrcpy( lpszTargetExe, lpTarget->szExe );
	wsprintf( g_szTarget[ 2 ], TEXT( "%s (Watching)" ), lpszTargetPath );

	for( i = 8; i < 12; i++ )
	{
		lstrcpy( lphp->lpszStatus[ i ], TEXT( "" ) );
	}

	lstrcpy( lphp->lpszStatus[ 13 ], TEXT( "Watching..." ) );


	TCHAR str[ MAX_PATH * 2 ];

#ifdef _UNICODE
	lstrcpy( lphp->lpszStatus[ 14 ], lpszTargetPath );
#else
	lstrcpy( str, lpszTargetPath );
	if( lstrlen( str ) >= 19 )
	{
		str[ 15 ] = '\0';
		PathToExeEx( str, MAX_PATH * 2 );
	}
	lstrcpy( lphp->lpszStatus[ 14 ], str );
#endif

	wsprintf( str, TEXT( "Watching : %s" ), lpszTargetPath );
	WriteDebugLog( str );

	AdjustLength( lphp->lpszStatus[ 14 ] );

	g_Slider[ 2 ] = (BYTE) GetSliderIni( lpszTargetPath );
	
	for( ; ; )
	{
		if( lphp->lpTarget->dwProcessId == TARGET_PID_NOT_SET )
		{
			g_dwTargetProcessId[ 2 ] = PathToProcess( lpszTargetPath );
		}
		else
		{
			g_dwTargetProcessId[ 2 ] = lphp->lpTarget->dwProcessId;
			wsprintf( str, TEXT( "Watching : ProcessID given : %08lX" ), g_dwTargetProcessId[ 2 ] );
			WriteDebugLog( str );
		}

		if( g_dwTargetProcessId[ 2 ] == g_dwTargetProcessId[ 0 ] )
		{
			if( g_bHack[ 0 ] )
			{
				SendMessage( hWnd, WM_USER_STOP, 0U, 0L );
			}

			Sleep( 500UL );
			
			for( i = 0; i < 4; i++ )
			{	
				lstrcpy( lphp->lpszStatus[ i ], TEXT( "" ) );
			}
			g_dwTargetProcessId[ 0 ] = TARGET_PID_NOT_SET;
			lstrcpy( g_szTarget[ 0 ], TARGET_UNDEF );
		}
		else if( g_dwTargetProcessId[ 2 ] == g_dwTargetProcessId[ 1 ] )
		{
			if( g_bHack[ 1 ] )
			{
				SendMessage( hWnd, WM_USER_STOP, 1U, 0L );
			}

			Sleep( 500UL );
			
			for( i = 4; i < 8; i++ )
			{
				lstrcpy( lphp->lpszStatus[ i ], TEXT( "" ) );
			}
			g_dwTargetProcessId[ 1 ] = TARGET_PID_NOT_SET;
			lstrcpy( g_szTarget[ 1 ], TEXT( "<target not specified>" ) );
		}


		if( g_dwTargetProcessId[ 2 ] == (DWORD) -1 )
		{
			if( lstrcmp( lphp->lpszStatus[ 15 ], TEXT( "Not found" ) ) != 0 )
			{
				wsprintf( lphp->lpszStatus[ 15 ], TEXT( "Not found" ) );
				InvalidateRect( hWnd, NULL, TRUE );
			}
			for( j = 0; j < 80; j++ ) // wait 8 seconds
			{
				Sleep( 100UL );
				if( ! g_bHack[ 3 ] ) goto EXIT_THREAD;
			}
		}
		else
		{
			wsprintf( str, TEXT( "Watching : Found! <%s>" ), lpszTargetPath );
			WriteDebugLog( str );
			lphp->lpTarget->dwProcessId = g_dwTargetProcessId[ 2 ];
			lstrcpy( lphp->lpTarget->szPath, lpszTargetPath );
			lstrcpy( lphp->lpTarget->szExe , lpszTargetExe );
			lstrcpy( lphp->lpTarget->szText, TEXT( "" ) );
			g_Slider[ 2 ] = (BYTE) GetSliderIni( lpszTargetPath );

			SendMessage( hWnd, WM_USER_HACK, (WPARAM) 2, (LPARAM) lphp );
			
			UpdateStatus( hWnd );

			wsprintf( lphp->lpszStatus[ 15 ], TEXT( "Found ( %08lX )" ), g_dwTargetProcessId[ 2 ] );
			InvalidateRect( hWnd, NULL, TRUE );

			while( g_dwTargetProcessId[ 2 ] != TARGET_PID_NOT_SET )
			{
				Sleep( 100UL );
				if( g_bHack[ 3 ] == FALSE )
				{
					WriteDebugLog( TEXT( "g_bHack[ 3 ] == FALSE" ) );

					goto EXIT_THREAD;
				}
			}
			
			if( WaitForSingleObject( hSemaphore[ 2 ], 1000UL ) != WAIT_OBJECT_0 )
			{
				g_bHack[ 2 ] = FALSE;
				MessageBox( hWnd, TEXT("Sema Error in LimitPlus()"), APP_NAME, MB_OK | MB_ICONEXCLAMATION );
				goto EXIT_THREAD;
			}
			
			g_dwTargetProcessId[ 2 ] = WATCHING_IDLE;
			UpdateStatus( hWnd );
		}

	}

EXIT_THREAD:
	WriteDebugLog( TEXT( "Watch: Exiting Thread..." ) );

	// if the Watched is idle, we will free the slot #2 when 'unwatch' is posted
	// (Otherwise, #2 should be still active)
	if( g_dwTargetProcessId[ 2 ] == WATCHING_IDLE )
	{
		g_dwTargetProcessId[ 2 ] = TARGET_PID_NOT_SET;
		lphp->lpTarget->dwProcessId = TARGET_PID_NOT_SET;
		g_bHack[ 2 ] = FALSE;
		ReleaseSemaphore( hSemaphore[ 2 ], 1L, (LONG *) NULL );
	}

	if( ReleaseSemaphore( hSemaphore[ 3 ], 1L, (LONG *) NULL ) )
	{
		WriteDebugLog( TEXT( "* Sema3 Released" ) );
	}
	else
	{
		WriteDebugLog( TEXT( "[!] Sema3 Not released" ) );
	}

	lstrcpy( lphp->lpszStatus[ 13 ], TEXT( "* Not watching *" ) );
	lstrcpy( lphp->lpszStatus[ 14 ], lpszTargetPath );
	AdjustLength( lphp->lpszStatus[ 14 ] );
	lstrcpy( lphp->lpszStatus[ 15 ], TEXT( "" ) );
	InvalidateRect( hWnd, NULL, TRUE );

	if( ! g_bHack[ 2 ] )
	{
		EnableMenuItem( GetMenu( hWnd ), IDM_WATCH, MF_BYCOMMAND | MF_ENABLED );
	}
	EnableMenuItem( GetMenu( hWnd ), IDM_UNWATCH, MF_BYCOMMAND | MF_GRAYED );

	UpdateStatus( hWnd );

	g_bHack[ 3 ] = FALSE; // just in case
	return NOT_WATCHING;
}


BOOL SetTargetPlus( const HWND hWnd, HANDLE * phChildThread, LPHACK_PARAMS lphp )
{
	TCHAR szIniPath[ MAX_PATH * 2 ];
	GetIniPath( szIniPath );
	TCHAR szDir[ MAX_PATH + 1 ] = _T( "" );
	GetPrivateProfileString(
		TEXT( "Options" ),
		TEXT( "WatchDir" ),
		TEXT( "" ),
		szDir,
		MAX_PATH,
		szIniPath
	);
	if( lstrlen( szDir ) == 0 )
	{
		if( GetShell32Version() < 5 || ! SUCCEEDED(
				SHGetFolderPath( NULL, CSIDL_PROGRAM_FILES, NULL, SHGFP_TYPE_CURRENT, szDir )
			)
		) 
		{
			GetCurrentDirectory( MAX_PATH + 1, szDir );
		}
	}
	TCHAR lpszTargetPath[ MAX_PATH * 2 ] = TEXT( "" );
	TCHAR lpszTargetExe[ MAX_PATH * 2 ] = TEXT( "" );
	OPENFILENAME ofn;
	ZeroMemory( &ofn, sizeof( OPENFILENAME ) );
	ofn.lStructSize = sizeof( OPENFILENAME );
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = TEXT( "Application (*.exe)\0*.exe\0All (*.*)\0*.*\0\0" );
	ofn.lpstrInitialDir = szDir;
	ofn.nFilterIndex = 1;
	ofn.lpstrDefExt = TEXT( "exe" );
	ofn.lpstrFile = lpszTargetPath;
	ofn.nMaxFile = MAX_PATH * 2;
	ofn.lpstrFileTitle = lpszTargetExe;
	ofn.nMaxFileTitle = MAX_PATH * 2;
#ifdef _UNICODE
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_READONLY | OFN_ENABLESIZING | OFN_DONTADDTORECENT;
#else
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_READONLY | OFN_ENABLESIZING;
#endif


	ofn.lpstrTitle = TEXT( "Limit & Watch / Watch & Limit" );

	if( ! GetOpenFileName( &ofn ) )
	{
		return FALSE;
	}
/*
	int len = lstrlen( lpszTargetExe );
	if( lstrcmpi( &lpszTargetExe[ len - 4 ], TEXT( ".exe" ) ) != 0 )
	{
		MessageBox( hWnd, lpszTargetPath, TEXT( "Not an EXE file!" ), MB_OK | MB_ICONEXCLAMATION );
		return FALSE;
	}
*/

	if( ofn.nFileOffset >= 1 )
	{
		lstrcpy( szDir, lpszTargetPath );
		szDir[ ofn.nFileOffset - 1 ] = _T('\0');
		WritePrivateProfileString(
			TEXT( "Options" ), 
			TEXT( "WatchDir" ),
			szDir,
			szIniPath
		);
	}


	
	DWORD dwTargetProcess = PathToProcess( lpszTargetPath );

	if( dwTargetProcess == GetCurrentProcessId() )
	{
		MessageBox( hWnd, TEXT( "BES cannot target itself." ), lpszTargetPath, MB_OK | MB_ICONEXCLAMATION );
		return FALSE;
	}

	//+ 1.1b11 : We alwasy do this to reset the watched...
	
	//- This is needed, to renew the info in reWatching something new
	//- if( dwTargetProcess == ( DWORD ) -1 )
	//- {
	
	lphp->lpTarget->dwProcessId = TARGET_PID_NOT_SET;
	
	//- }

#if !defined( _UNICODE )
	lstrcpy( lpszTargetPath, lpszTargetExe );
#endif

	return SetTargetPlus( hWnd, phChildThread, lphp, lpszTargetPath, lpszTargetExe );
}




BOOL SetTargetPlus( const HWND hWnd, HANDLE * phChildThread, LPHACK_PARAMS lphp,
				   LPCTSTR lpszTargetPath, LPCTSTR lpszTargetExe )
{
	if( g_bHack[ 2 ] || g_bHack[ 3 ] )
	{
		MessageBox( hWnd, TEXT("Already busy."), APP_NAME, MB_OK | MB_ICONEXCLAMATION );
		return FALSE;
	}

	if( WaitForSingleObject( hSemaphore[ 2 ], 100UL ) != WAIT_OBJECT_0 )
	{
		MessageBox( hWnd, TEXT("Sema Error (2)"), APP_NAME, MB_OK | MB_ICONEXCLAMATION );
		return FALSE;
	}

	if( WaitForSingleObject( hSemaphore[ 3 ], 100UL ) != WAIT_OBJECT_0 )
	{
		MessageBox( hWnd, TEXT("Sema Error (3)"), APP_NAME, MB_OK | MB_ICONEXCLAMATION );
		return FALSE;
	}

	g_bHack[ 2 ] = TRUE;
	g_bHack[ 3 ] = TRUE;


	if( lphp->lpTarget->dwProcessId == TARGET_PID_NOT_SET )
	{
		ZeroMemory( lphp->lpTarget, sizeof( TARGETINFO ) );
		lstrcpy( lphp->lpTarget->szExe, lpszTargetExe );
		lstrcpy( lphp->lpTarget->szPath, lpszTargetPath );
		lphp->lpTarget->dwProcessId = TARGET_PID_NOT_SET;
	}
	lphp->myHwnd = hWnd;
	lphp->iMyId = 2;



	DWORD dwChildThreadId;

	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof( SECURITY_ATTRIBUTES );
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = FALSE;

	*phChildThread = 
				CreateThread(
					&sa,
					(DWORD) 0,
					(LPTHREAD_START_ROUTINE) LimitPlus,
					(LPVOID) lphp,
					(DWORD) 0,
					&dwChildThreadId
				);

	if( *phChildThread == NULL )
	{
		ReleaseSemaphore( hSemaphore[ 2 ], 1L, (LONG *) NULL );
		ReleaseSemaphore( hSemaphore[ 3 ], 1L, (LONG *) NULL );
		g_bHack[ 2 ] = FALSE;
		g_bHack[ 3 ] = FALSE;
		MessageBox( hWnd, TEXT( "CreateThread failed." ), APP_NAME, MB_OK | MB_ICONSTOP );
		return FALSE;
	}
/*
	if( g_bRealTime )
	{
		SetThreadPriority( *phChildThread, THREAD_PRIORITY_HIGHEST );
	}
	else
	{
		SetThreadPriority( *phChildThread, THREAD_PRIORITY_NORMAL );
	}
*/
	return TRUE;
}


VOID Unwatch( HANDLE& hSema, HANDLE& hThread, BOOL * lpbWatch )
{
	*lpbWatch = FALSE;
	if( WaitForSingleObject( hSema, 1000UL ) == WAIT_OBJECT_0 )
	{
		WriteDebugLog( TEXT( "Closing hChildThread[ 3 ]" ) );
		if( hThread )
		{
			CloseHandle( hThread );
			hThread = NULL;
		}
		ReleaseSemaphore( hSema, 1L, (LPLONG) NULL );
	}
	else
	{
		WriteDebugLog( TEXT( "[!] Sema3 Error" ) );
	}
}

