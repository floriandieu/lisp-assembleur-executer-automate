#include "BattleEnc.h"

extern DWORD g_dwTargetProcessId[ 4 ];
extern BYTE g_Slider[ 4 ];
extern HANDLE hSemaphore[ 4 ];
extern TCHAR g_szTarget[ 4 ][ 1024 ];
extern BOOL g_bHack[ 4 ];
extern BOOL g_bRealTime;

BOOL g_bBlock = TRUE;

#define MAX_INNER_LOOPS 100

DWORD WINAPI Hack( LPVOID lpVoid )
{
	TCHAR msg[ 4096 ];
	LPHACK_PARAMS lphp = (LPHACK_PARAMS) lpVoid;
	HWND hWnd = lphp -> myHwnd;
	const int iMyId = lphp -> iMyId;
	LPTARGETINFO lpTarget = lphp -> lpTarget;
	LPTSTR * lpszStatus = lphp -> lpszStatus;
	DWORD dwTargetProcessId = lpTarget -> dwProcessId;
	DWORD dwPrevSuspendCount;
	DWORD dwTotalChanges = 0;
	int iPrevThreadCount = 0;
	int errorflag = 0;

	const RECT minirect = { 20L, 20L + 90L * iMyId, 479L, 40L + 90L * iMyId };
	const RECT hackrect = { 20L, 20L + 90L * iMyId, 479L, 100L + 90L * iMyId };

	TCHAR lpszEnemyPath[ MAX_PATH * 2 ] = _T("");
	TCHAR lpszEnemyExe[ MAX_PATH * 2 ] = _T("");
	lstrcpy( lpszEnemyPath, lpTarget->szPath );
	lstrcpy( lpszEnemyExe, lpTarget->szExe );

	int i , j;

	HANDLE hProcess = NULL;
	DWORD dwOldPriority = NORMAL_PRIORITY_CLASS;
	int iOpenThreadRetry = 0;
	int iSuspendThreadRetry = 0;
	int iResumeThreadRetry = 0;
	SYSTEMTIME st;

	for( i = 0 + iMyId * 4; i < 4 + iMyId * 4; i++ )
	{
		lstrcpy( lpszStatus[ i ], TEXT( "" ) );
	}

	lstrcpy( lpszStatus[ 2 + iMyId * 4 ], lpszEnemyPath );

#if !defined( _UNICODE )
	// for watching...
	if( iMyId == 2 && lstrlen( lpszEnemyPath ) >= 19 )
	{
		lpszStatus[ 2 + iMyId * 4 ][ 15 ] = '\0';
		PathToExeEx( lpszStatus[ 2 + iMyId * 4 ], MAX_PATH * 2 );
	}
#endif

	AdjustLength( lpszStatus[ 2 + iMyId * 4 ] );
	
	for( DWORD dwOuterLoops = 0; 
		g_bHack[ iMyId ];
		dwOuterLoops++ )
	{
		if( dwOuterLoops == 0UL )
		{
			hProcess = OpenProcess(
				PROCESS_QUERY_INFORMATION | PROCESS_SET_INFORMATION ,
				FALSE,
				dwTargetProcessId
			);
			if( hProcess )
			{
				dwOldPriority = GetPriorityClass( hProcess );
				wsprintf( msg, TEXT( "Target #%d: \"%s\"\r\n\tProcess ID = 0x%08lX : hProcess = 0x%08lX (Priority: %lu)" ),
					iMyId + 1, lpszEnemyPath, dwTargetProcessId, hProcess, dwOldPriority
				);
				WriteDebugLog( msg );
				if( g_bRealTime )
				{
					WriteDebugLog( SetPriorityClass( hProcess, IDLE_PRIORITY_CLASS )?
						TEXT( "SetPriorityClass OK" ) : TEXT( "SetPriorityClass failed" ) );
				}
			}
			else
			{
				WriteDebugLog( TEXT( "OpenProcess failed." ) );

				GetLocalTime( &st );
				wsprintf( lpszStatus[ 1 + iMyId * 4 ], TEXT( "Process ID = n/a" ) );
				wsprintf( lpszStatus[ 3 + iMyId * 4 ], TEXT( "%02d:%02d:%02d Initial OpenThread failed" ),
					st.wHour, st.wMinute, st.wSecond
				);
				g_dwTargetProcessId[ iMyId ] = TARGET_PID_NOT_SET;

				// This is needed as a workaround
				UpdateStatus( hWnd );

				if( ReleaseSemaphore( hSemaphore[ iMyId ], 1L, (LPLONG) NULL ) )
				{
					wsprintf( msg, TEXT( "* ReleaseSemaphore #%d in Hack()" ), iMyId + 1 );
					WriteDebugLog( msg );
				}
				else
				{
					wsprintf( msg, TEXT( "[!] Target #%d : ReleaseSemaphore failed in Hack(): ErrCode %lu" ),
						iMyId + 1, GetLastError() );
					WriteDebugLog( msg );
				}
				lphp->lpTarget->dwProcessId = TARGET_PID_NOT_SET;


				if( PathToProcess( lpszEnemyPath ) == ( DWORD ) -1 )
				{
					SendMessageCallback( hWnd, WM_USER_STOP, (WPARAM) iMyId, (LPARAM) TARGET_MISSING,
						(SENDASYNCPROC) DummyCallback, (DWORD) hWnd );

					InvalidateRect( hWnd, NULL, TRUE );
					return TARGET_MISSING;
				}
				else
				{
					SendMessageCallback( hWnd, WM_USER_STOP, (WPARAM) iMyId, (LPARAM) THREAD_NOT_OPENED,
						(SENDASYNCPROC) DummyCallback, (DWORD) hWnd );

					
					InvalidateRect( hWnd, NULL, TRUE );
					return THREAD_NOT_OPENED;

				}

			}

			
			wsprintf( lpszStatus[ 0 + iMyId * 4 ],
				TEXT( "Target #%d [ -%d%% ]" ),
				iMyId + 1,
				g_Slider[ iMyId ]
			);
		
		} // endif( dwOuterLoops == 0 )



		DWORD dwThreadId[ MAX_THREAD_CNT ];
		int iThreadCount = ListProcessThreads( dwTargetProcessId, dwThreadId );


		// ----------------------- @@@
		if( iThreadCount == 0 )
		{
			GetLocalTime( &st );
			wsprintf( lpszStatus[ 1 + iMyId * 4 ], TEXT( "Process ID = n/a" ) );
			wsprintf( lpszStatus[ 3 + iMyId * 4 ], TEXT( "%02d:%02d:%02d No threads" ),
				st.wHour, st.wMinute, st.wSecond
			);
			g_dwTargetProcessId[ iMyId ] = TARGET_PID_NOT_SET;
			if( hProcess ) 
			{
				SetPriorityClass( hProcess, dwOldPriority );
				CloseHandle( hProcess );
			}

			if( ReleaseSemaphore( hSemaphore[ iMyId ], 1L, (LPLONG) NULL ) )
			{
				wsprintf( msg, TEXT( "* TARGET_MISSING #%d : ReleaseSemaphore in Hack()" ), iMyId + 1 );
				WriteDebugLog( msg );
			}
			else
			{
				wsprintf( msg, TEXT( "[!] Target #%d : ReleaseSemaphore failed in Hack(): ErrCode %lu" ),
					iMyId + 1, GetLastError() );
				WriteDebugLog( msg );
			}

			g_bBlock = TRUE;
			
			WriteDebugLog( TEXT("SendMessage Doing...") );

			SendMessageCallback( hWnd, WM_USER_STOP, (WPARAM) iMyId, (LPARAM) TARGET_MISSING,
				(SENDASYNCPROC) DummyCallback, (DWORD) hWnd );

			WriteDebugLog( TEXT("SendMessage Done!") );

			for( int z = 0; z < 10; z++ )
			{
				if( ! g_bBlock )
				{
					wsprintf( msg, TEXT("g_bBlock cleared at z=%d"), z );
					WriteDebugLog( msg );
					break;
				}
				Sleep( 10UL );
			}

			lphp->lpTarget->dwProcessId = TARGET_PID_NOT_SET;
// -@1.2.1 beta
// This is not needed because WM_USER_STOP will do the same,
// and this thread will die (so thie order wil be probably void) anyway
//			InvalidateRect( hWnd, NULL, TRUE );
			return TARGET_MISSING;
		}

		if( dwOuterLoops == 0UL )
		{
			iPrevThreadCount = iThreadCount;
			wsprintf( msg, TEXT( "Target #%d : iThreadCount = %3d" ), iMyId + 1, iThreadCount );
			WriteDebugLog( msg );
		}
		else if( dwOuterLoops % 50UL == 0UL )
		{
			wsprintf( msg, TEXT( "Target #%d : Loops %5d, ThreadCnt %3d (Prev %3d)" ), 
				iMyId + 1, dwOuterLoops, iThreadCount, iPrevThreadCount );
			WriteDebugLog( msg );
		}

		wsprintf( lpszStatus[ 1 + iMyId * 4 ], TEXT( "Process ID = 0x%04X %04X [ %d %s ]" ),
			HIWORD( dwTargetProcessId ),
			LOWORD( dwTargetProcessId ),
			iThreadCount,
			( iThreadCount > 1 ? TEXT( "Threads" ) : TEXT( "Thread" ) )
		);
		GetLocalTime( &st );
		if( iThreadCount != iPrevThreadCount || errorflag )
		{
			dwTotalChanges++;
			wsprintf( msg, TEXT( "Target #%d : iThreadCount = %3d (Prev = %d)" ), iMyId + 1, iThreadCount, iPrevThreadCount );
			WriteDebugLog( msg );
			iPrevThreadCount = iThreadCount;
			errorflag = 0;
			wsprintf( lpszStatus[ 3 + iMyId * 4 ], TEXT( "%02d:%02d:%02d Target Re-Locked On: OK" ),
				st.wHour, st.wMinute, st.wSecond
			);
		}
		else
		{
			wsprintf( lpszStatus[ 3 + iMyId * 4 ], TEXT( "%02d:%02d:%02d %s: OK" ),
				st.wHour, st.wMinute, st.wSecond,
				dwOuterLoops == 0 ? TEXT( "Started" ) : TEXT( "Calm" )
			);
		}

		HANDLE hTarget[ MAX_THREAD_CNT ];
		ZeroMemory( hTarget, sizeof( HANDLE ) * MAX_THREAD_CNT );
		int iOpenedThread = 0;
		for( i = 0; i < iThreadCount; i++ )
		{
			hTarget[ i ] = OpenThread( THREAD_SUSPEND_RESUME, FALSE, dwThreadId[ i ] );
			if( hTarget[ i ] == NULL )
			{
				wsprintf( msg, TEXT( "[!] Target #%d : OpenThread failed: Thread #%03d, ThreadId 0x%08lX" ),
					iMyId + 1, i + 1, dwThreadId[ i ] );
				WriteDebugLog( msg );
			}
			else
			{
				iOpenedThread++;
			}
		}

		// ###----------------
		if( iOpenedThread == 0
			||
			iOpenedThread != iThreadCount && iOpenThreadRetry > 10
			||
			iSuspendThreadRetry > 100
			||
			iResumeThreadRetry > 50			
		)
		{
			GetLocalTime( &st );
			if( iResumeThreadRetry > 50 )
			{
				wsprintf( lpszStatus[ 3 + iMyId * 4 ], TEXT( "%02d:%02d:%02d ResumeThread Error" ),
					st.wHour, st.wMinute, st.wSecond
				);
				// to do
				// 'unfreeze'
			}
			else if( iSuspendThreadRetry > 100 )
			{
				wsprintf( lpszStatus[ 3 + iMyId * 4 ], TEXT( "%02d:%02d:%02d SuspendThread Error" ),
					st.wHour, st.wMinute, st.wSecond
				);
			}
			else
			{
				wsprintf( lpszStatus[ 3 + iMyId * 4 ], TEXT( "%02d:%02d:%02d OpenThread Error" ),
					st.wHour, st.wMinute, st.wSecond
				);
			}

			WriteDebugLog( lpszStatus[ 3 + iMyId * 4 ] );
			WriteDebugLog( TEXT( "### Giving up... ###" ) );

			lphp->lpTarget->dwProcessId = TARGET_PID_NOT_SET; //?
			g_dwTargetProcessId[ iMyId ] = TARGET_PID_NOT_SET;

			if( hProcess ) 
			{
				SetPriorityClass( hProcess, dwOldPriority );
				CloseHandle( hProcess );
			}

			// This is needed as a workaround ? (Possibly this works as a workaround) 1.0 beta6
			UpdateStatus( hWnd );

			if( ReleaseSemaphore( hSemaphore[ iMyId ], 1L, NULL ) )
			{
				wsprintf( msg, TEXT( "* THREAD_NOT_OPENED #%d : ReleaseSemaphore in Hack()" ), iMyId + 1 );
				WriteDebugLog( msg );
			}

			BOOL bMissing = ( PathToProcess( lpszEnemyPath ) == ( DWORD ) -1 );
			WriteDebugLog( TEXT("SendMessage Doing...") );

			SendMessageCallback( hWnd, WM_USER_STOP, (WPARAM) iMyId,
				bMissing? TARGET_MISSING : THREAD_NOT_OPENED,
				(SENDASYNCPROC) DummyCallback, (DWORD) hWnd );

			WriteDebugLog( TEXT("SendMessage Done!") );

			lphp->lpTarget->dwProcessId = TARGET_PID_NOT_SET;

			InvalidateRect( hWnd, NULL, TRUE );

			return ( bMissing ) ? TARGET_MISSING : THREAD_NOT_OPENED ;
		}
		else if( iOpenedThread != iThreadCount )
		{
			for( j = 0; j < i; j++ ) if( hTarget[ j ] ) CloseHandle( hTarget[ j ] );
			iOpenThreadRetry++;
			wsprintf( msg, TEXT( "@ Couldn't open some threads: Retrying #%d..." ), iOpenThreadRetry );
			WriteDebugLog( msg );
			Sleep( 100UL );
			continue;
		}

		iOpenThreadRetry = 0;

		if( g_bHack[ 0 ] || g_bHack[ 1 ] || g_bHack[ 2 ] )
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
				
			if( g_bHack[ 2 ] && g_dwTargetProcessId[ 2 ] != (DWORD) -1 )
			{
				lstrcat( strStatus, TEXT( " #3" ) );
			}
		
			wsprintf( lpszStatus[ 12 ], TEXT( "%s" ), strStatus );
		}
		else
		{
			lstrcpy( lpszStatus[ 12 ], TEXT( "" ) );
		}



		InvalidateRect( hWnd, &hackrect, FALSE );




		for( DWORD dwInnerLoops = 1; dwInnerLoops <= MAX_INNER_LOOPS; dwInnerLoops++ )
		{
			BYTE TimeRed = g_Slider[ iMyId ];
			BYTE TimeGreen = (BYTE)( 100 - TimeRed );
			
			for( i = 0; i < iThreadCount; i++ )
			{
				if( ! hTarget[ i ] ) continue;

				dwPrevSuspendCount = SuspendThread( hTarget[ i ] );
				if( ( DWORD ) -1 == dwPrevSuspendCount )
				{
					errorflag = 1;
					wsprintf( msg,
						TEXT( "Target #%d : SuspendThread failed (Thread #%03d) : Retry=%d" ),
						iMyId + 1, i + 1, iSuspendThreadRetry );
					WriteDebugLog( msg );
				}
//				else if( dwPrevSuspendCount != 0UL )
//				{
//					WriteDebugLog( TEXT( "### dwPrevSuspendCount != 0UL" ) );
//					do
//					{
//						dwPrevSuspendCount = ResumeThread( hTarget[ i ] );
//					}
//					while( dwPrevSuspendCount > 1UL );
//				}
			}

			if( errorflag == 1 )
			{
				iSuspendThreadRetry++;
			}
			else
			{
				iSuspendThreadRetry = 0;
			}

			Sleep( (DWORD) TimeRed );
			for( i = 0; i < iThreadCount; i++ )
			{
				if( ! hTarget[ i ] ) continue;
				DWORD dwPrevSuspendCount = ResumeThread( hTarget[ i ] );
				if( ( DWORD ) -1 == dwPrevSuspendCount )
				{
					errorflag = 2;
					wsprintf( msg,
						TEXT( "Target #%d : ResumeThread failed: Thread #%03d, ThreadId 0x%08lX, iResumeThreadRetry=%d" ),
						iMyId + 1, i + 1, dwThreadId[ i ], iResumeThreadRetry );
					WriteDebugLog( msg );
				}
//
//				else if( dwPrevSuspendCount != 1UL )
//				{
//					WriteDebugLog( TEXT( "### dwPrevSuspendCount != 1UL" ) );
//					do
//					{
//						dwPrevSuspendCount = ResumeThread( hTarget[ i ] );
//					}
//					while( dwPrevSuspendCount > 1UL );
//				}

			}

			if( errorflag == 2 )
			{
				iResumeThreadRetry++;
			}
			else
			{
				iResumeThreadRetry = 0;
			}

			if( g_bHack[ iMyId ] == FALSE || errorflag )
			{
				break;
			}
			Sleep( (DWORD) TimeGreen );
		
		
			if( dwInnerLoops % 10UL == 0UL )
			{
				wsprintf( lpszStatus[ 0 + iMyId * 4 ],
					TEXT( "Target #%d [ -%d%% ] %s" ),
					iMyId + 1,
					g_Slider[ iMyId ],
					( dwInnerLoops % 20UL == 0UL )?
						_T( " " ) :
#ifdef _UNICODE
						L"\x25CF"
#else
						"*"
#endif
				);
				InvalidateRect( hWnd, &minirect, 0 );
			}
		
		}

		for( i = 0; i < iThreadCount; i++ )
		{
			if( hTarget[ i ] ) CloseHandle( hTarget[ i ] );
		}
	}

	wsprintf( lpszStatus[ 1 + iMyId * 4 ], TEXT( "* Unlimited *" ) );

	if( ReleaseSemaphore( hSemaphore[ iMyId ], 1L, (LONG *) NULL ) )
	{
		wsprintf( msg, TEXT( "* Target #%d : ReleaseSemaphore in Hack()" ), iMyId + 1 );
		WriteDebugLog( msg );
	}
	else
	{
		wsprintf( msg, TEXT( "[!] Target #%d : ReleaseSemaphore failed in Hack(): ErrCode %lu" ),
			iMyId + 1, GetLastError() );
		WriteDebugLog( msg );
	}

	if( hProcess ) 
	{
		WriteDebugLog( SetPriorityClass( hProcess, dwOldPriority )? TEXT( "Set dwOldPriority: OK" ) : TEXT( "Set dwOldPriority: Failed" ) );
		CloseHandle( hProcess );
	}

	lstrcpy( lpszStatus[ 3 + iMyId * 4 ], TEXT( "" ) );
	return NORMAL_TERMINATION;
}

VOID CALLBACK DummyCallback( HWND hWnd, UINT uMsg, DWORD dwData, LRESULT lResult )
{
	UNREFERENCED_PARAMETER( hWnd );
	UNREFERENCED_PARAMETER( uMsg );
	UNREFERENCED_PARAMETER( dwData );
	UNREFERENCED_PARAMETER( lResult );
}

inline BOOL SetSliderText( HWND hDlg, int iControlId, LONG lPercentage )
{
	if( lPercentage < 0L || lPercentage > 99L ) lPercentage = 33L;
	else if( lPercentage == 0L ) lPercentage = 1L;
	else if( lPercentage == 100L ) lPercentage = 99L;

	TCHAR wcstr[ 100 ];
#ifdef _UNICODE
	wsprintf( wcstr, TEXT( "\x2212 %2ld%%" ), lPercentage );
#else
	wsprintf( wcstr, TEXT( "- %2ld%%" ), lPercentage );
#endif
	return SetDlgItemText( hDlg, iControlId, wcstr );
}


inline BOOL SliderMoved( WPARAM wParam )
{
	switch( LOWORD( wParam ) )
	{
		case TB_TOP: case TB_BOTTOM: case TB_LINEUP: case TB_LINEDOWN: case TB_THUMBPOSITION: case TB_THUMBTRACK:
		case TB_PAGEUP: case TB_PAGEDOWN:
			return TRUE;
	}
	return FALSE;
}

inline BYTE GetSliderParam( LONG lSlider )
{
	if( lSlider < 0L || lSlider > 99L ) return (BYTE) 33;
	else if( lSlider == 0L ) return (BYTE) 1;
	else if( lSlider == 100L ) return (BYTE) 99;
	else return (BYTE) lSlider;
}



LRESULT CALLBACK Settings( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
#define TIMER_ID_S (UINT) 'S'

	static LONG lSlider[ 3 ];
	static int iPathId[ 3 ] = { IDC_EDIT_TARGET1, IDC_EDIT_TARGET2, IDC_EDIT_TARGET3 };
	static int iButtonId[ 3 ] = { IDC_BUTTON_STOP1, IDC_BUTTON_STOP2, IDC_BUTTON_STOP3 };
	static int iTextId[ 3 ] = { IDC_TEXT_TARGET1, IDC_TEXT_TARGET2, IDC_TEXT_TARGET3 };
	static int iSliderId[ 3 ] = { IDC_SLIDER1, IDC_SLIDER2, IDC_SLIDER3 };
	static HWND hWnd;
	static int iMyId;
	static LPTSTR * lpszStatus;
	static TCHAR lpszWindowText[ 1024 ];

	static HFONT hMyFont = NULL;


	switch (message)
	{
		case WM_INITDIALOG:
		{
			hWnd = ( ( LPHACK_PARAMS ) lParam ) -> myHwnd;
			iMyId =  ( ( LPHACK_PARAMS ) lParam ) -> iMyId;
			lpszStatus = ( ( LPHACK_PARAMS ) lParam ) -> lpszStatus;

			
			HDC hDC = GetDC( hDlg );
			
			hMyFont = MyCreateFont( hDC, TEXT( "Verdana" ), 12, TRUE, FALSE );

			

			ReleaseDC( hDlg, hDC );
		

#ifdef _UNICODE
			if( IS_JAPANESE )
			{
				
				MultiByteToWideChar( CP_UTF8, MB_CUTE,
					IS_JAPANESEo ? S_JPNo_1002 : S_JPN_1002,
					-1, lpszWindowText, 1023 );
				SetWindowText( hDlg, lpszWindowText );
			}
			else if( IS_FRENCH )
			{

				MultiByteToWideChar( CP_UTF8, MB_CUTE,
					S_FRE_1002,
					-1, lpszWindowText, 1023 );
				SetWindowText( hDlg, lpszWindowText );
			}
			else if( IS_SPANISH )
			{

				MultiByteToWideChar( CP_UTF8, MB_CUTE,
					S_SPA_1002,
					-1, lpszWindowText, 1023 );
				SetWindowText( hDlg, lpszWindowText );
			}
			else
#endif
			{
				lstrcpy( lpszWindowText, _T( "Limiter control" ) );
				SetWindowText( hDlg, lpszWindowText );
			}

			for( int i = 0; i < 3; i++ )
			{
				lSlider[ i ] = (long) g_Slider[ i ];
				SetDlgItemText( hDlg, iPathId[ i ], g_szTarget[ i ] );
				TCHAR tmpstr[ 100 ];
				wsprintf( tmpstr, TEXT( "%s #&%d" ),
						( ( g_bHack[ i ] )? TEXT( "Unlimit" ) : TEXT( "Limit" ) ),
						i + 1
				);
				SetDlgItemText( hDlg, iButtonId[ i ], tmpstr );
				EnableWindow( GetDlgItem( hDlg, iButtonId[ i ] ), ( g_dwTargetProcessId[ i ] != TARGET_PID_NOT_SET ) );
				EnableWindow( GetDlgItem( hDlg, iSliderId[ i ] ), ( g_dwTargetProcessId[ i ] != TARGET_PID_NOT_SET ) );
				EnableWindow( GetDlgItem( hDlg, iTextId[ i ] ), ( g_dwTargetProcessId[ i ] != TARGET_PID_NOT_SET ) );

				SendDlgItemMessage( hDlg, iSliderId[ i ], TBM_SETRANGE, TRUE, MAKELONG( 1, 99 ) );
				
				
				SendDlgItemMessage( hDlg, iSliderId[ i ], TBM_SETPOS,   TRUE, lSlider[ i ] );

				for( long lPos = 10L; lPos <= 90L; lPos += 10L )
				{
					SendDlgItemMessage( hDlg, iSliderId[ i ], TBM_SETTIC, 0U, lPos );
				}

				SendDlgItemMessage( hDlg, iTextId[ i ], WM_SETFONT, ( WPARAM ) hMyFont, 0L );
				SetSliderText( hDlg, iTextId[ i ] , lSlider[ i ] );
			}

			if( g_dwTargetProcessId[ 2 ] == (DWORD) -1 || g_bHack[ 3 ] )
			{
				SetDlgItemText( hDlg, iButtonId[ 2 ], TEXT( "(Watching)" ) );
				EnableWindow( GetDlgItem( hDlg, iButtonId[ 2 ] ), FALSE );
				EnableWindow( GetDlgItem( hDlg, iTextId[ 2 ] ), TRUE );
			}

			// Anti-Ukagaka
			SetTimer( hDlg, TIMER_ID_S, 500U, (TIMERPROC) NULL );
			break;
		}

		case WM_CTLCOLORSTATIC:
		{
			for( int i = 0; i < 3; i++ )
			{
				HWND hEdit = GetDlgItem( hDlg, iTextId[ i ] );
				if( (HWND) lParam == hEdit )
				{
					HDC hDC = (HDC) wParam;
					SetBkMode( hDC, TRANSPARENT );
					SetBkColor( hDC, GetSysColor( COLOR_3DFACE ) );
					SetTextColor( hDC, RGB( 0,0,0xaa ) );
					return (BOOL) (HBRUSH) GetSysColorBrush( COLOR_3DFACE );
				}
			}
			return 0L;
		}
		
		
		case WM_HSCROLL:
		{
			if( GetDlgItem( hDlg, IDC_SLIDER1 ) == (HWND) lParam && SliderMoved( wParam ) )
			{
				const RECT minirect = { 20L, 20L + 90L * 0L, 479L, 40L + 90L * 0L };
				lSlider[ 0 ] = SendDlgItemMessage( hDlg, IDC_SLIDER1, TBM_GETPOS, 0U, 0L );
				SetSliderText( hDlg, IDC_TEXT_TARGET1, lSlider[ 0 ] );
				g_Slider[ 0 ] = GetSliderParam( lSlider[ 0 ] );
				if( g_bHack[ 0 ] )
				{
					wsprintf( lpszStatus[ 0 ], TEXT( "Target #1 [ -%d%% ]" ),
						g_Slider[ 0 ] );
					InvalidateRect( hWnd, &minirect, 0 );
				}
			}
			else
			if( GetDlgItem( hDlg, IDC_SLIDER2 ) == (HWND) lParam && SliderMoved( wParam ) )
			{
				const RECT minirect = { 20L, 20L + 90L * 1L, 479L, 40L + 90L * 1L };
				lSlider[ 1 ] = SendDlgItemMessage( hDlg, IDC_SLIDER2, TBM_GETPOS, 0U, 0L );
				SetSliderText( hDlg, IDC_TEXT_TARGET2, lSlider[ 1 ] );
				g_Slider[ 1 ] = GetSliderParam( lSlider[ 1 ] );
				if( g_bHack[ 1 ] )
				{
					wsprintf( lpszStatus[ 0 + 4 * 1 ], TEXT( "Target #2 [ -%d%% ]" ),
						g_Slider[ 1 ] );
					InvalidateRect( hWnd, &minirect, 0 );
				}
			}
			else
			if( GetDlgItem( hDlg, IDC_SLIDER3 ) == (HWND) lParam && SliderMoved( wParam ) )
			{
				const RECT minirect = { 20L, 20L + 90L * 2L, 479L, 40L + 90L * 2L };
				lSlider[ 2 ] = SendDlgItemMessage( hDlg, IDC_SLIDER3, TBM_GETPOS, 0U, 0L );
				SetSliderText( hDlg, IDC_TEXT_TARGET3, lSlider[ 2 ] );
				g_Slider[ 2 ] = GetSliderParam( lSlider[ 2 ] );
				if( g_bHack[ 2 ] )
				{
					wsprintf( lpszStatus[ 0 + 4 * 2 ], TEXT( "Target #3 [ -%d%% ]" ),
						g_Slider[ 2 ] );
					InvalidateRect( hWnd, &minirect, 0 );
				}
			}
			break;
		}
		case WM_COMMAND:
		{
			switch( LOWORD( wParam ) )
			{
				case IDOK:
				{
					TCHAR msg[ 4096 ];
					int i;
					for( i = 0; i < 3; i++ )
					{
						if( g_dwTargetProcessId[ i ] != 0UL )
						{
							wsprintf( msg, TEXT( "g_Slider[ %d ] = %d : %s" ), i, (int) g_Slider[ i ], g_szTarget[ i ] );
							WriteDebugLog( msg );
							SetSliderIni( g_szTarget[ i ], (int) g_Slider[ i ] );
						}
					}

					EndDialog( hDlg, TRUE );
					break;
				}
				case IDCANCEL:
				{
					EndDialog( hDlg, FALSE );
					break;
				}
				case IDC_BUTTON_STOP1:
				{
					if( g_bHack[ 0 ] )
					{
						SendMessage( hWnd, WM_USER_STOP, 0U, 0L );
						SetDlgItemText( hDlg, iButtonId[ 0 ], TEXT( "Limit #&1" ) );
					}
					else
					{
						SendMessage( hWnd, WM_USER_RESTART, 0U, 0L );
						SetDlgItemText( hDlg, iButtonId[ 0 ], TEXT( "Unlimit #&1" ) );
					}
					break;
				}
				case IDC_BUTTON_STOP2:
				{
					if( g_bHack[ 1 ] )
					{
						SendMessage( hWnd, WM_USER_STOP, 1U, 0L );
						SetDlgItemText( hDlg, iButtonId[ 1 ], TEXT( "Limit #&2" ) );
					}
					else
					{
						SendMessage( hWnd, WM_USER_RESTART, 1U, 0L );
						SetDlgItemText( hDlg, iButtonId[ 1 ], TEXT( "Unlimit #&2" ) );
					}
					break;
				}
				case IDC_BUTTON_STOP3:
				{
					if( g_bHack[ 2 ] )
					{
						SendMessage( hWnd, WM_USER_STOP, 2U, 0L );
						SetDlgItemText( hDlg, iButtonId[ 2 ], TEXT( "Limit #&3" ) );
					}
					else
					{
						SendMessage( hWnd, WM_USER_RESTART, 2U, 0L );
						SetDlgItemText( hDlg, iButtonId[ 2 ], TEXT( "Unlimit #&3" ) );
					}
					break;
				}
				default:
				{
					break;
				}
			}
			break; // <-- fixed @ 1.1 beta3
		}

		case WM_TIMER:
		{
			SetWindowText( hDlg, lpszWindowText );
			break;
		}
		case WM_DESTROY:
		{
			DeleteFont( hMyFont );
			hMyFont = NULL;
			KillTimer( hDlg, TIMER_ID_S );
			
			break;
		}
		default:
			return 0L;//FALSE;
	}
    return 1L;//TRUE;
}


BOOL Unfreeze( HWND hWnd, LPTARGETINFO lpTarget )
{
	TCHAR msg[ 1024 * 100 ] = TEXT( "" );
	TCHAR str[ 1024 ];
	BOOL bSuccessful = TRUE;
	const int iThreadCount = (int) ( lpTarget->wThreadCount );
	wsprintf( msg, TEXT( "%d %hs:\r\n" ), iThreadCount,
		iThreadCount == 1 ? "thread" : "threads" );
	int iPreviousSuspendCount, iCurrentSuspendCount;
	int i, j;
	for( i = 0; i < iThreadCount; i++ )
	{
		HANDLE hThread = OpenThread( THREAD_SUSPEND_RESUME, FALSE, lpTarget->dwThreadId[ i ] );
		if( hThread )
		{
			iPreviousSuspendCount = SuspendThread( hThread );
			if( iPreviousSuspendCount == (DWORD) -1 )
			{
				wsprintf( str, TEXT( "Thread #%3d (0x%08lX) opened. ERROR while checking Suspend Count.\r\n" ),
					i + 1, lpTarget->dwThreadId[ i ] );
				lstrcat( msg, str );
				CloseHandle( hThread );
				continue;
			}
			wsprintf( str, TEXT( "Thread #%3d (0x%08lX) opened.\tPrevious suspend count = %d  \r\n" ),
				i + 1, lpTarget->dwThreadId[ i ], iPreviousSuspendCount );
			lstrcat( msg, str );

			iCurrentSuspendCount = iPreviousSuspendCount + 1;
			for( j = 0; j < iCurrentSuspendCount; j++ )
			{
				ResumeThread( hThread );
			}
			CloseHandle( hThread );
		}
		else
		{
			bSuccessful = FALSE;
			wsprintf( str, TEXT( "ERROR: Thread #%d (0x%08lX) not opened.\r\n" ), i + 1, lpTarget->dwThreadId[ i ] );
			lstrcat( msg, str );
			continue;
		}
	}

	MessageBox( hWnd, msg, TEXT( "Unfreezing : Status" ), MB_OK );
	return bSuccessful;
}





VOID AdjustLength( LPTSTR lptstr )
{
	int len = lstrlen( lptstr );
	if( len > 45 )
	{
		if( SURROGATE_LO( lptstr[ 10 ] ) )
		{
			//WriteDebugLog( _T("Surrogate @ Head End") );
			lptstr[ 11 ] = TEXT( '\0' );
		}
		else  lptstr[ 10 ] = TEXT( '\0' );

#ifdef _UNICODE
		lstrcat( lptstr, L"\x2026" ); // 'HORIZONTAL ELLIPSIS' (U+2026)
#else
		lstrcat( lptstr, "..." );
#endif

		if( SURROGATE_LO( lptstr[ len - 30 ] ) )
		{
			lstrcat( lptstr, &lptstr[ len - 31 ] );
		}
		else lstrcat( lptstr, &lptstr[ len - 30 ] );
	}
}

