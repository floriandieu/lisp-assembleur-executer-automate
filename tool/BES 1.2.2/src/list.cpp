#include "BattleEnc.h"
#include <Psapi.h> // GetModuleFileNameEx
extern HFONT hFont;
extern DWORD g_dwTargetProcessId[ 4 ];
extern BOOL g_bHack[ 4 ];
extern BOOL g_bSelChange;
extern TCHAR g_lpszEnemy[ MAX_PROCESS_CNT ][ MAX_PATH * 2 ];
extern int g_iEnemyIndex;
extern TCHAR g_lpszFriend[ MAX_PROCESS_CNT ][ MAX_PATH * 2 ];
extern int g_iFriendIndex;
extern HHOOK g_hHook;

HWND g_hListDlg = NULL;

BOOL CALLBACK WndEnumProc( HWND hwnd, LPARAM lParam )
{
	LPWININFO lpWinInfo = (LPWININFO) lParam;
	WORD idx = lpWinInfo->wIndex;
	lpWinInfo->hwnd[ idx ] = hwnd;

	lpWinInfo->dwThreadId[ idx ] = GetWindowThreadProcessId( hwnd, NULL );
	TCHAR szWindowText[ MAX_WINDOWTEXT ] = _T("");

	if( ! GetWindowText( hwnd, szWindowText, MAX_WINDOWTEXT ) )
	{
		lstrcpy( szWindowText, _T("") );
	}

	lstrcpy( lpWinInfo->szTitle[ idx ], szWindowText );
	idx++;
	lpWinInfo->wIndex = idx;
	return ( idx < MAX_PROCESS_CNT );
}


BOOL BES_ShowWindow( HWND hCurWnd, HWND hwnd, int iShow )
{
	int nCmdShow;
	if( iShow == BES_SHOW_MANUALLY )
	{
		TCHAR lpWindowText[ 1024 ];
		TCHAR msg[ 4096 ];
		GetWindowText( hwnd, lpWindowText, 1023 );
		if( lstrlen( lpWindowText ) == 0 ) lstrcpy( lpWindowText, TEXT( "n/a" ) );
		wsprintf( msg,
			TEXT( "Show this window?\r\n\r\nhWnd : %08lX\r\n\r\nWindow Text: %s" ),
			(DWORD) hwnd, lpWindowText
		);
		int iResponse =	MessageBox( hCurWnd,
			msg,
			APP_NAME, 
			MB_ICONQUESTION | MB_YESNOCANCEL | MB_DEFBUTTON2
		);
		if( iResponse == IDNO ) return TRUE;
		else if( iResponse == IDCANCEL ) return FALSE;
			
		ShowWindow( hwnd, SW_SHOWDEFAULT );
		BringWindowToTop( hwnd );
		RECT rect;
		GetWindowRect( hwnd, &rect );
		MoveWindow( hwnd, 0, 0, rect.right - rect.left, rect.bottom - rect.top, TRUE );
		return TRUE;
	}
	else if( iShow == BES_SHOW )
	{
		nCmdShow = GetCmdShow( hwnd );
		
		if( IsWindowVisible( hwnd ) || nCmdShow == BES_ERROR )
		{
			return TRUE;
		}

		ShowWindow( hwnd, nCmdShow );
		if( IsIconic( hwnd ) ) ShowWindow( hwnd, SW_RESTORE );
		BringWindowToTop( hwnd );
		SaveCmdShow( hwnd, BES_DELETE_KEY );
		SetForegroundWindow( hCurWnd );
		return TRUE;
	}
	else if( iShow == BES_HIDE )
	{
		if( ! IsWindowVisible( hwnd )
			&& ! IsIconic( hwnd ) // Just in case
		)
		{
			return TRUE;
		}
		else if( IsIconic( hwnd ) )
		{
			nCmdShow = /*SW_SHOWMINIMIZED*/ SW_SHOWMINNOACTIVE;
		}
		else if( IsZoomed( hwnd ) )
		{
			nCmdShow = SW_SHOWMAXIMIZED;
		}
		else
		{
			nCmdShow = SW_SHOW;
		}
		SaveCmdShow( hwnd, nCmdShow );
		ShowWindow( hwnd, SW_HIDE );
		return TRUE;
	}

	return TRUE;
}

VOID ShowProcessWindow( HWND hCurWnd, LPTARGETINFO lpTarget, int iShow )
{
	for( WORD i = 0; i < lpTarget->wThreadCount; i++ )
	{
		static WININFO WinInfo;
		ZeroMemory( &WinInfo, sizeof( WININFO ) );
		WinInfo.wIndex = 0;
		EnumThreadWindows( lpTarget->dwThreadId[ i ], (WNDENUMPROC) WndEnumProc, (LPARAM) &WinInfo );

		for( WORD j = 0; j < WinInfo.wIndex; j++ )
		{
			if( ! BES_ShowWindow( hCurWnd, WinInfo.hwnd[ j ], iShow ) ) return;
		}
	}

}



VOID GetProcessDetails( DWORD dwProcessId, LPTARGETINFO lpTargetInfo )
{
	lpTargetInfo->wThreadCount = (WORD) ListProcessThreads( dwProcessId, lpTargetInfo->dwThreadId );

	int iTitleImportance0 = 0;

	lstrcpy( lpTargetInfo->szText, TEXT( "" ) );

	for( WORD i = 0; i < lpTargetInfo->wThreadCount; i++ )
	{
		static WININFO WinInfo;
		ZeroMemory( &WinInfo, sizeof( WININFO ) );
		WinInfo.wIndex = (WORD) 0;
		EnumThreadWindows( lpTargetInfo->dwThreadId[ i ], (WNDENUMPROC) WndEnumProc, (LPARAM) &WinInfo );

		for( WORD j = 0; j < WinInfo.wIndex; j++ )
		{
			if( ! lstrcmpi( WinInfo.szTitle[ j ] , TEXT("Default IME") ) ) continue;

			int iTitleImportance = lstrlen( WinInfo.szTitle[ j ] );
			if( IsWindowVisible( WinInfo.hwnd[ j ] ) ) iTitleImportance *= 100;
			else if( IsIconic( WinInfo.hwnd[ j ] ) ) iTitleImportance *= 50;
			if( iTitleImportance > iTitleImportance0 )
			{
				lstrcpy( lpTargetInfo->szText, WinInfo.szTitle[ j ] );
				iTitleImportance0 = iTitleImportance;
			}
		}
	}
	if( lstrlen( lpTargetInfo->szText ) == 0 ) lstrcpy( lpTargetInfo->szText , TEXT( "<no text>" ) );
}

BOOL PathToExe( LPCTSTR lpszPath, LPTSTR lpszExe, int iBufferSize )
{
	if( lpszPath == NULL || lpszExe == NULL ) return FALSE;
	if( lstrlen( lpszPath ) >= iBufferSize ) return FALSE;

	lstrcpy( lpszExe, lpszPath );
	return PathToExe( lpszExe );
}

BOOL PathToExe( LPTSTR lpszPath )
{
	if( lpszPath == NULL ) return FALSE;
	int len = lstrlen( lpszPath );
	for( int i = len - 1; i >= 0; i-- )
	{
		if( lpszPath[ i ] == TEXT( '\\' ) )
		{
			lstrcpy( lpszPath, &lpszPath[ i + 1 ] );
			break;
		}
	}
	return TRUE;
}


BOOL PathToExeEx( LPTSTR lpszPath, int iBufferSize )
{
	if( ! PathToExe( lpszPath ) ) return FALSE;
	int len = lstrlen( lpszPath );
	if( iBufferSize <= len + 8 ) return FALSE;

	if(
		lstrlen( lpszPath ) < 15 ||
		lstrcmpi( &lpszPath[ len - 4 ], _T( ".exe" ) ) == 0
	)
	{
		return TRUE;
	}

	if
	(
		lstrcmpi( &lpszPath[ len - 3 ], _T( ".ex" ) ) == 0
	)
	{
		lstrcpy( &lpszPath[ len - 3 ], TEXT( ".exe" ) );
		return TRUE;
	}

	if
	(
		lstrcmpi( &lpszPath[ len - 2 ], _T( ".e" ) ) == 0
	)
	{
		lstrcpy( &lpszPath[ len - 2 ], TEXT( ".exe" ) );
		return TRUE;
	}

	if
	(
		lpszPath[ len - 1 ] == TEXT( '.' )
	)
	{
		lstrcat( lpszPath, TEXT( "exe" ) );
		return TRUE;
	}

/* @1.1b5 Will no longer do this
	if
	(
		lpszPath[ len - 1 ] == TEXT( ' ' )
	)
	{
		lpszPath[ len - 1 ] = TEXT( '\0' );
	}
*/


	lstrcat( lpszPath, TEXT( "~.exe" ) );

	return TRUE;
}


DWORD PathToProcess( LPCTSTR lpszTargetPath )
{
	if( lpszTargetPath == NULL || lstrlen( lpszTargetPath ) > MAX_PATH * 2 )
	{
		return (DWORD) -1;
	}

#ifdef _UNICODE
	WCHAR szPath[ MAX_PATH * 2 ] = L"";
	WCHAR szLongPath[ MAX_PATH * 2 ] = L"";
	WCHAR szTargetLongPath[ MAX_PATH * 2 ] = L"";

	DWORD dwResult = GetLongPathName( lpszTargetPath, szTargetLongPath, MAX_PATH * 2 );
	if( dwResult == 0UL || dwResult > MAX_PATH * 2 )
	{
		// Use it as it is if GetLongPathName fails
		lstrcpy( szTargetLongPath, lpszTargetPath );
	}
#else
	char lpszTargetExe[ MAX_PATH * 2 ] = "";
	PathToExe( lpszTargetPath, lpszTargetExe, MAX_PATH * 2 );
	BOOL bLongExeName = ( lstrlen( lpszTargetExe ) > 15 );
#endif

	HANDLE hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0UL );

	if( hProcessSnap == (HANDLE) -1 ) // INVALID_HANDLE_VALUE
	{
		return (DWORD) -1;
	}

	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof( PROCESSENTRY32 );
	if( ! Process32First( hProcessSnap, &pe32 ) )
	{
	    CloseHandle( hProcessSnap );
		return (DWORD) -1;
	}

	do
	{
		HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
			FALSE, pe32.th32ProcessID );
		if( hProcess != NULL ) 
		{
#ifdef _UNICODE
//+ 1.1 b5 -------------------------------------------------------------------
			GetModuleFileNameEx( hProcess, NULL, szPath, MAX_PATH * 2UL );
			DWORD dwResult = GetLongPathName( szPath, szLongPath, MAX_PATH * 2UL );
			if( dwResult == 0UL || dwResult > MAX_PATH * 2UL )
			{
				lstrcpy( szLongPath, szPath );
			}

			if( lstrcmpi( szTargetLongPath, szLongPath ) == 0 )
			{
				CloseHandle( hProcess );
				CloseHandle( hProcessSnap );
				return (DWORD) ( pe32.th32ProcessID );
			}
// ---------------------------------------------------------------------------
#else
			if( lstrcmpi( lpszTargetExe, pe32.szExeFile ) == 0 )
			{
				CloseHandle( hProcess );
				CloseHandle( hProcessSnap );
				return (DWORD) ( pe32.th32ProcessID );
			}

			// fixed @ 1.1b5
			if( bLongExeName )
			{
				TCHAR lpszThisExe[ MAX_PATH ];
				lstrcpy( lpszThisExe, lpszTargetExe );
				lpszThisExe[ 15 ] = '\0';
				if( lstrcmpi( lpszThisExe, pe32.szExeFile ) == 0 )
				{
					CloseHandle( hProcess );
					CloseHandle( hProcessSnap );
					return (DWORD) ( pe32.th32ProcessID );
				}
			}
#endif

			CloseHandle( hProcess );

		}
	} while( Process32Next( hProcessSnap, &pe32 ) );

    CloseHandle( hProcessSnap );

	return (DWORD) -1;
}



int UpdateProcessSnap( LPTARGETINFO target, UINT uMaxTargets, int iListAll )
{
	TCHAR lpszPath[ MAX_PATH * 2 ] = TEXT( "" );

	HANDLE hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0UL );
	if( hProcessSnap == (HANDLE) -1 ) return 0;
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof( PROCESSENTRY32 );
	if( ! Process32First( hProcessSnap, &pe32 ) )
	{
	    CloseHandle( hProcessSnap );
		return 0;
	}

	UINT index = 0U;
	int i;
	do
	{
		target[ index ].iIFF = IFF_UNKNOWN;

		if( pe32.th32ProcessID == 0UL ) continue;

		if( pe32.th32ProcessID == GetCurrentProcessId() && iListAll < 2 )
		{
			continue;
		}

		HANDLE hProcess = OpenProcess(
			PROCESS_QUERY_INFORMATION | PROCESS_SET_INFORMATION | PROCESS_VM_READ,
			FALSE,
			pe32.th32ProcessID
		);

		if( hProcess == NULL )
		{
			if( iListAll >= 2 ) // list everything anyway
			{
				target[ index ].iIFF = IFF_SYSTEM;

				hProcess = OpenProcess(
					PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
					FALSE,
					pe32.th32ProcessID
				);
			}
			else
			{
				continue;
			}
		}


		if( hProcess != NULL )
		{
#ifdef _UNICODE
//+ 1.1 b4 -------------------------------------------------------------------
			GetModuleFileNameEx( hProcess, NULL, lpszPath, MAX_PATH * 2UL );

// +1.1 b5
			DWORD dwResult = GetLongPathName( 
				lpszPath, target[ index ].szPath, MAX_PATH * 2UL );
			if( dwResult == 0UL || dwResult > MAX_PATH * 2UL )
			{
				// Use the Short Name if GetLongPathName fails
				lstrcpy( target[ index ].szPath, lpszPath );
			}

			PathToExe( target[ index ].szPath, target[ index ].szExe, MAX_PATH * 2 );

// ---------------------------------------------------------------------------
#endif
			CloseHandle( hProcess );
		}

		GetProcessDetails( pe32.th32ProcessID, &target[ index ] );

		target[ index ].dwProcessId = pe32.th32ProcessID;

		
		
///- 1.1b4
#ifdef _UNICODE
		if( iListAll >= 2 && target[ index ].iIFF == IFF_SYSTEM )
		{
			if( lstrlen( target[ index ].szPath ) == 0 )
			{
				lstrcpy( target[ index ].szExe , pe32.szExeFile );
				lstrcpy( target[ index ].szPath, pe32.szExeFile );
			}
#else
			lstrcpy( target[ index ].szExe , pe32.szExeFile );
			lstrcpy( lpszPath, pe32.szExeFile );

			if( PathToExeEx( lpszPath, MAX_PATH * 2 ) )
			{
				lstrcpy( target[ index ].szPath, lpszPath );
				lstrcpy( target[ index ].szExe , lpszPath );
			}
			else
			{
				lstrcpy( target[ index ].szPath, TEXT( "n/a" ) );
			}
#endif

#ifdef _UNICODE
		}
#endif




		// These are special
		if(
			lstrcmpi( target[ index ].szExe , TEXT( "EXPLORER.EXE" ) ) == 0 
			||
			pe32.th32ProcessID == GetCurrentProcessId()
		)
		{
			// Don't mess with them
			if( iListAll < 2 ) goto DO_NOT_LIST;
				
			target[ index ].iIFF = IFF_SYSTEM;
		}

		// These are absolutely foes (IFF_ABS_FOE)
		// We should make sure they are in the Enemy list.
		if( target[ index ].iIFF == IFF_UNKNOWN )
		{
			if( IsAbsFoe( pe32.szExeFile, pe32.szExeFile ) )
			{
				target[ index ].iIFF = IFF_ABS_FOE;
			}
		}

		// Detect the user-defined foes
		if( target[ index ].iIFF == IFF_UNKNOWN )
		{
			for( i = 0; i < g_iEnemyIndex; i++ )
			{
				if( lstrcmpi( target[ index ].szExe, g_lpszEnemy[ i ] ) == 0 )
				{
					target[ index ].iIFF = IFF_FOE;
					break;
				}
			}
		}

		// detect friends
		if( target[ index ].iIFF == IFF_UNKNOWN )
		{
			for( i = 0; i < g_iFriendIndex; i++ )
			{
				if( lstrcmpi( target[ index ].szExe, g_lpszFriend[ i ] ) == 0 )
				{
					target[ index ].iIFF = IFF_FRIEND;
					break;
				}
			}
		}

		index++;

		DO_NOT_LIST:
		{
			; /* do nothing */
		}
	} while( Process32Next( hProcessSnap, &pe32 ) && index < uMaxTargets );

    CloseHandle( hProcessSnap );

	return index;
}

BOOL SaveSnap( const HWND hWnd )
{
	TCHAR lpszPath[ MAX_PATH * 2 ] = TEXT( "snap.txt" );
	OPENFILENAME ofn;
	ZeroMemory( &ofn, sizeof( OPENFILENAME ) );
	ofn.lStructSize = sizeof( OPENFILENAME );
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = TEXT( "Text file (*.txt)\0*.txt\0All files (*.*)\0*.*\0\0" );
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = lpszPath;
	ofn.nMaxFile = MAX_PATH * 2;
#ifdef _UNICODE
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_ENABLESIZING | OFN_DONTADDTORECENT;
#else
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_ENABLESIZING;
#endif
	ofn.lpstrDefExt = TEXT( "txt" );
	ofn.lpstrTitle = TEXT( "Save a Snapshot of the Processes" );

	if( ! GetSaveFileName( &ofn ) ) return FALSE;

	return !! SaveSnap( lpszPath );
}

int SaveSnap( LPCTSTR lpszSavePath )
{
	TCHAR str[ 1024 ];

	HANDLE hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0UL );
	if( hProcessSnap == (HANDLE) -1 ) return 0;

	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof( PROCESSENTRY32 );

	if( ! Process32First( hProcessSnap, &pe32 ) )
	{
	    CloseHandle( hProcessSnap );
		return 0;
	}
	
	FILE * fp = _tfopen( lpszSavePath, TEXT( "wb" ) );
	if( fp == NULL ) return FALSE;

#ifdef _UNICODE
	fputc( 0xFF, fp );
	fputc( 0xFE, fp );
#endif
	_fputts( TEXT( "---------------------------\r\n" ), fp );
	_fputts( TEXT( " Snapshot of the Processes \r\n" ), fp );
	_fputts( TEXT( "---------------------------\r\n\r\n" ), fp );

	PrintFileHeader( fp );

	SYSTEMTIME st;
	GetLocalTime( &st );
	double t0 = st.wSecond + st.wMilliseconds / 1000.0;

	do
	{
		if( pe32.th32ProcessID == 0 ) continue; // this case could be interesting tho...




		wsprintf( str, TEXT( "[ %s ]\r\nProcess ID = %08lX\r\nThread Count = %d\r\n" ),
			pe32.szExeFile, pe32.th32ProcessID, pe32.cntThreads
		);
		_fputts( str, fp );

		wsprintf( str, TEXT( "Base Priority = %lu\r\n" ), pe32.pcPriClassBase );
		_fputts( str, fp );


		HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
			FALSE, pe32.th32ProcessID );
		if( hProcess != NULL ) 
		{
#ifdef _UNICODE
			WCHAR szPath[ MAX_PATH * 2 ] = L"";
			WCHAR szLongPath[ MAX_PATH * 2 ] = L"";
			GetModuleFileNameEx( hProcess, NULL, szPath, MAX_PATH * 2UL );

			DWORD dwResult = GetLongPathName( szPath, szLongPath, MAX_PATH * 2UL );
			if( dwResult == 0UL || dwResult > MAX_PATH * 2UL )
			{
				lstrcpy( szLongPath, szPath );
			}

			if( lstrlen( szLongPath ) == 0 ) lstrcpy( szLongPath, L"n/a" );

			wsprintf( str, TEXT( "Full Path = %s\r\n" ), szLongPath );
			_fputts( str, fp );
#endif

			DWORD dwPriorityClass = GetPriorityClass( hProcess );
			wsprintf( str, TEXT( "Process Priority = %lu\r\n" ), dwPriorityClass );
			_fputts( str, fp );
			CloseHandle( hProcess );
		}
		else
		{
			continue;
		}

		HANDLE hThreadSnap = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, 0UL );

		if( hThreadSnap == (HANDLE) -1 )
		{
			continue;
		}

		THREADENTRY32 te32;
		te32.dwSize = sizeof( THREADENTRY32 ); 

		if( ! Thread32First( hThreadSnap, &te32 ) )
		{
			CloseHandle( hThreadSnap );
			continue;
		}

		do 
		{ 
			if( te32.th32OwnerProcessID == pe32.th32ProcessID )
			{
				wsprintf( str, TEXT("  Thread %08lX : Priority %2ld\r\n"),
							te32.th32ThreadID, te32.tpBasePri
				);
				_fputts( str, fp );
			}
		} while( Thread32Next( hThreadSnap, &te32 ) ); 

		CloseHandle( hThreadSnap );
		_fputts( TEXT( "\r\n\r\n* * *\r\n\r\n" ), fp );

	} while( Process32Next( hProcessSnap, &pe32 ) );

    CloseHandle( hProcessSnap );

	GetLocalTime( &st );
	double t1 = st.wSecond + st.wMilliseconds / 1000.0;
	double cost = t1 - t0;
	if( cost < 0.0 ) cost += 60.0;
	char hstr[ 1024 ];
	sprintf( hstr, "Cost : %.3f seconds", cost );
	wsprintf( str, TEXT( "%hs" ), hstr );
	_fputts( str, fp );
	fclose( fp );

	return TRUE;
}

LRESULT CALLBACK xList( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
#define TIMER_ID_XLIST (UINT) 'X'

	static int iIFF = 0;
	static TARGETINFO ti[ MAX_PROCESS_CNT ];
	static int MetaIndex[ MAX_PROCESS_CNT ];
	static LPHACK_PARAMS lphp;
	static HWND hWnd;
	static iMyId;
	TCHAR msg[ 4096 ];
	TCHAR str[ 1024 ];
	static TCHAR lpszWindowText[ 1024 ];
	static ULONG ulPrevF5 = 0UL;
	static HBRUSH hBrushLB = NULL;

	switch( message )
	{
		case WM_INITDIALOG:
		{
			g_hListDlg = hDlg;

#ifdef _UNICODE
			if( IS_JAPANESEo )
			{
				MultiByteToWideChar( CP_UTF8, MB_CUTE,
					S_JPNo_1000, -1, lpszWindowText, 1023 );
				SetWindowText( hDlg, lpszWindowText );
			}
			else if( IS_JAPANESE )
			{
				MultiByteToWideChar( CP_UTF8, MB_CUTE,
					S_JPN_1000, -1, lpszWindowText, 1023 );
				SetWindowText( hDlg, lpszWindowText );
			}
			else if( IS_FINNISH )
			{
				MultiByteToWideChar( CP_UTF8, MB_CUTE,
					S_FIN_1000, -1, lpszWindowText, 1023 );
				SetWindowText( hDlg, lpszWindowText );
			}
			else if( IS_SPANISH )
			{
				MultiByteToWideChar( CP_UTF8, MB_CUTE,
					S_SPA_1000, -1, lpszWindowText, 1023 );
				SetWindowText( hDlg, lpszWindowText );
			}
			else if( IS_CHINESE_T )
			{
				MultiByteToWideChar( CP_UTF8, MB_CUTE,
					S_CHI_1000T, -1, lpszWindowText, 1023 );
				SetWindowText( hDlg, lpszWindowText );
			}
			else if( IS_CHINESE_S )
			{
				MultiByteToWideChar( CP_UTF8, MB_CUTE,
					S_CHI_1000S, -1, lpszWindowText, 1023 );
				SetWindowText( hDlg, lpszWindowText );
			}
			else if( IS_FRENCH )
			{
				MultiByteToWideChar( CP_UTF8, MB_CUTE,
					S_FRE_1000, -1, lpszWindowText, 1023 );
				SetWindowText( hDlg, lpszWindowText );
			}
			else
#endif
			{
				lstrcpy( lpszWindowText, _T( "Which process would you like to limit?" ) );
				SetWindowText( hDlg, lpszWindowText );
			}

			lphp = ( LPHACK_PARAMS ) lParam;
			hWnd = lphp -> myHwnd;
			iMyId = lphp->iMyId;
			if( iMyId < 0 || iMyId > 2 )
			{
				EndDialog( hDlg, 0 );
				break;
			}



			if( g_bHack[ 2 ] || g_bHack[ 3 ] )
			{
				EnableWindow( GetDlgItem( hDlg, IDC_WATCH ), FALSE );
			}
			ZeroMemory( &ti[ 0 ], sizeof( TARGETINFO ) * MAX_PROCESS_CNT );
			ZeroMemory( &MetaIndex[ 0 ], sizeof( int ) * MAX_PROCESS_CNT );

			SendDlgItemMessage( hDlg, IDC_TARGET_LIST, WM_SETFONT, ( WPARAM ) hFont, 0L );
			SendDlgItemMessage( hDlg, IDC_EDIT_INFO, WM_SETFONT, ( WPARAM ) hFont, 0L );
			SendDlgItemMessage( hDlg, IDC_EDIT_INFO, EM_SETMARGINS,
				(WPARAM) ( EC_LEFTMARGIN | EC_RIGHTMARGIN ),
				MAKELPARAM( 5 , 5 )
			);

			SendDlgItemMessage( hDlg, IDC_TARGET_LIST, LB_SETHORIZONTALEXTENT, 600U, 0L );
			SendMessage( hDlg, WM_USER_REFRESH, 0U, 0L );

			// Anti-Ukagaka
			SetTimer( hDlg, TIMER_ID_XLIST, 500, (TIMERPROC) NULL );
			break;
		}


		case WM_USER_REFRESH:
		{
			BOOL bBusySSTP = FALSE;
			
			SendDlgItemMessage( hDlg, IDC_TARGET_LIST, LB_RESETCONTENT, 0U, 0L );

			TCHAR szTitle[ MAX_WINDOWTEXT ];
			int iProcessCount = UpdateProcessSnap(
				ti,
				MAX_PROCESS_CNT,
				(int) lParam
			);

			int index = 0, m = 0;
			DWORD dwTargetId = ( DWORD ) wParam;

			UINT uCurSel = 0U;
			UINT uItem = 0U;

			for( index = 0; index < iProcessCount; index++ )
			{
				if( ti[ index ].iIFF == IFF_ABS_FOE )
				{
					MetaIndex[ m++ ] = index;
					lstrcpy( szTitle, ti[ index ].szText );
					if( lstrlen( szTitle ) == 0 )
					{
						lstrcpy( szTitle, ti[ index ].szPath );
					}

					if( lstrlen( szTitle ) > 48 )
					{
						szTitle[ 45 ] = TEXT( '\0' );
						lstrcat( szTitle, TEXT( "..." ) );
					}
					wsprintf( str, TEXT( "[++] %s <%s>" ), ti[ index ].szExe, szTitle );
					SendDlgItemMessage( hDlg, IDC_TARGET_LIST, LB_INSERTSTRING, (WPARAM) -1, (LPARAM) str );
					if( ti[ index ].dwProcessId == dwTargetId ) uCurSel = uItem;
					uItem++;

					if( lstrcmpi( TEXT( "aviutl.exe" ), ti[ index ].szExe ) == 0 )
					{
						bBusySSTP = SSTP_Aviutl( hDlg, ti[ index ].szText );
					}

				}
			}

			for( index = 0; index < iProcessCount; index++ )
			{
				if( ti[ index ].iIFF == IFF_FOE )
				{
					MetaIndex[ m++ ] = index;
					lstrcpy( szTitle, ti[ index ].szText );
					if( lstrlen( szTitle ) == 0 )
					{
						lstrcpy( szTitle, ti[ index ].szPath );
					}

					if( lstrlen( szTitle ) > 48 )
					{
						szTitle[ 45 ] = TEXT( '\0' );
						lstrcat( szTitle, TEXT( "..." ) );
					}
					wsprintf( str, TEXT( "[+] %s <%s>" ), ti[ index ].szExe, szTitle );
					SendDlgItemMessage( hDlg, IDC_TARGET_LIST, LB_INSERTSTRING, (WPARAM) -1, (LPARAM) str );
					if( ti[ index ].dwProcessId == dwTargetId ) uCurSel = uItem;
					uItem++;
				}
			}

			for( index = 0; index < iProcessCount; index++ )
			{
				if( ti[ index ].iIFF == IFF_UNKNOWN )
				{
					MetaIndex[ m++ ] = index;
					lstrcpy( szTitle, ti[ index ].szText );
					if( lstrlen( szTitle ) == 0 )
					{
						lstrcpy( szTitle, ti[ index ].szPath );
					}
							
					if( lstrlen( szTitle ) > 48 )
					{
						szTitle[ 45 ] = TEXT( '\0' );
						lstrcat( szTitle, TEXT( "..." ) );
					}
					wsprintf( str, TEXT( "%s <%s>" ), ti[ index ].szExe, szTitle );
					SendDlgItemMessage( hDlg, IDC_TARGET_LIST, LB_INSERTSTRING, (WPARAM) -1, (LPARAM) str );
					if( ti[ index ].dwProcessId == dwTargetId ) uCurSel = uItem;
					uItem++;
				}
			}
			for( index = 0; index < iProcessCount; index++ )
			{
				if( ti[ index ].iIFF == IFF_FRIEND )
				{
					MetaIndex[ m++ ] = index;
					lstrcpy( szTitle, ti[ index ].szText );

					if( lstrlen( szTitle ) == 0 )
					{
						lstrcpy( szTitle, ti[ index ].szPath );
					}

					if( lstrlen( szTitle ) > 48 )
					{
						szTitle[ 45 ] = TEXT( '\0' );
						lstrcat( szTitle, TEXT( "..." ) );
					}
					wsprintf( str, TEXT( "[-] %s <%s>" ), ti[ index ].szExe, szTitle );
					SendDlgItemMessage( hDlg, IDC_TARGET_LIST, LB_INSERTSTRING, (WPARAM) -1, (LPARAM) str );
					if( ti[ index ].dwProcessId == dwTargetId ) uCurSel = uItem;
					uItem++;
				}
			}

			for( index = 0; index < iProcessCount; index++ )
			{
				if( ti[ index ].iIFF == IFF_SYSTEM )
				{
					MetaIndex[ m++ ] = index;
					lstrcpy( szTitle, ti[ index ].szText );
					if( lstrlen( szTitle ) == 0 )
					{
						lstrcpy( szTitle, ti[ index ].szPath );
					}
							
					if( lstrlen( szTitle ) > 48 )
					{
						szTitle[ 45 ] = TEXT( '\0' );
						lstrcat( szTitle, TEXT( "..." ) );
					}
					wsprintf( str, TEXT( "[--] %s <%s>" ), ti[ index ].szExe, szTitle );
					SendDlgItemMessage( hDlg, IDC_TARGET_LIST, LB_INSERTSTRING, (WPARAM) -1, (LPARAM) str );
					if( ti[ index ].dwProcessId == dwTargetId ) uCurSel = uItem;
					uItem++;
				}
			}

			if( iProcessCount >= 1 )
			{
				SendDlgItemMessage( hDlg, IDC_TARGET_LIST, LB_SETCURSEL, uCurSel, 0L );
				SendMessage( hDlg, WM_COMMAND,
					MAKEWPARAM( IDC_TARGET_LIST, LBN_SELCHANGE ), 0L );
			}

			SetFocus( GetDlgItem( hDlg, IDC_TARGET_LIST ) );

			if
			( 
				! bBusySSTP
				&&
				LANG_JAPANESE == PRIMARYLANGID( GetSystemDefaultLangID() )
			)
			{
				DirectSSTP( hDlg, S_TARGET_SJIS, "" );
			}
			break;
		}
/*		
		case WM_VKEYTOITEM:
		{
			if( LOWORD( wParam ) == VK_F5 )
			{
//				if( GetTickCount() - ulPrevF5 > 1000 )
				{
					SendMessage( hDlg, WM_COMMAND, (WPARAM) IDC_REFRESH, 0L );
//					ulPrevF5 = GetTickCount();
				}
				return (BOOL) -2;
			}
			return (BOOL) -1;
		}
*/
		case WM_COMMAND:
		{

#define THIS_BUTTON_IS_DISABLED	( ! IsWindowEnabled( GetDlgItem( hDlg, LOWORD( wParam ) ) ) )

			switch( LOWORD( wParam ) )
			{
				case IDOK:
				{
					if( THIS_BUTTON_IS_DISABLED ) break;


					int iSelected = SendDlgItemMessage( hDlg, IDC_TARGET_LIST, LB_GETCURSEL, 0U, 0L );

					if( LB_ERR == iSelected )
					{
						break;
					}

					iSelected = MetaIndex[ iSelected ];


					if( ti[ iSelected ].dwProcessId == GetCurrentProcessId() )
					{
						MessageBox( hDlg,
							TEXT( "BES can't target itself." ),
							APP_NAME, MB_OK | MB_ICONEXCLAMATION );
						return 1L;
					}

					if( lParam != (LONG) XLIST_WATCH_THIS )
					{
						for( int i = 0; i < 3; i++ )
						{
							if(
								ti[ iSelected ].dwProcessId == g_dwTargetProcessId[ i ]
								&&
								g_bHack[ i ] 
							)
							{
								wsprintf( str, TEXT( "%s [ 0x%08lX ] is already targeted as #%d." ),
									ti[ iSelected ].szExe, ti[ iSelected ].dwProcessId, ( i + 1 )
								);
								MessageBox( hDlg, str, APP_NAME, MB_OK | MB_ICONEXCLAMATION );
								return 1L;
							}
						}
					}
					
					if( lParam == (LONG) XLIST_WATCH_THIS )
					{
						ti[ iSelected ].bWatch = TRUE;
					}

					int iResponse = DialogBoxParam( 
						(HINSTANCE) GetWindowLongPtr( hDlg, GWLP_HINSTANCE ),
						(LPCTSTR) IDD_QUESTION, hDlg, (DLGPROC) Question,
						(LPARAM) &ti[ iSelected ] );

					if( iResponse == IDYES ) 
					{
						lphp->lpTarget->dwProcessId = ti[ iSelected ].dwProcessId;
						for( int s = 0; s < (int) ti[ iSelected ].wThreadCount; s++ )
						{
							lphp->lpTarget->dwThreadId[ s ] = ti[ iSelected ].dwThreadId[ s ];
						}
						lphp->lpTarget->iIFF = ti[ iSelected ].iIFF;
						lstrcpy( lphp->lpTarget->szExe, ti[ iSelected ].szExe );
						lstrcpy( lphp->lpTarget->szPath, ti[ iSelected ].szPath );
						lstrcpy( lphp->lpTarget->szText, ti[ iSelected ].szText );
						lphp->lpTarget->wThreadCount = ti[ iSelected ].wThreadCount;

						if( ti[ iSelected ].iIFF == IFF_UNKNOWN || ti[ iSelected ].iIFF == IFF_FRIEND )
						{
							lstrcpy( g_lpszEnemy[ g_iEnemyIndex++ ] , ti[ iSelected ].szExe );

							for( int i = 0; i < g_iFriendIndex; i++ )
							{
								// formerly considered as Friend
								if( lstrcmpi( ti[ iSelected ].szExe, g_lpszFriend[ i ] ) == 0 )
								{
									if( g_iFriendIndex >= 2 )
									{
										lstrcpy( g_lpszFriend[ i ], g_lpszFriend[ g_iFriendIndex - 1 ] );
										lstrcpy( g_lpszFriend[ g_iFriendIndex - 1 ], TEXT( "" ) );
									}
									g_iFriendIndex--;
									break;
								}
							}
						}

						if( lParam == (LONG) XLIST_WATCH_THIS )
						{
							EndDialog( hDlg, XLIST_WATCH_THIS );
						}
						else if( ti[ iSelected ].dwProcessId == g_dwTargetProcessId[ 0 ] )
						{
							EndDialog( hDlg, XLIST_RESTART_0 );
						}
						else if( ti[ iSelected ].dwProcessId == g_dwTargetProcessId[ 1 ] )
						{
							EndDialog( hDlg, XLIST_RESTART_1 );
						}
						else if( ti[ iSelected ].dwProcessId == g_dwTargetProcessId[ 2 ] )
						{
							EndDialog( hDlg, XLIST_RESTART_2 );
						}
						else
						{
							EndDialog( hDlg, XLIST_NEW_TARGET );
						}
						break;
					}
					else if( iResponse == IDNO )
					{
						break;
					}
					else
					{
						EndDialog( hDlg, XLIST_CANCELED );
						break;
					}

					break;
				}
				case IDCANCEL:
				{
					EndDialog( hDlg, XLIST_CANCELED );
					break;
				}

				case IDC_WATCH:
				{
					if( THIS_BUTTON_IS_DISABLED ) break;
					SendMessage( hDlg, WM_COMMAND, IDOK, (LONG) XLIST_WATCH_THIS );
					break;
				}

				case IDC_LISTALL_SYS:
				{
					if( THIS_BUTTON_IS_DISABLED ) break;
					int iSelected = SendDlgItemMessage( hDlg, IDC_TARGET_LIST, LB_GETCURSEL, 0U, 0L );
					if( LB_ERR == iSelected ) break;
					iSelected = MetaIndex[ iSelected ];
					SendMessage( hDlg, WM_USER_REFRESH, (WPARAM) ti[ iSelected ].dwProcessId, 3L );
					break;
				}

				case IDC_RELOAD:
				{
					if( THIS_BUTTON_IS_DISABLED ) break;

					HWND hBtn = GetDlgItem( hDlg, IDC_RELOAD );
					if( GetTickCount() - ulPrevF5 > 1000UL )
					{
						EnableWindow( hBtn, FALSE );
						ulPrevF5 = GetTickCount();
					}
					else
					{
						break;
					}

					int iSelected = SendDlgItemMessage( hDlg, IDC_TARGET_LIST, LB_GETCURSEL, 0U, 0L );
					if( LB_ERR == iSelected ) break;
					iSelected = MetaIndex[ iSelected ];
					SendMessage( hDlg, WM_USER_REFRESH, (WPARAM) ti[ iSelected ].dwProcessId, 0L );
					
					EnableWindow( hBtn, TRUE );
//					SetFocus( hActiveCtrl );
					break;
				}

				case IDC_FOE:
				{
					if( THIS_BUTTON_IS_DISABLED ) break;

					int iSelected = SendDlgItemMessage( hDlg, IDC_TARGET_LIST, LB_GETCURSEL, 0U, 0L );
					if( LB_ERR == iSelected ) break;
					iSelected = MetaIndex[ iSelected ];
					lstrcpy( g_lpszEnemy[ g_iEnemyIndex++ ] , ti[ iSelected ].szExe );

					for( int i = 0; i < g_iFriendIndex; i++ )
					{
						// formerly considered as Friend
						if( lstrcmpi( ti[ iSelected ].szExe, g_lpszFriend[ i ] ) == 0 )
						{
							if( g_iFriendIndex >= 2 )
							{
								lstrcpy( g_lpszFriend[ i ], g_lpszFriend[ g_iFriendIndex - 1 ] );
								lstrcpy( g_lpszFriend[ g_iFriendIndex - 1 ], TEXT( "" ) );
							}
							g_iFriendIndex--;
							break;
						}
					}

					SendMessage( hDlg, WM_USER_REFRESH, (WPARAM) ti[ iSelected ].dwProcessId, 0L );
					break;
				}

				case IDC_FRIEND:
				{
					if( THIS_BUTTON_IS_DISABLED ) break;

					int iSelected = SendDlgItemMessage( hDlg, IDC_TARGET_LIST, LB_GETCURSEL, 0U, 0L );
					if( LB_ERR == iSelected ) break;
					iSelected = MetaIndex[ iSelected ];
					lstrcpy( g_lpszFriend[ g_iFriendIndex++ ] , ti[ iSelected ].szExe );

					for( int i = 0; i < g_iEnemyIndex; i++ )
					{
						// formerly considered as Foe
						if( lstrcmpi( ti[ iSelected ].szExe, g_lpszEnemy[ i ] ) == 0 )
						{
							if( g_iEnemyIndex >= 2 )
							{
								lstrcpy( g_lpszEnemy[ i ], g_lpszEnemy[ g_iEnemyIndex - 1 ] );
								lstrcpy( g_lpszEnemy[ g_iEnemyIndex - 1 ], TEXT( "" ) );
							}
							g_iEnemyIndex--;
							break;
						}
					}

					SendMessage( hDlg, WM_USER_REFRESH, (WPARAM) ti[ iSelected ].dwProcessId, 0L );
					break;
				}

				case IDC_RESET_IFF:
				{
					if( THIS_BUTTON_IS_DISABLED ) break;

					int iSelected = SendDlgItemMessage( hDlg, IDC_TARGET_LIST, LB_GETCURSEL, 0U, 0L );
					if( LB_ERR == iSelected ) break;
					iSelected = MetaIndex[ iSelected ];

					if( ti[ iSelected ].iIFF == IFF_FRIEND )
					{
						ti[ iSelected ].iIFF = IFF_UNKNOWN;
						for( int i = 0; i < g_iFriendIndex; i++ )
						{
							if( lstrcmpi( ti[ iSelected ].szExe, g_lpszFriend[ i ] ) == 0 )
							{
								if( g_iFriendIndex >= 2 )
								{
									lstrcpy( g_lpszFriend[ i ], g_lpszFriend[ g_iFriendIndex - 1 ] );
									lstrcpy( g_lpszFriend[ g_iFriendIndex - 1 ], TEXT( "" ) );
								}
								g_iFriendIndex--;
								break;
							}
						}
					}
					else if( ti[ iSelected ].iIFF == IFF_FOE )
					{
						ti[ iSelected ].iIFF = IFF_UNKNOWN;
						for( int i = 0; i < g_iEnemyIndex; i++ )
						{
							// formerly considered as Foe
							if( lstrcmpi( ti[ iSelected ].szExe, g_lpszEnemy[ i ] ) == 0 )
							{
								if( g_iEnemyIndex >= 2 )
								{
									lstrcpy( g_lpszEnemy[ i ], g_lpszEnemy[ g_iEnemyIndex - 1 ] );
									lstrcpy( g_lpszEnemy[ g_iEnemyIndex - 1 ], TEXT( "" ) );
								}
								g_iEnemyIndex--;
								break;
							}
						}
					}
					else break;

					SendMessage( hDlg, WM_USER_REFRESH, (WPARAM) ti[ iSelected ].dwProcessId, 0L );
					break;
				}

				case IDC_UNFREEZE:
				{
					if( THIS_BUTTON_IS_DISABLED ) break;

					int iSelected = SendDlgItemMessage( hDlg, IDC_TARGET_LIST, LB_GETCURSEL, 0U, 0L );
					if( LB_ERR == iSelected ) break;
					iSelected = MetaIndex[ iSelected ];
					if( ti[ iSelected ].dwProcessId == GetCurrentProcessId() ) break;
					wsprintf( msg,
						TEXT( "Try to unfreeze %s (%08lX)?\r\n\r\nUse this command ONLY IN EMERGENCY when" )
						TEXT( " the target is frozen by BES.exe (i.e. process has been suspended and won't resume).\r\n\r\n" )
						TEXT( "Such a situation should be quite exceptional, but might happen if BES crashes" )
						TEXT( " while being active." ),
						ti[ iSelected ].szPath, ti[ iSelected ].dwProcessId );
					if( IDCANCEL == MessageBox( hDlg, msg, APP_NAME, MB_OKCANCEL | MB_ICONQUESTION | MB_DEFBUTTON2 ) ) break;

					*( lphp -> lpTarget ) = ti[ iSelected ];
					EndDialog( hDlg, XLIST_UNFREEZE );
					break;
				}

				case IDC_HIDE:
				{
					if( THIS_BUTTON_IS_DISABLED ) break;

					LONG lSelected = SendDlgItemMessage( hDlg, IDC_TARGET_LIST, LB_GETCURSEL, 0U, 0L );
					if( LB_ERR == lSelected ) break;
					int iSelected = MetaIndex[ lSelected ];
					ShowProcessWindow( hDlg, &ti[ iSelected ], BES_HIDE );
					if( g_bSelChange)
					{
						LONG lCount = SendDlgItemMessage( hDlg, IDC_TARGET_LIST, LB_GETCOUNT, 0U, 0L );
						if( LB_ERR == lCount || lSelected == lCount ) break;
						SendDlgItemMessage( hDlg, IDC_TARGET_LIST, LB_SETCURSEL, (WPARAM) ( lSelected + 1L ), 0L );
						SendMessage( hDlg, WM_COMMAND, MAKEWPARAM( IDC_TARGET_LIST, LBN_SELCHANGE ), 0L );
					}
					break;
				}
				case IDC_SHOW:
				{
					if( THIS_BUTTON_IS_DISABLED ) break;

					LONG lSelected = SendDlgItemMessage( hDlg, IDC_TARGET_LIST, LB_GETCURSEL, 0U, 0L );
					if( LB_ERR == lSelected ) break;
					int iSelected = MetaIndex[ lSelected ];
					ShowProcessWindow( hDlg, &ti[ iSelected ], BES_SHOW );
					if( g_bSelChange)
					{
						LONG lCount = SendDlgItemMessage( hDlg, IDC_TARGET_LIST, LB_GETCOUNT, 0U, 0L );
						if( LB_ERR == lCount || lSelected == lCount ) break;
						SendDlgItemMessage( hDlg, IDC_TARGET_LIST, LB_SETCURSEL, (WPARAM) ( lSelected + 1L ), 0L );
						SendMessage( hDlg, WM_COMMAND, MAKEWPARAM( IDC_TARGET_LIST, LBN_SELCHANGE ), 0L );
					}
					break;
				}
				case IDC_SHOW_MANUALLY:
				{
					if( THIS_BUTTON_IS_DISABLED ) break;

					int iSelected = SendDlgItemMessage( hDlg, IDC_TARGET_LIST, LB_GETCURSEL, 0U, 0L );
					if( LB_ERR == iSelected ) break;
					iSelected = MetaIndex[ iSelected ];
					ShowProcessWindow( hDlg, &ti[ iSelected ], BES_SHOW_MANUALLY );
					break;
				}

				case IDC_TARGET_LIST:
				{
					if( HIWORD( wParam ) == LBN_SELCHANGE )
					{
						int iSelected = (int) SendDlgItemMessage( hDlg, IDC_TARGET_LIST, LB_GETCURSEL, 0U, 0L );
						if( LB_ERR == iSelected ) break;
						iSelected = MetaIndex[ iSelected ];
						GetProcessInfoMsg( &ti[ iSelected ], msg, 4096 );
						SetDlgItemText( hDlg, IDC_EDIT_INFO, msg );

						EnableWindow( GetDlgItem( hDlg, IDOK ), TRUE );
						EnableWindow( GetDlgItem( hDlg, IDC_WATCH ), TRUE );
						EnableWindow( GetDlgItem( hDlg, IDC_FRIEND ), TRUE );
						EnableWindow( GetDlgItem( hDlg, IDC_FOE), TRUE );
						EnableWindow( GetDlgItem( hDlg, IDC_UNFREEZE ), TRUE );
						EnableWindow( GetDlgItem( hDlg, IDC_RESET_IFF ), FALSE );
						EnableWindow( GetDlgItem( hDlg, IDC_SHOW ), TRUE );
						EnableWindow( GetDlgItem( hDlg, IDC_SHOW_MANUALLY ), TRUE );
						EnableWindow( GetDlgItem( hDlg, IDC_HIDE ), TRUE );

						for( int i = 0; i < 3; i++ )
						{
							if(	ti[ iSelected ].dwProcessId == g_dwTargetProcessId[ i ]
								&& g_bHack[ i ] )
							{
								EnableWindow( GetDlgItem( hDlg, IDOK ), FALSE );
								break;
							}
						}

						if( g_bHack[ 3 ] )
						{
							EnableWindow( GetDlgItem( hDlg, IDC_WATCH ), FALSE );
						}


						if( ti[ iSelected ].dwProcessId == GetCurrentProcessId() )
						{
							EnableWindow( GetDlgItem( hDlg, IDOK ), FALSE );
							EnableWindow( GetDlgItem( hDlg, IDC_WATCH ), FALSE );
							EnableWindow( GetDlgItem( hDlg, IDC_FRIEND ), FALSE );
							EnableWindow( GetDlgItem( hDlg, IDC_FOE), FALSE );
							EnableWindow( GetDlgItem( hDlg, IDC_UNFREEZE ), FALSE );
							EnableWindow( GetDlgItem( hDlg, IDC_SHOW ), FALSE );
							EnableWindow( GetDlgItem( hDlg, IDC_SHOW_MANUALLY ), FALSE );
							EnableWindow( GetDlgItem( hDlg, IDC_HIDE ), FALSE );
						}
						else if( ti[ iSelected ].iIFF == IFF_FRIEND )
						{
							EnableWindow( GetDlgItem( hDlg, IDC_FRIEND ), FALSE );
							EnableWindow( GetDlgItem( hDlg, IDC_RESET_IFF ), TRUE );
						}
						else if( ti[ iSelected ].iIFF == IFF_FOE )
						{
							EnableWindow( GetDlgItem( hDlg, IDC_FOE), FALSE );
							EnableWindow( GetDlgItem( hDlg, IDC_RESET_IFF ), TRUE );
						}
						else if( ti[ iSelected ].iIFF == IFF_ABS_FOE || ti[ iSelected ].iIFF == IFF_SYSTEM )
						{
							EnableWindow( GetDlgItem( hDlg, IDC_FRIEND ), FALSE );
							EnableWindow( GetDlgItem( hDlg, IDC_FOE), FALSE );
						}
						
						iIFF = ti[ iSelected ].iIFF;

						RECT rect = { 10L, 290L, 150L, 320L };
						InvalidateRect( hDlg, &rect, TRUE );
					}
					else if( HIWORD( wParam ) == LBN_DBLCLK )
					{
						int iSelected = SendDlgItemMessage( hDlg, IDC_TARGET_LIST, LB_GETCURSEL, 0U, 0L );
						if( LB_ERR == iSelected ) break;
						iSelected = MetaIndex[ iSelected ];
						if( ti[ iSelected ].dwProcessId == GetCurrentProcessId() ) break;

						SendMessage( hDlg, WM_COMMAND, (WPARAM) IDOK, 0L );
					}
					break;
				}
			}

			break;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint( hDlg, &ps );

			SelectObject( hdc, hFont );
			TCHAR lpStr[ 100 ];
			lstrcpy( lpStr, ( iIFF < 0 )? TEXT("Friend") : ( iIFF > 0 )? TEXT("Foe") : TEXT("Unknown") );
			SetBkMode( hdc, TRANSPARENT );
			SetTextColor( hdc, RGB( 0, 0, 0xff ) );
			TextOut( hdc, 50, 293, lpStr, lstrlen( lpStr ) );

			HPEN hPen = CreatePen( PS_SOLID, 1, RGB( 0x80, 0x80, 0x80 ) );


			HPEN hOldPen = (HPEN) SelectObject( hdc, hPen );
			HBRUSH hBrushIFF = CreateSolidBrush(
				iIFF > 0 ? RGB( 0xff, 0x00, 0x00 ) :
				iIFF < 0 ? RGB( 0x00, 0xff, 0x66 ) :
				           RGB( 0xff, 0xff, 0x00 )
			);
			HBRUSH hOldBrush = (HBRUSH) SelectObject( hdc, hBrushIFF );
			Ellipse( hdc, 20, 292, 40, 312 );

			HBRUSH hBrush = CreateSolidBrush( RGB( 198, 214, 255 ) );
			SelectObject( hdc, hBrush );
			Rectangle( hdc, 518, 257, 618, 371 );
			
			SelectObject( hdc, hOldPen );
			DeletePen( hPen );
			SelectObject( hdc, hOldBrush );
			DeleteBrush( hBrush );
			DeleteBrush( hBrushIFF );
			EndPaint( hDlg, &ps );
			break;
		}		

		case WM_CTLCOLORLISTBOX:
		{
			if( (HWND) lParam == GetDlgItem( hDlg, IDC_TARGET_LIST ) )
			{
				SetBkMode( (HDC) wParam, TRANSPARENT );
				SetTextColor( (HDC) wParam, RGB( 0x00, 0x00, 0x99 ) );
				if( hBrushLB == NULL )
				{
					hBrushLB = CreateSolidBrush( RGB( 0xff, 0xff, 0xcc ) );
				}
				return (BOOL) hBrushLB;
			}
			return FALSE;
			break;
		}

		case WM_TIMER:
		{
			SetWindowText( hDlg, lpszWindowText );
			break;
		}

		case WM_DESTROY:
		{
			if( hBrushLB != NULL )
			{
				DeleteBrush( hBrushLB );
				hBrushLB = NULL;
			}

			KillTimer( hDlg, TIMER_ID_XLIST );
			g_hListDlg = NULL;
			
			break;
		}
		default:
			return 0L;
	}
    return 1L;
}
#undef TIMER_ID_XLIST


BOOL GetProcessInfoMsg( LPTARGETINFO lpTarget, LPTSTR msg, int iBufferSize )
{
	if( msg == NULL || lpTarget->dwProcessId == 0UL || iBufferSize < 4096 ) return FALSE;

	lstrcpy( msg, TEXT( "" ) );

	TCHAR str[ 1024 ];
#ifdef _UNICODE
	if( IS_JAPANESE )
	{
		MultiByteToWideChar( CP_UTF8, MB_CUTE, S_FORMAT1_JPN, -1, str, 1023 );
	}
	else
	{
		lstrcpy( str, TEXT( S_FORMAT1_ASC ) );
	}
#else
	lstrcpy( str, S_FORMAT1_ASC );
#endif

	wsprintf( msg, str,
		lpTarget->szExe,
		lpTarget->szPath,
		lstrlen( lpTarget->szText ) ? lpTarget->szText : TEXT( "n/a" ),
		HIWORD( lpTarget->dwProcessId ), LOWORD( lpTarget->dwProcessId ),
		lpTarget->wThreadCount
	);
#ifdef _UNICODE
	if( ! IS_JAPANESE )
	{
		// "including %u thread(s)" <-- this (s)
		if( lpTarget->wThreadCount > 1 ) lstrcat( msg, TEXT( "s" ) );
	}
#endif
	lstrcat( msg, TEXT( ":\r\n" ) );

	for( int i = 0; i < lpTarget->wThreadCount; i++ )
	{
		TCHAR tmpstr[ 100 ];
		DWORD dwThreadId = lpTarget->dwThreadId[ i ];
		wsprintf( tmpstr,
			TEXT( "Thread #%03d: ID 0x%04X %04X\r\n" ),
			i + 1,
			HIWORD( dwThreadId ),
			LOWORD( dwThreadId )
		);
		lstrcat( msg, tmpstr );
	}

	lstrcat( msg, TEXT( "\r\n" ) );

	return TRUE;
}



int ListProcessThreads( DWORD dwOwnerPID, DWORD * dwThreadIdTable ) 
{
	int iThreadCount = 0; 
	HANDLE hThreadSnap = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, 0UL );
	if( hThreadSnap == ( HANDLE ) -1 ) return 0;

	THREADENTRY32 te32;
	te32.dwSize = sizeof( THREADENTRY32 ); 

	if( ! Thread32First( hThreadSnap, &te32 ) )
	{
		CloseHandle( hThreadSnap );
		return 0;
	}

	do 
	{ 
		if( te32.th32OwnerProcessID == dwOwnerPID )
		{
			dwThreadIdTable[ iThreadCount++ ] = te32.th32ThreadID;
		}
	} while( Thread32Next( hThreadSnap, &te32 ) && iThreadCount < MAX_THREAD_CNT ); 

	CloseHandle( hThreadSnap );
	return iThreadCount;
}


LRESULT CALLBACK Question( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
#define TIMER_ID_Q (UINT) 'Q'

	static LPTARGETINFO lpTarget;
	static TCHAR lpszWindowText[ 1024 ];
	switch ( message )
	{
		case WM_INITDIALOG:
		{
			TCHAR msg1[ 1024 ];
			TCHAR msg2[ 4096 ];
			TCHAR format[ 1024 ];

			lpTarget = (LPTARGETINFO) lParam;

#ifdef _UNICODE
			if( IS_JAPANESE )
			{
				
				MultiByteToWideChar( CP_UTF8, MB_CUTE,
					IS_JAPANESEo ? S_JPNo_1001 : S_JPN_1001,
					-1, lpszWindowText, 1023 );
				SetWindowText( hDlg, lpszWindowText );
				MultiByteToWideChar( CP_UTF8, MB_CUTE,
					lpTarget->bWatch? S_QUESTION2_JPN : S_QUESTION1_JPN,
					-1, msg1, 1023
				);
				MultiByteToWideChar( CP_UTF8, MB_CUTE,
					S_FORMAT2_JPN, -1, format, 1023 );
			}
			else if( IS_FINNISH )
			{
				
				MultiByteToWideChar( CP_UTF8, MB_CUTE,
					S_FIN_1001, -1, lpszWindowText, 1023 );
				SetWindowText( hDlg, lpszWindowText );
				MultiByteToWideChar( CP_UTF8, MB_CUTE,
					lpTarget->bWatch? S_QUESTION2_FIN : S_QUESTION1_FIN,
					-1, msg1, 1023
				);
				lstrcpy( format, TEXT( S_FORMAT2_ASC ) );
			}
			else if( IS_SPANISH )
			{
				
				MultiByteToWideChar( CP_UTF8, MB_CUTE,
					S_SPA_1001, -1, lpszWindowText, 1023 );
				SetWindowText( hDlg, lpszWindowText );
				MultiByteToWideChar( CP_UTF8, MB_CUTE,
					lpTarget->bWatch? S_QUESTION2_SPA : S_QUESTION1_SPA,
					-1, msg1, 1023
				);
				lstrcpy( format, TEXT( S_FORMAT2_ASC ) );
			}
			else if( IS_CHINESE_T )
			{
				
				MultiByteToWideChar( CP_UTF8, MB_CUTE,
					S_CHI_1001T, -1, lpszWindowText, 1023 );
				SetWindowText( hDlg, lpszWindowText );
				MultiByteToWideChar( CP_UTF8, MB_CUTE,
					lpTarget->bWatch? S_QUESTION2_CHIT : S_QUESTION1_CHIT,
					-1, msg1, 1023
				);
				lstrcpy( format, TEXT( S_FORMAT2_ASC ) );
			}
			else if( IS_CHINESE_S )
			{

				MultiByteToWideChar( CP_UTF8, MB_CUTE,
					S_CHI_1001S, -1, lpszWindowText, 1023 );
				SetWindowText( hDlg, lpszWindowText );
				MultiByteToWideChar( CP_UTF8, MB_CUTE,
					lpTarget->bWatch? S_QUESTION2_CHIS : S_QUESTION1_CHIS,
					-1, msg1, 1023
				);
				lstrcpy( format, TEXT( S_FORMAT2_ASC ) );
			}
			else if( IS_FRENCH )
			{

				MultiByteToWideChar( CP_UTF8, MB_CUTE,
					S_FRE_1001, -1, lpszWindowText, 1023 );
				SetWindowText( hDlg, lpszWindowText );
				MultiByteToWideChar( CP_UTF8, MB_CUTE,
					lpTarget->bWatch? S_QUESTION2_FRE : S_QUESTION1_FRE,
					-1, msg1, 1023
				);
				lstrcpy( format, TEXT( S_FORMAT2_ASC ) );
			}
			else
#endif

			{
				lstrcpy( msg1, lpTarget->bWatch?
					TEXT( S_QUESTION2_ASC ) : TEXT( S_QUESTION1_ASC ) );
				lstrcpy( format, TEXT( S_FORMAT2_ASC ) );

				lstrcpy( lpszWindowText, _T( "Confirmation" ) );
				SetWindowText( hDlg, lpszWindowText );
			}

			wsprintf( msg2, format,
				lpTarget->szPath,
				HIWORD( lpTarget->dwProcessId ), LOWORD( lpTarget->dwProcessId ),
				lstrlen( lpTarget->szText ) ? lpTarget->szText : TEXT( "n/a" )
			);

			SendDlgItemMessage( hDlg, IDC_MSG1, WM_SETFONT, ( WPARAM ) hFont, 0 );
			SendDlgItemMessage( hDlg, IDC_MSG2, WM_SETFONT, ( WPARAM ) hFont, 0 );
			SetDlgItemText( hDlg, IDC_MSG1, msg1 );
			SetDlgItemText( hDlg, IDC_EDIT, msg2 );
			// Anti-Ukagaka
			SetTimer( hDlg, TIMER_ID_Q, 500U, (TIMERPROC) NULL );
			break;
		}
		case WM_COMMAND:
		{
			switch( LOWORD( wParam ) )
			{
				case IDYES:
					EndDialog( hDlg, IDYES );
					break;
				case IDNO:
					EndDialog( hDlg, IDNO );
					break;
				case IDCANCEL:
					EndDialog( hDlg, IDCANCEL );
					break;
			}
		
			break;
		}
		case WM_TIMER:
		{
			SetWindowText( hDlg, lpszWindowText );
			break;
		}
		case WM_DESTROY:
		{
			KillTimer( hDlg, TIMER_ID_Q );
			break;
		}
		default:
		{
			return 0L;
		}
	}
    return 1L;
}
#undef TIMER_ID_Q

LRESULT CALLBACK HookProc( int nCode, WPARAM wParam, LPARAM lParam )
{
	if( nCode < 0 )
	{
		return CallNextHookEx( g_hHook, nCode, wParam, lParam );
	}
	else if( g_hListDlg != NULL && nCode == MSGF_DIALOGBOX )
	{
		LPMSG lpMsg = (LPMSG) lParam;
		WPARAM wKey = lpMsg->wParam;
		WPARAM wId = (WORD) -1;
		if( lpMsg->message != WM_KEYDOWN ) return 0L;
		
		if( wKey == (WPARAM) VK_F5 )
		{
			SendMessage( g_hListDlg, WM_COMMAND, IDC_RELOAD, 0L );
			return 1L;
		}
		
		if(
			lpMsg->hwnd != GetDlgItem( g_hListDlg, IDC_TARGET_LIST )
			&& 
			lpMsg->hwnd != GetDlgItem( g_hListDlg, IDC_EDIT_INFO )
		) return 0L;



		if( wKey == VK_F5 || wKey == 'R' ) wId = IDC_RELOAD;
		else if( wKey == 'L' ) wId = IDOK;
		else if( wKey == 'W' ) wId = IDC_WATCH;
		else if( wKey == 'C' ) wId = IDCANCEL;
		else if( wKey == 'H' ) wId = IDC_HIDE;
		else if( wKey == 'S' ) wId = IDC_SHOW;
		else if( wKey == 'K' ) wId = IDC_SHOW_MANUALLY;
		else if( wKey == 'A' ) wId = IDC_LISTALL_SYS;
		else if( wKey == 'F' ) wId = IDC_FRIEND;
		else if( wKey == 'U' ) wId = IDC_RESET_IFF;
		else if( wKey == 'O' ) wId = IDC_FOE;
		else if( wKey == 'Z' ) wId = IDC_UNFREEZE;
		else if( wKey >= 'A' && wKey <= 'Z' ) return 1L;
		else return 0L;


		

		SendMessage( g_hListDlg, WM_COMMAND, wId, 0L );
		return 1L;
	}
	return 0L;
}

