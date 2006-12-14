#include "BattleEnc.h"

extern BOOL g_bRealTime;
extern BOOL g_bBlock;
BOOL g_bLogging = TRUE;
BYTE g_Slider[ 4 ];

TCHAR g_lpszEnemy[ MAX_PROCESS_CNT ][ MAX_PATH * 2 ];
int g_iEnemyIndex = 0;
TCHAR g_lpszFriend[ MAX_PROCESS_CNT ][ MAX_PATH * 2 ];
int g_iFriendIndex = 0;

HANDLE hSemaphore[ 4 ];
BOOL g_bHack[ 4 ];

DWORD g_dwTargetProcessId[ 4 ];
TCHAR g_szTarget[ 4 ][ 1024 ];

HFONT hFont = NULL;

BOOL LoadSkin( HWND hWnd, HDC& hMemDC, SIZE& PicSize );
BOOL ChangeSkin( HWND hWnd, HDC hMemDC, SIZE& PicSize );
BOOL DrawSkin( HDC hdc, HDC hMemDC, SIZE& SkinSize );



LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
#define TIMER_ID_MAIN (UINT) 'M'

	const RECT urlrect = { 420L, 390L, 620L, 420L };
	static HCURSOR hCursor[ 3 ];
	static iCursor = 0;
	static iMouseDown = FALSE;
	static HANDLE hChildThread[ 4 ] = { NULL, NULL, NULL, NULL };
	static HACK_PARAMS hp[ 4 ];
	static TARGETINFO ti[ 4 ];
	static HFONT hFontItalic;
	static HWND hButton[ 4 ];
	static HWND hToolTip[ 4 ];
	static TCHAR lpszStatus[ 16 ][ MAX_PATH * 2 ];
	static HINSTANCE hInst;
	static TCHAR lpszWindowText[ 1024 ];
	static TCHAR strToolTips[ 4 ][ 256 ];

	static HDC hMemDC = NULL;
	static SIZE SkinSize = { 640L, 480L };

	static UINT uMsgTaskbarCreated;
	switch ( message ) 
	{
		case WM_CREATE:
		{
			hInst = (HINSTANCE) GetWindowLongPtr( hWnd, GWLP_HINSTANCE );

			LoadSkin( hWnd, hMemDC, SkinSize );
			

#ifdef _UNICODE
			if( IS_JAPANESE )
			{
				InitMenuJpn( hWnd );
				InitToolTipsJpn( strToolTips );
			}
			else if( IS_SPANISH )
			{
				InitMenuSpa( hWnd );
				InitToolTipsSpa( strToolTips );
			}
			else if( IS_FINNISH )
			{
				InitMenuFin( hWnd );
				InitToolTipsFin( strToolTips );
			}
			else if( IS_CHINESE_T )
			{
				InitMenuChiT( hWnd );
				InitToolTipsChiT( strToolTips );
			}
			else if( IS_CHINESE_S )
			{
				InitMenuChiS( hWnd );
				InitToolTipsChiS( strToolTips );
			}
			else if( IS_FRENCH )
			{
				InitMenuFre( hWnd );
				InitToolTipsFre( strToolTips );
			}
			else
			{
				InitMenuEng( hWnd );
				InitToolTipsEng( strToolTips );
			}
#else
			InitToolTipsEng( strToolTips );

			HMENU hOptMenu = GetSubMenu( GetMenu( hWnd ), 2 );
			EnableMenuItem( hOptMenu, 4U, MF_BYPOSITION | MF_GRAYED );
#endif
			CheckLanguageMenuRadio( hWnd );

			CheckMenuItem( GetMenu( hWnd ), IDM_LOGGING, MF_BYCOMMAND | 
				( g_bLogging? MFS_CHECKED : MFS_UNCHECKED )
			);

			ZeroMemory( &hp, sizeof( HACK_PARAMS ) * 4 ); // static, but just in case

			hFont = MyCreateFont( TEXT( "Tahoma" ), 17, TRUE, FALSE );

			HDC hDC = GetDC( hWnd );
			hFontItalic = MyCreateFont( hDC, TEXT( "Georgia" ), 13, FALSE, TRUE );
			ReleaseDC( hWnd, hDC );
			
			hButton[ 0 ] = CreateWindow( TEXT( "BUTTON" ), TEXT( "&Target..." ), 
				WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				480,30,130,75,
				hWnd, (HMENU)IDM_LIST, hInst, NULL );
			hButton[ 1 ] = CreateWindow( TEXT( "BUTTON" ), TEXT( "&Unlimit all" ), 
				WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				480,125,130,50,
				hWnd, (HMENU)IDM_STOP, hInst, NULL );
			hButton[ 2 ] = CreateWindow( TEXT( "BUTTON" ), TEXT( "&Control..." ), 
				WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				480,195,130,75,
				hWnd, (HMENU)IDM_SETTINGS, hInst, NULL );
			hButton[ 3 ] = CreateWindow( TEXT( "BUTTON" ), TEXT( "E&xit" ), 
				WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				480,330,130,50,
				hWnd, (HMENU)IDM_EXIT, hInst, NULL );

			EnableWindow( hButton[ 1 ], FALSE );
			EnableWindow( hButton[ 2 ], FALSE );
			EnableMenuItem( GetMenu( hWnd ), IDM_SETTINGS, MF_BYCOMMAND | MF_GRAYED );
			EnableMenuItem( GetMenu( hWnd ), IDM_STOP, MF_BYCOMMAND | MF_GRAYED );
			EnableMenuItem( GetMenu( hWnd ), IDM_UNWATCH, MF_BYCOMMAND | MF_GRAYED );
			
			if( g_bRealTime )
			{
				CheckMenuItem( GetMenu( hWnd ), IDM_REALTIME, MF_BYCOMMAND | MFS_CHECKED );
				wsprintf( lpszWindowText, TEXT("%s - Idle (Real-time mode)"), APP_NAME );
				WriteDebugLog( TEXT( "Real-time mode: Yes" ) );
			}
			else
			{
				CheckMenuItem( GetMenu( hWnd ), IDM_REALTIME, MF_BYCOMMAND | MFS_UNCHECKED );
				wsprintf( lpszWindowText, TEXT("%s - Idle"), APP_NAME );
				WriteDebugLog( TEXT( "Real-time mode: No" ) );
			}
			SetWindowText( hWnd, lpszWindowText );

			for( int i = 0; i < 4; i++ )
			{
				hp[ i ].myHwnd = hWnd;			
				lstrcpy( g_szTarget[ i ], TARGET_UNDEF );
				lstrcpy( ti[ i ].szPath, TARGET_UNDEF );
				lstrcpy( ti[ i ].szExe , TARGET_UNDEF );
				hp[ i ].iMyId = i;
				hp[ i ].lpTarget = &ti[ i ];
				for( int j = 0; j < 16; j++ ) hp[ i ].lpszStatus[ j ] = lpszStatus[ j ];
				g_bHack[ i ] = FALSE;
				hSemaphore[ i ] = CreateSemaphore( NULL, 1L, 1L, NULL );

				hToolTip[ i ] = CreateTooltip( hInst, hButton[ i ], strToolTips[ i ] );
				SendMessage( hButton[ i ], WM_SETFONT, ( WPARAM ) hFontItalic, 0L );
			}
			lstrcpy( lpszStatus[ 0 ], APP_NAME );
			lstrcpy( lpszStatus[ 1 ], TEXT( "Hit [a] for more info." ) );


			uMsgTaskbarCreated = RegisterWindowMessage( TEXT( "TaskbarCreated" ) );

			WriteDebugLog( GetCommandLine() );
	
			TCHAR lpszMyPath[ MAX_PATH * 2 ] = TEXT( "" );
			TCHAR lpszTargetPath[ MAX_PATH * 2 ] = TEXT( "" );
			TCHAR lpszTargetExe[ MAX_PATH * 2 ] = TEXT( "" );

			int iSlider = GetArgument( GetCommandLine(), MAX_PATH * 2, lpszMyPath, lpszTargetPath, lpszTargetExe );

			if( iSlider != IGNORE_ARGV )
			{
				WriteDebugLog( TEXT( "Run from the command line..." ) );
				DWORD dwTargetProcess = PathToProcess( lpszTargetPath );
				if( dwTargetProcess != ( DWORD ) -1 && dwTargetProcess == GetCurrentProcessId() )
				{
					WriteDebugLog( TEXT( "BES cannot target itself!" ) );
					break;
				}
			
				hp[ 2 ].lpTarget->dwProcessId = TARGET_PID_NOT_SET; // for sure

				if( iSlider >= 1 && iSlider <= 99 )
				{
					SetSliderIni( lpszTargetPath, iSlider );
				}

				SetTargetPlus( hWnd, &hChildThread[ 3 ], &hp[ 2 ], lpszTargetPath, lpszTargetExe );
			}


			NOTIFYICONDATA ni;
			//int iDllVersion = 
			InitNotifyIconData( hWnd, &ni, &ti[ 0 ] );
			Shell_NotifyIcon( NIM_ADD, &ni );
			/*
			if( iDllVersion >= 5 )
			{
				ni.uVersion = NOTIFYICON_VERSION;
				Shell_NotifyIcon( NIM_SETVERSION, &ni );
			}
			*/
			
			hCursor[ 0 ] = LoadCursor( (HINSTANCE) NULL, IDC_ARROW );
			hCursor[ 2 ] = hCursor[ 1 ] = LoadCursor( (HINSTANCE) NULL, IDC_HAND );
			SetCursor( hCursor[ 0 ] );

			SetTimer( hWnd, TIMER_ID_MAIN, 500U, (TIMERPROC) NULL );
			break;
		}

		case WM_SIZE:
		{
			if( wParam == SIZE_MINIMIZED )
			{
				ShowWindow( hWnd, SW_HIDE );
			}
			break;
		}

		case WM_MOVE:
		{
			if( ! IsIconic( hWnd ) )
			{
				SetWindowPosIni( hWnd );
			}
			break;				
		}

		case WM_USER_RESTART:
		{
			int id = (int) wParam;
			if( id >= 0 && id <= 2 )
			{
				if( WaitForSingleObject( hSemaphore[ id ], 200 ) == WAIT_OBJECT_0 )
				{
					SendMessage( hWnd, WM_USER_HACK, (WPARAM) id, (LPARAM) &hp[ id ] );
				}
				else
				{
					MessageBox( hWnd, TEXT( "Semaphore Error" ), APP_NAME, MB_OK | MB_ICONEXCLAMATION );
					break;
				}
			}

			break;
		}

		case WM_USER_HACK:
		{
			DWORD dwChildThreadId;
			int iMyId = ( int ) wParam;
			if( ! ( iMyId >= 0 && iMyId < 3 ) ) break;

			LPTARGETINFO lpTarget = ( (LPHACK_PARAMS) lParam ) -> lpTarget;
			g_bHack[ iMyId ] = TRUE;
			SECURITY_ATTRIBUTES sa;
			sa.nLength = sizeof( SECURITY_ATTRIBUTES );
			sa.lpSecurityDescriptor = NULL;
			sa.bInheritHandle = FALSE;

			hChildThread[ iMyId ] = 
				CreateThread(
					&sa,
					(DWORD) 0,
					(LPTHREAD_START_ROUTINE) Hack,
					(LPVOID) (LPHACK_PARAMS) lParam,
					(DWORD) 0,
					&dwChildThreadId
				);

			if( hChildThread[ iMyId ] == NULL )
			{
				ReleaseSemaphore( hSemaphore[ iMyId ], 1L, (LPLONG) NULL );
				g_bHack[ iMyId ] = FALSE;
				MessageBox( hWnd, TEXT( "CreateThread failed." ), APP_NAME, MB_OK | MB_ICONSTOP );
				break;
			}
			else
			{
/*
				if( g_bRealTime )
				{
					SetThreadPriority( hChildThread[ iMyId ],
						THREAD_PRIORITY_TIME_CRITICAL );
				}
				else
				{
					SetThreadPriority( hChildThread[ iMyId ], THREAD_PRIORITY_NORMAL );
				}
*/

				SetThreadPriority( hChildThread[ iMyId ],
						THREAD_PRIORITY_TIME_CRITICAL );



				g_dwTargetProcessId[ iMyId ] = lpTarget->dwProcessId;
				lstrcpy( g_szTarget[ iMyId ], lpTarget->szPath );

				wsprintf( lpszWindowText, TEXT( "%s - Active%s" ), APP_NAME,
					g_bRealTime ? TEXT( " (Real-time mode)" ) : TEXT( "" ) );

				SetWindowText( hWnd, lpszWindowText );
				UpdateStatus( hWnd );
			}
			break;
		}

		case WM_USER_STOP:
		{
			g_bBlock = FALSE;

			TCHAR str[ 1024 ];
			wsprintf( str, TEXT( "WM_USER_STOP: wParam = 0x%04lX , lParam = 0x%04lX" ), wParam, (UINT) lParam );

			WriteDebugLog( str );

			int iMyId = (int) wParam;

			if( iMyId < 0 || iMyId > 3 ) break;

			if( iMyId != 3 ) // 3 : JUST_UPDATE_STATUS
			{
				g_bHack[ iMyId ] = FALSE;
				DWORD dwExitCode = 0UL;
				if( WaitForSingleObject( hSemaphore[ iMyId ], 1000UL ) == WAIT_OBJECT_0 )
				{
					for( int t = 0; t < 50; t++ )
					{
						GetExitCodeThread( hChildThread[ iMyId ], &dwExitCode );
						if( dwExitCode != STILL_ACTIVE )
						{
							break;
						}
						Sleep( 50UL );
					}

					wsprintf( str, TEXT( "ChildThread[ %d ] : ExitCode = 0x%04lX" ), iMyId, dwExitCode );
					WriteDebugLog( str );
					ReleaseSemaphore( hSemaphore[ iMyId ], 1L, (LPLONG) NULL );
					wsprintf( lpszStatus[ 0 + iMyId * 4 ], TEXT( "Target #%d %s" ), iMyId + 1,
						dwExitCode == NORMAL_TERMINATION ? TEXT( "OK" )	:
						dwExitCode == THREAD_NOT_OPENED ? TEXT( "Access denied" ) :
						dwExitCode == TARGET_MISSING ? TEXT( "Target missinig" ) :
						dwExitCode == STILL_ACTIVE ?  TEXT( "Time out" ) :
						dwExitCode == NOT_WATCHING ? TEXT( "Unwatch: OK" ) : TEXT( "Status unknown" )
					);

					if( hChildThread[ iMyId ] )
					{
						CloseHandle( hChildThread[ iMyId ] );
						hChildThread[ iMyId ] = NULL;
					}
				}
				else
				{
					WriteDebugLog( TEXT( "### Semaphore Error ###" ) );
					GetExitCodeThread( hChildThread[ iMyId ], &dwExitCode );
					wsprintf( str, TEXT( "ChildThread[ %d ] : ExitCode = 0x%04lX" ), iMyId, dwExitCode );
					WriteDebugLog( str );
					wsprintf( lpszStatus[ 0 + iMyId * 4 ], TEXT( "Target #%d Semaphore Error" ), iMyId + 1 );
				}

				EnableWindow( hButton[ 0 ], TRUE );
				EnableMenuItem( GetMenu( hWnd ), IDM_LIST, MF_BYCOMMAND | MF_ENABLED );

			}
			
			
			if( IsActive() )
			{
				TCHAR strStatus[ 1024 ] = TEXT( " Limiting CPU load:" );
#ifdef _UNICODE
				if( IS_JAPANESE )
				{
					MultiByteToWideChar( CP_UTF8, MB_CUTE, S_JPN_2000, -1, strStatus, 1023 );
				}
				else if( IS_FINNISH )
				{
					MultiByteToWideChar( CP_UTF8, MB_CUTE, S_FIN_2000, -1, strStatus, 1023 );
				}
				else if( IS_SPANISH )
				{
					MultiByteToWideChar( CP_UTF8, MB_CUTE, S_SPA_2000, -1, strStatus, 1023 );
				}
				else if( IS_CHINESE_T )
				{
					MultiByteToWideChar( CP_UTF8, MB_CUTE, S_CHI_2000T, -1, strStatus, 1023 );
				}
				else if( IS_CHINESE_S )
				{
					MultiByteToWideChar( CP_UTF8, MB_CUTE, S_CHI_2000S, -1, strStatus, 1023 );
				}
				else if( IS_FRENCH )
				{
					MultiByteToWideChar( CP_UTF8, MB_CUTE, S_FRE_2000, -1, strStatus, 1023 );
				}
#endif
				if( g_bHack[ 0 ] )
				{
					lstrcat( strStatus, TEXT( " #1" ) );
				}
				
				if( g_bHack[ 1 ] )
				{
					lstrcat( strStatus, TEXT( " #2" ) );
				}
				
				if( g_bHack[ 2 ] )
				{
					if( g_dwTargetProcessId[ 2 ] != (DWORD) -1 ) // -1 means "watching"
					{
						lstrcat( strStatus, TEXT( " #3" ) );
					}
				}
		
				wsprintf( lpszStatus[ 12 ], TEXT( "%s" ), strStatus );

				EnableWindow( hButton[ 1 ], TRUE );
				EnableMenuItem( GetMenu( hWnd ), IDM_STOP, MF_BYCOMMAND | MF_ENABLED );
				EnableWindow( hButton[ 2 ], TRUE );
				EnableMenuItem( GetMenu( hWnd ), IDM_SETTINGS, MF_BYCOMMAND | MF_ENABLED );
				EnableWindow( hButton[ 3 ], FALSE );
				EnableMenuItem( GetMenu( hWnd ), IDM_EXIT, MF_BYCOMMAND | MF_GRAYED );
				EnableMenuItem( GetMenu( hWnd ), IDM_EXIT_ANYWAY, MF_BYCOMMAND | MF_ENABLED );

				SetClassLong( hWnd, GCL_HICON, (LONG) LoadIcon( hInst, MAKEINTRESOURCE( IDI_ACTIVE ) ) );
				SetClassLong( hWnd, GCL_HICONSM, (LONG) LoadIcon( hInst, MAKEINTRESOURCE( IDI_ACTIVE ) ) );
				SendMessage( hWnd, WM_SETICON, ICON_BIG, (LPARAM) LoadIcon( hInst, MAKEINTRESOURCE( IDI_ACTIVE ) ) );
				SendMessage( hWnd, WM_SETICON, ICON_SMALL, (LPARAM) LoadIcon( hInst, MAKEINTRESOURCE( IDI_ACTIVE ) ) );

			}
			else
			{
				lstrcpy( lpszStatus[ 12 ], TEXT( "" ) );
				if( g_dwTargetProcessId[ 2 ] == (DWORD) -1 ) // Watching Idly
				{
					EnableWindow( hButton[ 1 ], TRUE );
					EnableMenuItem( GetMenu( hWnd ), IDM_STOP, MF_BYCOMMAND | MF_ENABLED );
				}
				else
				{
					EnableWindow( hButton[ 1 ], FALSE );
				}
				if( g_dwTargetProcessId[ 0 ] || g_dwTargetProcessId[ 1 ] || g_dwTargetProcessId[ 2 ] )
				{
					EnableWindow( hButton[ 2 ], TRUE );
					EnableMenuItem( GetMenu( hWnd ), IDM_SETTINGS, MF_BYCOMMAND | MF_ENABLED );
				}
				else
				{
					EnableWindow( hButton[ 2 ], FALSE );
					EnableMenuItem( GetMenu( hWnd ), IDM_SETTINGS, MF_BYCOMMAND | MF_GRAYED );
				}

				EnableWindow( hButton[ 3 ], TRUE );
				EnableMenuItem( GetMenu( hWnd ), IDM_EXIT, MF_BYCOMMAND | MF_ENABLED );
				EnableMenuItem( GetMenu( hWnd ), IDM_EXIT_ANYWAY, MF_BYCOMMAND | MF_GRAYED );

				SetClassLong( hWnd, GCL_HICON, (LONG) LoadIcon( hInst, MAKEINTRESOURCE( IDI_IDLE ) ) );
				SetClassLong( hWnd, GCL_HICONSM, (LONG) LoadIcon( hInst, MAKEINTRESOURCE( IDI_IDLE ) ) );
				SendMessage( hWnd, WM_SETICON, ICON_BIG, (LPARAM) LoadIcon( hInst, MAKEINTRESOURCE( IDI_IDLE ) ) );
				SendMessage( hWnd, WM_SETICON, ICON_SMALL, (LPARAM) LoadIcon( hInst, MAKEINTRESOURCE( IDI_IDLE ) ) );

				wsprintf( lpszWindowText, TEXT( "%s - Idle%s" ), APP_NAME,
					g_bRealTime ? TEXT( " (Real-time mode)" ) : TEXT( "" ) );
				SetWindowText( hWnd, lpszWindowText );
			}


			if( g_bHack[ 3 ] )
			{
				EnableMenuItem( GetMenu( hWnd ), IDM_WATCH, MF_BYCOMMAND | MF_GRAYED );
				EnableMenuItem( GetMenu( hWnd ), IDM_UNWATCH, MF_BYCOMMAND | MF_ENABLED );
			}
			else if( g_bHack[ 2 ] )
			{
				EnableMenuItem( GetMenu( hWnd ), IDM_WATCH, MF_BYCOMMAND | MF_GRAYED );
				EnableMenuItem( GetMenu( hWnd ), IDM_UNWATCH, MF_BYCOMMAND | MF_GRAYED );
			}
			else
			{
				EnableMenuItem( GetMenu( hWnd ), IDM_WATCH, MF_BYCOMMAND | MF_ENABLED );
				EnableMenuItem( GetMenu( hWnd ), IDM_UNWATCH, MF_BYCOMMAND | MF_GRAYED );
			}



			if( g_bHack[ 0 ] && g_bHack[ 1 ] && ( g_bHack[ 2 ] || g_bHack[ 3 ] ) )
			{
				EnableWindow( hButton[ 0 ], FALSE );
				EnableMenuItem( GetMenu( hWnd ), IDM_LIST, MF_BYCOMMAND | MF_GRAYED );
			}
			else
			{
				EnableWindow( hButton[ 0 ], TRUE );
				EnableMenuItem( GetMenu( hWnd ), IDM_LIST, MF_BYCOMMAND | MF_ENABLED );
			}

			if( g_bHack[ 2 ] || g_bHack[ 3 ] )
			{
				EnableMenuItem( GetMenu( hWnd ), IDM_WATCH, MF_BYCOMMAND | MF_GRAYED );
			}
			else
			{
				EnableMenuItem( GetMenu( hWnd ), IDM_WATCH, MF_BYCOMMAND | MF_ENABLED );
			}

			NOTIFYICONDATA ni;
			//int iDllVersion = 
			InitNotifyIconData( hWnd, &ni, &ti[ 0 ] );

///* XP won't work NIF_INFO here...
/*
			if( g_bHack[ 2 ] && g_bHack[ 3 ] && g_dwTargetProcessId[ 2 ] != WATCHING_IDLE && iDllVersion >= 5 )
			{
				ni.uFlags = NIF_ICON | NIF_INFO | NIF_TIP;
			}
*/
			Shell_NotifyIcon( NIM_MODIFY, &ni );

			InvalidateRect( hWnd, NULL, 0 );


			break;
		}

		case WM_USER_NOTIFYICON:
		{
			static BOOL bClicked = FALSE;
			if( lParam == WM_LBUTTONDOWN )
			{
				bClicked = TRUE;
			}
			else if( lParam == WM_LBUTTONUP || lParam == WM_LBUTTONDBLCLK || lParam == NIN_KEYSELECT || lParam == NIN_SELECT )
			{
				if( bClicked || lParam != WM_LBUTTONUP )
				{
					ShowWindow( hWnd, SW_RESTORE );
					SetForegroundWindow( hWnd );
				}
				bClicked = FALSE;
			}
			else if( lParam == WM_RBUTTONDOWN || lParam == WM_CONTEXTMENU )
			{
				bClicked = FALSE;

				HMENU hMenu = CreatePopupMenu();
				POINT pt;
				/*
				if( GetShell32Version() >= 5 )
				{
					AppendMenu( hMenu, MFT_STRING, IDM_TRAY_STATUS, TEXT( "&Status" ) );
					AppendMenu( hMenu, MFT_SEPARATOR, 0U, NULL );
				}*/

				TCHAR str[ 3 ][ 256 ];
#ifdef _UNICODE 
				if( IS_JAPANESE )
				{
					MultiByteToWideChar( CP_UTF8, MB_CUTE, JPN_24, -1, str[ 0 ], 255 );
					MultiByteToWideChar( CP_UTF8, MB_CUTE, JPN_5, -1, str[ 1 ], 255 );
					MultiByteToWideChar( CP_UTF8, MB_CUTE, JPN_26, -1, str[ 2 ], 255 );
				}
				else if( IS_FINNISH )
				{
					MultiByteToWideChar( CP_UTF8, MB_CUTE, FIN_24, -1, str[ 0 ], 255 );
					MultiByteToWideChar( CP_UTF8, MB_CUTE, FIN_5, -1, str[ 1 ], 255 );
					MultiByteToWideChar( CP_UTF8, MB_CUTE, FIN_26, -1, str[ 2 ], 255 );
				}
				else if( IS_SPANISH )
				{
					MultiByteToWideChar( CP_UTF8, MB_CUTE, SPA_24, -1, str[ 0 ], 255 );
					MultiByteToWideChar( CP_UTF8, MB_CUTE, SPA_5, -1, str[ 1 ], 255 );
					MultiByteToWideChar( CP_UTF8, MB_CUTE, SPA_26, -1, str[ 2 ], 255 );
				}
				else if( IS_CHINESE_T )
				{
					MultiByteToWideChar( CP_UTF8, MB_CUTE, CHT_24, -1, str[ 0 ], 255 );
					MultiByteToWideChar( CP_UTF8, MB_CUTE, CHT_5, -1, str[ 1 ], 255 );
					MultiByteToWideChar( CP_UTF8, MB_CUTE, CHT_26, -1, str[ 2 ], 255 );
				}
				else if( IS_CHINESE_S )
				{
					MultiByteToWideChar( CP_UTF8, MB_CUTE, CHS_24, -1, str[ 0 ], 255 );
					MultiByteToWideChar( CP_UTF8, MB_CUTE, CHS_5, -1, str[ 1 ], 255 );
					MultiByteToWideChar( CP_UTF8, MB_CUTE, CHS_26, -1, str[ 2 ], 255 );
				}
				else if( IS_FRENCH )
				{
					MultiByteToWideChar( CP_UTF8, MB_CUTE, FRE_24, -1, str[ 0 ], 255 );
					MultiByteToWideChar( CP_UTF8, MB_CUTE, FRE_5, -1, str[ 1 ], 255 );
					MultiByteToWideChar( CP_UTF8, MB_CUTE, FRE_26, -1, str[ 2 ], 255 );
				}
				else
				{
					MultiByteToWideChar( CP_UTF8, MB_CUTE, ENG_24, -1, str[ 0 ], 255 );
					MultiByteToWideChar( CP_UTF8, MB_CUTE, ENG_5, -1, str[ 1 ], 255 );
					MultiByteToWideChar( CP_UTF8, MB_CUTE, ENG_26, -1, str[ 2 ], 255 );
				}
#else
				lstrcpy( str[ 0 ], TEXT( "&Restore BES" ) );
				lstrcpy( str[ 1 ], TEXT( "&Unlimit all" ) );
				lstrcpy( str[ 2 ], TEXT( "E&xit" ) );
#endif
				AppendMenu( hMenu, MFT_STRING, IDM_SHOWWINDOW, str[ 0 ] );
				AppendMenu( hMenu, MFT_SEPARATOR, 0U, NULL );
				if( ! IsIconic( hWnd ) )
				{
					EnableMenuItem( hMenu, IDM_SHOWWINDOW, MF_BYCOMMAND | MF_GRAYED );
				}

				AppendMenu( hMenu, MFT_STRING, IDM_STOP_FROM_TRAY, str[ 1 ] );

				AppendMenu( hMenu, MFT_SEPARATOR, 0U, NULL );
				if( ! IsActive() && ! g_bHack[ 3 ] )
				{
					EnableMenuItem( hMenu, IDM_STOP_FROM_TRAY, MF_BYCOMMAND | MF_GRAYED );
				}
				
				AppendMenu( hMenu, MFT_STRING, IDM_EXIT_FROM_TRAY, str[ 2 ] );

				EnableMenuItem( hMenu, IDM_EXIT_FROM_TRAY,
					( MF_BYCOMMAND | ( IsActive() ? MF_GRAYED : MF_ENABLED ) )
				);

				GetCursorPos( &pt );
				SetForegroundWindow( hWnd );
				TrackPopupMenu( hMenu, TPM_RIGHTBUTTON | TPM_TOPALIGN | TPM_LEFTALIGN,
					pt.x, pt.y, 0, hWnd, (LPRECT) NULL );
			}
			break;
		}

		// Handling rare cases when the user LBUTTONDOWNed on one of the buttons
		// created as child windows, and then he or she LBUTTONUPed *not* on it.
		// The button was not pushed after all,
		// and without this algo, the button will keep having the focus.
		// We let the parent (hWnd) reget the focus here.
		// WM_USER_BUTTON is sent from the GetMessage Loop (main.cpp)
		// whenever WM_LBUTTONUP is sent to a window whose handle is not hWnd.
		// wParam is its handle.
		case WM_USER_BUTTON:
		{
			HWND h = (HWND) wParam;
			// Who received WM_LBUTTONUP ?
			int iHotButton;
			if( h == hButton[ 0 ] ) iHotButton = 0;
			else if( h == hButton[ 1 ] ) iHotButton = 1;
			else if( h == hButton[ 2 ] ) iHotButton = 2;
			else if( h == hButton[ 3 ] ) iHotButton = 3;
			else break;

			// Where is the Mouse Cursor now?
			POINT pt;
			GetCursorPos( &pt );
			ScreenToClient( hWnd, &pt );
			int x = (int) pt.x;
			int y = (int) pt.y;

			// Is the Mouse Cursor on the 'hot' button?
			int iCursorPos;
			if( x < BTN_X1 || x > BTN_X2 ) iCursorPos = -1;
			else if( y >= BTN0_Y1 && y <= BTN0_Y2 ) iCursorPos = 0;
			else if( y >= BTN1_Y1 && y <= BTN1_Y2 ) iCursorPos = 1;
			else if( y >= BTN2_Y1 && y <= BTN2_Y2 ) iCursorPos = 2;
			else if( y >= BTN3_Y1 && y <= BTN3_Y2 ) iCursorPos = 3;
			else iCursorPos = -1;

			// If no, we reset the focus.
			if( iCursorPos != iHotButton )
			{
				SetFocus( hWnd );
			}
			break;
		}

		case WM_COMMAND:
		{
			switch( LOWORD(wParam) )
			{
				case IDM_LIST:
				{
					int iMyId = 3;

					if( !g_bHack[ 0 ] && WaitForSingleObject( hSemaphore[ 0 ], 100UL ) == WAIT_OBJECT_0 ) iMyId = 0;
					else if( !g_bHack[ 1 ] && WaitForSingleObject( hSemaphore[ 1 ], 100UL ) == WAIT_OBJECT_0 ) iMyId = 1;
					else if( !g_bHack[ 2 ] && WaitForSingleObject( hSemaphore[ 2 ], 100UL ) == WAIT_OBJECT_0 ) iMyId = 2;
					else
					{
						MessageBox( hWnd, TEXT( "Semaphore Error" ), APP_NAME, MB_OK | MB_ICONEXCLAMATION );
						break;
					}

					int iReturn = DialogBoxParam( hInst,
						(LPCTSTR) IDD_XLIST, hWnd, (DLGPROC) xList, (LPARAM) &hp[ iMyId ] );

					if( iReturn == XLIST_CANCELED )
					{
						ReleaseSemaphore( hSemaphore[ iMyId ], 1L, NULL );
						break;
					}
					else if( iReturn == XLIST_RESTART_0 || iReturn == XLIST_RESTART_1 || iReturn == XLIST_RESTART_2 )
					{
						ReleaseSemaphore( hSemaphore[ iMyId ], 1L, (LONG *) NULL );
						SendMessage( hWnd, WM_USER_RESTART, (WPARAM) iReturn, 0L );
						break;
					}
					else if( iReturn == XLIST_NEW_TARGET )
					{
						g_Slider[ iMyId ] = (BYTE) GetSliderIni( hp[ iMyId ].lpTarget->szPath );

						TCHAR str[ 1024 ];
						wsprintf( str, TEXT( "GetSliderIni( \"%s\" ) = %d" ),
							hp[ iMyId ].lpTarget->szPath , (int) g_Slider[ iMyId ] );
						WriteDebugLog( str );

						SendMessage( hWnd, WM_USER_HACK, (WPARAM) iMyId, (LPARAM) &hp[ iMyId ] );

						break;
					}
					else if( iReturn == XLIST_WATCH_THIS )
					{
						if( g_bHack[ 2 ] || g_bHack[ 3 ] )
						{
							MessageBox( hWnd, TEXT( "Busy. Can't start watching." ),
								APP_NAME, MB_ICONEXCLAMATION | MB_OK );
							break;
						}
						
						if( iMyId != 2 )
						{
							// ZeroMemory( &hp[ iMyId ], sizeof( HACK_PARAMS ) ); <-- THIS IS BAD!!!
							hp[ 2 ].iMyId = 2;
							hp[ 2 ].lpTarget->dwProcessId = hp[ iMyId ].lpTarget->dwProcessId;
							for( int s = 0; s < (int) hp[ iMyId ].lpTarget->wThreadCount; s++ )
							{
								hp[ 2 ].lpTarget->dwThreadId[ s ] = hp[ iMyId ].lpTarget->dwThreadId[ s ];
							}
							hp[ 2 ].lpTarget->iIFF = hp[ iMyId ].lpTarget->iIFF;
							lstrcpy( hp[ 2 ].lpTarget->szExe, hp[ iMyId ].lpTarget->szExe );
							lstrcpy( hp[ 2 ].lpTarget->szPath, hp[ iMyId ].lpTarget->szPath );
							lstrcpy( hp[ 2 ].lpTarget->szText, hp[ iMyId ].lpTarget->szText );
							hp[ 2 ].lpTarget->wThreadCount = hp[ iMyId ].lpTarget->wThreadCount;

							hp[ iMyId ].lpTarget->dwProcessId = TARGET_PID_NOT_SET;
							hp[ iMyId ].lpTarget->iIFF = IFF_UNKNOWN;
							lstrcpy( hp[ iMyId ].lpTarget->szExe, TARGET_UNDEF );
							lstrcpy( hp[ iMyId ].lpTarget->szPath, TARGET_UNDEF );
							lstrcpy( hp[ iMyId ].lpTarget->szText, TEXT( "" ) );
							hp[ iMyId ].lpTarget->wThreadCount = (WORD) 0;
							g_dwTargetProcessId[ iMyId ] = 0UL;
							lstrcpy( g_szTarget[ iMyId ], TARGET_UNDEF );
							for( int i = 0 + iMyId * 4; i < 4 + iMyId * 4; i++ )
							{
								lstrcpy( lpszStatus[ i ], TEXT( "" ) );
							}

						}

						ReleaseSemaphore( hSemaphore[ iMyId ], 1L, NULL );

						SetTargetPlus( hWnd, &hChildThread[ 3 ], &hp[ 2 ],
							hp[ 2 ].lpTarget->szPath, hp[ 2 ].lpTarget->szExe );
						break;
					}
					else if( iReturn == XLIST_UNFREEZE )
					{
						SendMessage( hWnd, WM_COMMAND, (WPARAM) IDM_STOP, (LPARAM) 0 );
						Sleep( 1000 );
						if( Unfreeze( hWnd, hp[ iMyId ].lpTarget ) )
						{
							MessageBox( hWnd, TEXT( "Successful!" ),
								TEXT( "Unfreezing" ), MB_ICONINFORMATION | MB_OK );
						}
						else
						{
							MessageBox( hWnd, TEXT( "An error occurred.\r\nPlease retry." ),
								TEXT( "Unfreezing" ), MB_ICONEXCLAMATION | MB_OK );
						}
						ReleaseSemaphore( hSemaphore[ iMyId ], 1L, NULL );
						break;
					}
					break;
				}

				case IDM_WATCH:
				{
					SetTargetPlus( hWnd, &hChildThread[ 3 ], &hp[ 2 ] );
					break;
				}

				case IDM_UNWATCH:
				{//**
					WriteDebugLog( TEXT( "IDM_UNWATCH" ) );
					Unwatch( hSemaphore[ 3 ], hChildThread[ 3 ], &g_bHack[ 3 ] );
					break;
				}

				case IDM_STOP:
				{
					WriteDebugLog( TEXT( "IDM_STOP" ) );
					if( g_bHack[ 3 ] ) // Watching...
					{
						Unwatch( hSemaphore[ 3 ], hChildThread[ 3 ], &g_bHack[ 3 ] );
					}

					for( int i = 0; i < 3; i++ )
					{
						if( g_bHack[ i ] )
						{
							SendMessage( hWnd, WM_USER_STOP, (WPARAM) i, 0L );
						}
					}
					break;
				}

				case IDM_STOP_FROM_TRAY:
				{
					WriteDebugLog( TEXT( "IDM_STOP_FROM_TRAY" ) );
					if( g_bHack[ 3 ] ) // Watching...
					{
						Unwatch( hSemaphore[ 3 ], hChildThread[ 3 ], &g_bHack[ 3 ] );
					}

					for( int i = 0; i < 3; i++ )
					{
						if( g_bHack[ i ] )
						{
							SendMessage( hWnd, WM_USER_STOP, (WPARAM) i, (LPARAM) STOP_FROM_TRAY );
						}
					}
					break;
				}
				
				case IDM_SETTINGS:
				{
					DialogBoxParam( hInst, (LPCTSTR)IDD_SETTINGS, hWnd, (DLGPROC) Settings, (LPARAM) &hp[ 0 ] );
					UpdateStatus( hWnd ); // <-- 1.0 beta2
		 			break;
				}

				case IDM_REALTIME:
				{
					// toggle the mode...
					g_bRealTime = ! g_bRealTime;

					// Change the priority
					SetRealTimeMode( hWnd,
						g_bRealTime,
						&hChildThread[ 0 ],
						lpszWindowText
					);
					break;
				}


				case IDM_LOGGING:
				{
					UINT uMenuState = GetMenuState( GetMenu( hWnd ), IDM_LOGGING, MF_BYCOMMAND );
					if( uMenuState == (UINT) -1 )
					{
						break;
					}

					if( ( uMenuState & MFS_CHECKED ) )
					{
						if( EnableLoggingIni( FALSE ) )
						{
							MessageBox( hWnd,
								TEXT( "Logging will be disabled from the next time." ),
								APP_NAME,
								MB_ICONINFORMATION | MB_OK
							);
							CheckMenuItem( GetMenu( hWnd ), IDM_LOGGING, MF_BYCOMMAND | MFS_UNCHECKED );
						}
					}
					else
					{
						if( EnableLoggingIni( TRUE ) )
						{
							MessageBox( hWnd,
								TEXT( "Logging will be enabled from the next time." ),
								APP_NAME,
								MB_ICONINFORMATION | MB_OK
							);
							CheckMenuItem( GetMenu( hWnd ), IDM_LOGGING, MF_BYCOMMAND | MFS_CHECKED );
						}
					}
					break;
				}

				case IDM_SHOWWINDOW:
				{
					SendMessage( hWnd, WM_USER_NOTIFYICON, 0U, WM_LBUTTONDBLCLK );
					break;
				}

				case IDM_ABOUT:
				{
					DialogBoxParam(hInst, (LPCTSTR) IDD_ABOUTBOX, hWnd, (DLGPROC) About, (LPARAM) hWnd );
		 			break;
				}
				case IDM_ONLINEHELP:
				{
					OpenBrowser( TEXT( "http://hp.vector.co.jp/authors/VA022257/BES/" ) );
					break;
				}
				case IDM_HOMEPAGE:
				{
					OpenBrowser( APP_HOME_URL );
					break;
				}
				
				case IDM_GHOSTCENTER:
				{
					//OpenBrowser( TEXT( "http://ngc.sherry.jp/" ) );
					OpenBrowser( TEXT( "http://www.aqrs.jp/ngc/" ) );
					//OpenBrowser( TEXT( "http://ghosttown.mikage.jp/" ) );
					break;
				}
				
				case IDM_UKAGAKA:
				{
					OpenBrowser( TEXT( "http://usada.sakura.vg/" ) );
					break;
				}
				case IDM_SSP:
				{
					OpenBrowser( TEXT( "http://ssp.shillest.net/" ) );
					break;
				}
				case IDM_CROW:
				{
					OpenBrowser( TEXT( "http://crow.aqrs.jp/" ) );
					break;
				}

				case IDM_ABOUT_SHORTCUTS:
				{
					AboutShortcuts( hWnd );
					break;
				}

				case IDM_SKIN:
				{
					ChangeSkin( hWnd, hMemDC, SkinSize );
					break;
				}

				case IDM_SNAP:
				{
					SaveSnap( hWnd );
					break;
				}

				case IDM_EXIT_FROM_TRAY:
				{
					if( IsActive() )
					{
						MessageBox( hWnd, TEXT( "Can't exit now because it's active." ),
							APP_NAME, MB_OK | MB_ICONEXCLAMATION );
					}
					else
					{
						Exit_CommandFromTaskbar( hWnd );
					}
				    break;
				}
				case IDM_EXIT:
				{
					SendMessage( hWnd, WM_CLOSE, 0U, 0L );
				    break;
				}
				case IDM_EXIT_ANYWAY:
				{
					HFONT hMyFont = MyCreateFont( _T("Verdana"), 32, TRUE, FALSE );
					HDC hDC = GetDC( hWnd );
					HFONT hOldFont = ( HFONT ) SelectObject( hDC, hMyFont );
					SetBkMode( hDC, OPAQUE );
					SetBkColor( hDC, RGB( 0xFF, 0xFF, 0x00 ) );
					SetTextColor( hDC, RGB( 0xFF, 0x00, 0x00 ) );
					TCHAR str[ 100 ] = _T( " Forced Termination... " );
					TextOut( hDC, 50, 50, str, lstrlen( str ) );
					SelectObject( hDC, hOldFont );
					ReleaseDC( hWnd, hDC );
					DeleteFont( hMyFont );
					
					SendMessage( hWnd, WM_COMMAND, (WPARAM) IDM_STOP, 0L );
					Sleep( 2000 );
					SendMessage( hWnd, WM_CLOSE, 0U, 0L );
					break;
				}

				case IDM_GSHOW:
				{
					GShow( hWnd );
					break;
				}
#ifdef _UNICODE
				case IDM_ENGLISH:
				{
					SetLanguage( LANG_ENGLISH );

					InitToolTipsEng( strToolTips );
					for( int i = 0; i < 4; i++ )
					{
						UpdateTooltip( hInst, hButton[ i ], strToolTips[ i ], hToolTip[ i ] );
					}
					InitMenuEng( hWnd );
					CheckLanguageMenuRadio( hWnd );
					InvalidateRect( hWnd, (LPRECT) NULL, 0 );
					break;
				}
				case IDM_FRENCH:
				{
					SetLanguage( LANG_FRENCH );

					InitToolTipsFre( strToolTips );
					for( int i = 0; i < 4; i++ )
					{
						UpdateTooltip( hInst, hButton[ i ], strToolTips[ i ], hToolTip[ i ] );
					}
					InitMenuFre( hWnd );
					CheckLanguageMenuRadio( hWnd );
					InvalidateRect( hWnd, (LPRECT) NULL, 0 );
					break;
				}
				case IDM_FINNISH:
				{
					SetLanguage( LANG_FINNISH );

					InitToolTipsFin( strToolTips );
					for( int i = 0; i < 4; i++ )
					{
						UpdateTooltip( hInst, hButton[ i ], strToolTips[ i ], hToolTip[ i ] );
					}
					InitMenuFin( hWnd );
					CheckLanguageMenuRadio( hWnd );
					InvalidateRect( hWnd, (LPRECT) NULL, 0 );
					break;
				}
				case IDM_SPANISH:
				{
					SetLanguage( LANG_SPANISH );

					InitToolTipsSpa( strToolTips );
					for( int i = 0; i < 4; i++ )
					{
						UpdateTooltip( hInst, hButton[ i ], strToolTips[ i ], hToolTip[ i ] );
					}
					InitMenuSpa( hWnd );
					CheckLanguageMenuRadio( hWnd );
					InvalidateRect( hWnd, (LPRECT) NULL, 0 );
					break;
				}
				case IDM_CHINESE_T:
				{
					SetLanguage( LANG_CHINESE_T );
					
					InitToolTipsChiT( strToolTips );
					for( int i = 0; i < 4; i++ )
					{
						UpdateTooltip( hInst, hButton[ i ], strToolTips[ i ], hToolTip[ i ] );
					}
					InitMenuChiT( hWnd );
					CheckLanguageMenuRadio( hWnd );
					InvalidateRect( hWnd, (LPRECT) NULL, 0 );
					break;
				}
				case IDM_CHINESE_S:
				{
					SetLanguage( LANG_CHINESE_S );

					InitToolTipsChiS( strToolTips );
					for( int i = 0; i < 4; i++ )
					{
						UpdateTooltip( hInst, hButton[ i ], strToolTips[ i ], hToolTip[ i ] );
					}
					InitMenuChiS( hWnd );
					CheckLanguageMenuRadio( hWnd );
					InvalidateRect( hWnd, (LPRECT) NULL, 0 );
					break;
				}
				case IDM_JAPANESEo:
				{
					SetLanguage( LANG_JAPANESEo );

					InitToolTipsJpn( strToolTips );
					for( int i = 0; i < 4; i++ )
					{
						UpdateTooltip( hInst, hButton[ i ], strToolTips[ i ], hToolTip[ i ] );
					}
					InitMenuJpn( hWnd );
					CheckLanguageMenuRadio( hWnd );
					InvalidateRect( hWnd, (LPRECT) NULL, 0 );
					break;
				}
				case IDM_JAPANESE:
				{
					SetLanguage( LANG_JAPANESE );

					InitToolTipsJpn( strToolTips );
					for( int i = 0; i < 4; i++ )
					{
						UpdateTooltip( hInst, hButton[ i ], strToolTips[ i ], hToolTip[ i ] );
					}
					InitMenuJpn( hWnd );
					CheckLanguageMenuRadio( hWnd );
					InvalidateRect( hWnd, (LPRECT) NULL, 0 );
					break;
				}
#endif
			}
			break;
		}

		case WM_MOUSEMOVE:
		{
			if( GetFocus() != hWnd ) break;
			if( iMouseDown == -1 ) break;

			int iPrevCursor = iCursor;
			iCursor = MainWindowUrlHit( lParam );
			if( iCursor == 1 && iMouseDown == 1 ) iCursor = 2;


/*
			// This is actually enough:
			if( iPrevCursor != iCursor )
			{
				SetCursor( hCursor[ iCursor ] );
				InvalidateRect( hWnd, &urlrect, TRUE );
			}
*/

			// But we do this just in case:
			if( iCursor != 0 ) SetCursor( hCursor[ 1 ] );

			if( iPrevCursor != iCursor )
			{
				if( iCursor == 0 ) SetCursor( hCursor[ 0 ] );
				InvalidateRect( hWnd, &urlrect, 0 );
			}



			break;
		}
		case WM_LBUTTONDOWN:
		{
			SetCapture( hWnd );
			
			if( MainWindowUrlHit( lParam ) )
			{
				iMouseDown = 1;
				SetCursor( hCursor[ 1 ] );
				iCursor = 2;
				InvalidateRect( hWnd, &urlrect, 0 );
			}
			else
			{
				iMouseDown = -1;
			}
			break;
		}

		case WM_LBUTTONUP:
		{
			ReleaseCapture();
			

			if( iMouseDown == 1 && MainWindowUrlHit( lParam ) )
			{
				OpenBrowser( APP_HOME_URL );
			}
			iCursor = 0;
			SetCursor( hCursor[ 0 ] );
			iMouseDown = 0;

			break;
		}


		case WM_LBUTTONDBLCLK:
		{
			ChangeSkin( hWnd, hMemDC, SkinSize );
			break;
		}

		case WM_RBUTTONDOWN:
		{
			HMENU hMenu = CreatePopupMenu();
			TCHAR str[] = TEXT( "Ski&n..." );
			AppendMenu( hMenu, MFT_STRING, IDM_SKIN, str );

			POINT pt;
			GetCursorPos( &pt );

			if( lParam == 0L ) // VK_APPS
			{
				pt.x = 0, pt.y = 0;
				ClientToScreen( hWnd, &pt );
			}
			
			TrackPopupMenu( hMenu, TPM_RIGHTBUTTON | TPM_TOPALIGN | TPM_LEFTALIGN,
					pt.x, pt.y, 0, hWnd, (LPRECT) NULL );

			break;
		}

		case WM_KILLFOCUS:
		{
			iCursor = 0;
			SetCursor( hCursor[ 0 ] );
			InvalidateRect( hWnd, &urlrect, 0 );
			break;
		}


		case WM_PAINT:
		{
			PAINTSTRUCT ps;

			HDC hdc = BeginPaint( hWnd, &ps );

			DrawSkin( hdc, hMemDC, SkinSize );
/*
			BLENDFUNCTION BlendFunc;
			BlendFunc.AlphaFormat = AC_SRC_OVER;
			BlendFunc.BlendFlags = 0;
			BlendFunc.BlendOp = 0;
			BlendFunc.SourceConstantAlpha = 0x66;
			AlphaBlend( hdc,
				0, 0, 640, 480,
				hMemDC,
				0, 0, 640, 480,
				BlendFunc
			);
*/

			HFONT hOldFont;
			
			if( lstrcmp( lpszStatus[ 0 ], APP_NAME ) == 0 )
			{
				hOldFont = (HFONT) SelectObject( hdc, hFontItalic );
#ifdef _UNICODE
				lstrcpy( lpszStatus[ 2 ], TEXT( "Unicode Build / " ) );

				if( IS_JAPANESE )
				{
					WCHAR str[ 1024 ];
					MultiByteToWideChar( CP_UTF8, MB_CUTE,
						IS_JAPANESEo ? S_JPNo_0001 : S_JPN_0001,
						-1, str, 1023 );
					lstrcat( lpszStatus[ 2 ], str );
				}
				else if( IS_FINNISH )
				{
					WCHAR str[ 1024 ];
					MultiByteToWideChar( CP_UTF8, MB_CUTE, S_FIN_0001, -1, str, 1023 );
					lstrcat( lpszStatus[ 2 ], str );
				}
				else if( IS_SPANISH )
				{
					WCHAR str[ 1024 ];
					MultiByteToWideChar( CP_UTF8, MB_CUTE, S_SPA_0001, -1, str, 1023 );
					lstrcat( lpszStatus[ 2 ], str );
				}
				else if( IS_CHINESE_T )
				{
					WCHAR str[ 1024 ];
					MultiByteToWideChar( CP_UTF8, MB_CUTE, S_CHI_0001T, -1, str, 1023 );
					lstrcat( lpszStatus[ 2 ], str );
				}
				else if( IS_CHINESE_S )
				{
					WCHAR str[ 1024 ];
					MultiByteToWideChar( CP_UTF8, MB_CUTE, S_CHI_0001S, -1, str, 1023 );
					lstrcat( lpszStatus[ 2 ], str );
				}
				else if( IS_FRENCH )
				{
					WCHAR str[ 1024 ];
					MultiByteToWideChar( CP_UTF8, MB_CUTE, S_FRE_0001, -1, str, 1023 );
					lstrcat( lpszStatus[ 2 ], str );
				}
				else
				{
					lstrcat( lpszStatus[ 2 ], L"English" );
				}
#else
				lstrcpy( lpszStatus[ 2 ], TEXT( "Non-Unicode Build" ) );
#endif
			}
			else
			{
				hOldFont = (HFONT) SelectObject( hdc, hFont );
			}
			
			SetTextColor( hdc, RGB( 0x99, 0xff, 0x66 ) );
			SetBkMode( hdc, TRANSPARENT );
			int x = 30;
			int y = 20;

			TextOut( hdc, x, y, lpszStatus[ 0 ], lstrlen( lpszStatus[ 0 ] ) );
			y += 20;

			SelectObject( hdc, hFont );

			for( int i = 1; i < 12; i++ )
			{
				if( i % 4 == 0 ) SetTextColor( hdc, RGB( 0x99, 0xff, 0x66 ) );
				else if( i % 4 == 3 ) SetTextColor( hdc, RGB( 0x66, 0xee, 0xee ) );
				else SetTextColor( hdc, RGB( 0xff, 0xff, 0x99 ) );
				TextOut( hdc, x, y, lpszStatus[ i ], lstrlen( lpszStatus[ i ] ) );
				y += 20;
				if( i % 4 == 3 ) y += 10;
			}
			TextOut( hdc, x + 20, y, lpszStatus[ 12 ], lstrlen( lpszStatus[ 12 ] ) );
			SetTextColor( hdc, RGB( 0xff, 0xff, 0xcc ) );
			x += 40;
			y += 20;
			TextOut( hdc, x, y + 20, lpszStatus[ 13 ], lstrlen( lpszStatus[ 13 ] ) );
			TextOut( hdc, x, y + 40, lpszStatus[ 14 ], lstrlen( lpszStatus[ 14 ] ) );
			TextOut( hdc, x, y + 60, lpszStatus[ 15 ], lstrlen( lpszStatus[ 15 ] ) );


			HFONT hFontURL = GetFontForURL( hdc );
			SelectObject( hdc, hFontURL );

			SetTextColor( hdc, iCursor == 0? RGB( 0x66, 0x99, 0x99 ) :
				iCursor == 1? RGB( 0x66, 0xff, 0xff ) : RGB( 0xff, 0xcc, 0x00 ) );
			TextOut( hdc, 425, 400, APP_HOME_URL, lstrlen( APP_HOME_URL ) );

			SelectObject( hdc, hOldFont );
			DeleteFont( hFontURL );

			EndPaint( hWnd, &ps );
			break;
		}

		
		case WM_KEYDOWN:
		{
			if( wParam == VK_ESCAPE	&& ! REPEATED_KEYDOWN( lParam ) )
			{
				UINT uMenuState = GetMenuState( GetMenu( hWnd ), IDM_EXIT, MF_BYCOMMAND );
				if( uMenuState == (UINT) - 1 ) // error
				{
					break;
				}
				else if( ( uMenuState & MF_GRAYED ) ) // can't exit because it's active
				{
					SendMessage( hWnd, WM_COMMAND, (WPARAM) IDM_STOP, 0L );
				}
				else
				{
					SendMessage( hWnd, WM_COMMAND, (WPARAM) IDM_EXIT, 0L );
				}
			}
			else if( wParam == VK_APPS && ! REPEATED_KEYDOWN( lParam ) )
			{
				SendMessage( hWnd, WM_RBUTTONDOWN, 0U, 0L );
				break;
			}
			return DefWindowProc( hWnd, message, wParam, lParam );
		}

		case WM_TIMER:
		{
			SetWindowText( hWnd, lpszWindowText );
			break;
		}

		
		case WM_CLOSE:
		{
			WriteDebugLog( TEXT( "WM_CLOSE" ) );
			if( IsActive() )
			{
				WriteDebugLog( TEXT( "EXIT_ANYWAY?" ) );

				// Just in case
				ShowWindow( hWnd, SW_SHOWDEFAULT );
				SetForegroundWindow( hWnd );

				TCHAR msg[ 1024 ] = TEXT( "BES is active. Exiting now is risky.\r\n\r\nExit anyway?" );
#ifdef _UNICODE
				if( IS_JAPANESE )
				{
					MultiByteToWideChar( CP_UTF8, MB_CUTE, S_JPN_EXIT_ANYWAY, -1, msg, 1023 );
				}
				else if( IS_FINNISH )
				{
					MultiByteToWideChar( CP_UTF8, MB_CUTE, S_FIN_EXIT_ANYWAY, -1, msg, 1023 );
				}
				else if( IS_SPANISH )
				{
					MultiByteToWideChar( CP_UTF8, MB_CUTE, S_SPA_EXIT_ANYWAY, -1, msg, 1023 );
				}
				else if( IS_CHINESE_T )
				{
					MultiByteToWideChar( CP_UTF8, MB_CUTE, S_CHI_EXIT_ANYWAY_T, -1, msg, 1023 );
				}
				else if( IS_CHINESE_S )
				{
					MultiByteToWideChar( CP_UTF8, MB_CUTE, S_CHI_EXIT_ANYWAY_S, -1, msg, 1023 );
				}
				else if( IS_FRENCH )
				{
					MultiByteToWideChar( CP_UTF8, MB_CUTE, S_FRE_EXIT_ANYWAY, -1, msg, 1023 );
				}
#endif
				if( IDNO == MessageBox( hWnd, 
					msg,
					APP_NAME, MB_ICONEXCLAMATION | MB_YESNO | MB_DEFBUTTON2 )
				)
				{
					break;
				}

				WriteDebugLog( TEXT( "User wants to exit anyway" ) );
			}

			SendMessage( hWnd, WM_COMMAND, (WPARAM) IDM_STOP, 0L );

			if(
				WaitForSingleObject( hSemaphore[ 0 ], 1000UL ) == WAIT_OBJECT_0 &&
				WaitForSingleObject( hSemaphore[ 1 ], 1000UL ) == WAIT_OBJECT_0 &&
				WaitForSingleObject( hSemaphore[ 2 ], 1000UL ) == WAIT_OBJECT_0 &&
				WaitForSingleObject( hSemaphore[ 3 ], 1000 ) == WAIT_OBJECT_0
			)
			{
				WriteDebugLog( TEXT( "[ All Semaphores OK ]" ) );
				for( int i = 0; i < 4; i++ )
				{
					ReleaseSemaphore( hSemaphore[ i ], 1L, (LPLONG) NULL );
				}
			}
			else
			{
				WriteDebugLog( TEXT( "[!] Semaphore Error" ) );
			}

			DestroyWindow( hWnd );
			break;
		}


		

		case WM_DESTROY:
		{
			WriteDebugLog( TEXT( "WM_DESTROY" ) );

			DeleteDC( hMemDC );

			KillTimer( hWnd, TIMER_ID_MAIN );

			NOTIFYICONDATA ni;
			InitNotifyIconData( hWnd, &ni, &ti[ 0 ] );
			Shell_NotifyIcon( NIM_DELETE, &ni );

			DeleteFont( hFont );
			DeleteFont( hFontItalic );

			for( int i = 0; i < 4; i++ ) 
			{
				if( hChildThread[ i ] ) CloseHandle( hChildThread[ i ] );
				if( hSemaphore[ i ] ) CloseHandle( hSemaphore[ i ] );
			}

			PostQuitMessage( 0 );
			
			break;
		}
		
		default:
		{
			if( message == uMsgTaskbarCreated )
			{
				NOTIFYICONDATA ni;
				InitNotifyIconData( hWnd, &ni, &ti[ 0 ] );
				Shell_NotifyIcon( NIM_ADD, &ni );
				UpdateStatus( hWnd );
				break;
			}
			return DefWindowProc( hWnd, message, wParam, lParam );
		}
   }
   return 0L;
}


