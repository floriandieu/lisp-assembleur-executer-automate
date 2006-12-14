#include "BattleEnc.h"

extern BOOL g_bRealTime;
extern BOOL g_bHack[ 4 ];
extern DWORD g_dwTargetProcessId[ 4 ];
extern HFONT hFont;

/*
HFONT UpdateFont( HFONT hFont )
{
	if( hFont ) DeleteFont( hFont );

	if( IS_CHINESE )
	{
		return MyCreateFont( TEXT( "Tahoma" ), 17, TRUE, FALSE );
	}
	
	HFONT h = NULL;
	h = MyCreateFont( TEXT( "Lucida Sans Unicode" ), 17, TRUE, FALSE );
	if( ! h ) h = MyCreateFont( TEXT( "Arial Unicode MS" ), 20, TRUE, FALSE );
	if( ! h && IsWindowsXPOrLater() ) hFont = MyCreateFont( TEXT( "Verdana" ), 16, TRUE, FALSE );
	return (HFONT) h;
}
*/



int MainWindowUrlHit( LPARAM lParam )
{
	int x = (int) LOWORD( lParam );
	int y = (int) HIWORD( lParam );
	return ( x > 424 && x < 619 && y > 401 && y < 418 );
}

inline int AboutBoxUrlHit( LPARAM lParam )
{
	int x = (int) LOWORD( lParam );
	int y = (int) HIWORD( lParam );
	return ( x > 98 && x < 294 && y > 148 && y < 166 );
}

LRESULT CALLBACK About( HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam )
{
#define TIMER_ID_ABOUT (UINT) 'A'
	const RECT urlrect = { 90, 140, 320, 170 };

	static HCURSOR hCursor[ 3 ];
	static int iCursor = 0;
	static int iMouseDown = 0;
	static HWND hWnd = NULL;

	static HFONT hBold;
	static TCHAR lpszWindowText[ 1024 ];
	switch ( uMessage )
	{
		case WM_INITDIALOG:
		{
			hWnd = (HWND) lParam;

			
			HDC hDC = GetDC( hDlg );
			hBold = MyCreateFont( hDC, TEXT( "Georgia" ), 14, TRUE, TRUE );
			if( !hBold ) hBold = MyCreateFont( hDC, TEXT( "Times New Roman" ), 16, TRUE, TRUE );
			ReleaseDC( hDlg, hDC );

			SetDlgItemText( hDlg, IDC_APP_NAME , APP_NAME );
			SendDlgItemMessage( hDlg, IDC_APP_NAME, WM_SETFONT, ( WPARAM ) hBold, 0L );
			SendDlgItemMessage( hDlg, IDC_DESCRIPTION, WM_SETFONT, ( WPARAM ) hFont, 0L );
			TCHAR str[ 1024 ];
			wsprintf( str, TEXT( "Compiled on: %hs, %hs UTC" ), __TIME__, __DATE__ );
			SetDlgItemText( hDlg, IDC_DATETIME , str );
			SetDlgItemText( hDlg, IDC_COPYRIGHT , APP_COPYRIGHT );

			SendDlgItemMessage( hDlg, IDC_EDITBOX_GPL, EM_SETMARGINS,
				(WPARAM) ( EC_LEFTMARGIN | EC_RIGHTMARGIN ),
				MAKELPARAM( 5 , 10 )
			);


#ifdef _UNICODE
			SetDlgItemText( hDlg, IDC_DESCRIPTION2 , TEXT( "Unicode Build" ) );
#else
			SetDlgItemText( hDlg, IDC_DESCRIPTION2 , TEXT( "ANSI (Non-unicode) Build" ) );
#endif
			
#ifdef _UNICODE
			if( IS_JAPANESE )
			{
				lpszWindowText[ 0 ] = TEXT( '\0' ); // just in case
				MultiByteToWideChar( CP_UTF8, MB_CUTE, S_JPN_9000, -1, lpszWindowText, 1023 );
				SetWindowText( hDlg, lpszWindowText );

				str[ 0 ] = TEXT( '\0' );
				MultiByteToWideChar( CP_UTF8, MB_CUTE, S_JPN_DESC, -1, str, 1023 );
				SetDlgItemText( hDlg, IDC_DESCRIPTION, str );

				SendDlgItemMessage( hDlg, IDC_EDITBOX_GPL, WM_SETFONT, ( WPARAM ) hFont, 0L );
				str[ 0 ] = TEXT( '\0' );
				MultiByteToWideChar( CP_UTF8, MB_CUTE, S_JPN_GPL, -1, str, 1023 );
				SetDlgItemText( hDlg, IDC_EDITBOX_GPL, str );
			}
			else if( IS_FINNISH )
			{
				lpszWindowText[ 0 ] = TEXT( '\0' ); // just in case
				MultiByteToWideChar( CP_UTF8, MB_CUTE, S_FIN_9000, -1, lpszWindowText, 1023 );
				SetWindowText( hDlg, lpszWindowText );

				str[ 0 ] = TEXT( '\0' );
				MultiByteToWideChar( CP_UTF8, MB_CUTE, S_FIN_DESC, -1, str, 1023 );
				SetDlgItemText( hDlg, IDC_DESCRIPTION, str );
				SetDlgItemText( hDlg, IDC_EDITBOX_GPL, TEXT_GPL );
			}
			else if( IS_SPANISH )
			{
				lpszWindowText[ 0 ] = TEXT( '\0' ); // just in case
				MultiByteToWideChar( CP_UTF8, MB_CUTE, S_SPA_9000, -1, lpszWindowText, 1023 );
				SetWindowText( hDlg, lpszWindowText );

				str[ 0 ] = TEXT( '\0' );
				MultiByteToWideChar( CP_UTF8, MB_CUTE, S_SPA_DESC, -1, str, 1023 );
				SetDlgItemText( hDlg, IDC_DESCRIPTION, str );
				SetDlgItemText( hDlg, IDC_EDITBOX_GPL, TEXT_GPL );
			}
			else if( IS_CHINESE_T )
			{
				lpszWindowText[ 0 ] = TEXT( '\0' ); // just in case
				MultiByteToWideChar( CP_UTF8, MB_CUTE, S_CHI_9000T, -1, lpszWindowText, 1023 );
				SetWindowText( hDlg, lpszWindowText );

				str[ 0 ] = TEXT( '\0' );
				MultiByteToWideChar( CP_UTF8, MB_CUTE, S_CHI_DESC_T, -1, str, 1023 );
				SetDlgItemText( hDlg, IDC_DESCRIPTION, str );
				SetDlgItemText( hDlg, IDC_EDITBOX_GPL, TEXT_GPL );
			}
			else if( IS_CHINESE_S )
			{
				lpszWindowText[ 0 ] = TEXT( '\0' ); // just in case
				MultiByteToWideChar( CP_UTF8, MB_CUTE, S_CHI_9000S, -1, lpszWindowText, 1023 );
				SetWindowText( hDlg, lpszWindowText );

				str[ 0 ] = TEXT( '\0' );
				MultiByteToWideChar( CP_UTF8, MB_CUTE, S_CHI_DESC_S, -1, str, 1023 );
				SetDlgItemText( hDlg, IDC_DESCRIPTION, str );
				SetDlgItemText( hDlg, IDC_EDITBOX_GPL, TEXT_GPL );
			}
			else if( IS_FRENCH )
			{
				lpszWindowText[ 0 ] = TEXT( '\0' ); // just in case
				MultiByteToWideChar( CP_UTF8, MB_CUTE, S_FRE_9000, -1, lpszWindowText, 1023 );
				SetWindowText( hDlg, lpszWindowText );

				str[ 0 ] = TEXT( '\0' );
				MultiByteToWideChar( CP_UTF8, MB_CUTE, S_FRE_DESC, -1, str, 1023 );
				SetDlgItemText( hDlg, IDC_DESCRIPTION, str );
				SetDlgItemText( hDlg, IDC_EDITBOX_GPL, TEXT_GPL );
			}
			else
#endif
			{
				SetDlgItemText( hDlg, IDC_DESCRIPTION, S_ENG_DESC );
				SetDlgItemText( hDlg, IDC_EDITBOX_GPL, TEXT_GPL );
				lstrcpy( lpszWindowText, TEXT( "About..." ) );
				SetWindowText( hDlg, lpszWindowText );
			}

			char hcSstpVersion[ 1024 ] = "";

#ifdef _UNICODE
			WideCharToMultiByte(
					CP_ACP,
					0,
					APP_NAME,
					-1,
					hcSstpVersion,
					1023,
					NULL,
					NULL
			);
#else
			lstrcpyA( hcSstpVersion, APP_NAME );
#endif
			lstrcatA( hcSstpVersion, "\\nCompiled on: " );
			lstrcatA( hcSstpVersion, __TIME__ );
			lstrcatA( hcSstpVersion, "+00:00, " );
			lstrcatA( hcSstpVersion, __DATE__ );

#ifdef _UNICODE
			if( IS_JAPANESE && ( LANG_JAPANESE == PRIMARYLANGID( GetSystemDefaultLangID() ) ) )
#else
			if( LANG_JAPANESE == PRIMARYLANGID( GetSystemDefaultLangID() ) )
#endif
			{
				DirectSSTP( (HWND) lParam, S_ABOUT_SJIS, hcSstpVersion, "Shift_JIS" );
			}
			else
			{
				DirectSSTP( (HWND) lParam, "About 'Battle Encoder Shirase'...", hcSstpVersion, "ASCII" );
			}


			hCursor[ 0 ] = LoadCursor( ( HINSTANCE ) NULL, IDC_ARROW );
			hCursor[ 1 ] = LoadCursor( ( HINSTANCE ) NULL, IDC_HAND );
			hCursor[ 2 ] = hCursor[ 1 ];
			SetCursor( hCursor[ 0 ] );

			// Anti-Ukagaka
			SetTimer( hDlg, TIMER_ID_ABOUT, 500U, (TIMERPROC) NULL );
			break;
		}
		case WM_COMMAND:
		{
			if( LOWORD( wParam ) == IDOK || LOWORD( wParam ) == IDCANCEL ) 
			{
				EndDialog( hDlg, -1 );
				return TRUE;
			}
			break;
		}
		case WM_TIMER:
		{
			SetWindowText( hDlg, lpszWindowText );
			break;
		}
		case WM_MOUSEMOVE:
		{
			if( iMouseDown == -1 ) break;

			int iPrevCursor = iCursor;
			iCursor = AboutBoxUrlHit( lParam );

			// Reset Cursor for each WM_MOUSEMOVE, silly but...
			SetCursor( hCursor[ iCursor ] );

			if( iCursor == 1 && iMouseDown == 1 ) iCursor = 2;

			
			if( iPrevCursor != iCursor )
			{
/*
				// In theory, we have to change Cursor only when Prev!=Now, but
				// something is wrong here and that doesn't work.
				SetCursor( hCursor[ iCursor ] );
*/
				InvalidateRect( hDlg, &urlrect, TRUE );
			}
			break;
		}

		case WM_LBUTTONDOWN:
		{
			SetCapture( hDlg );
			
			if( AboutBoxUrlHit( lParam ) )
			{
				iMouseDown = 1;
				SetCursor( hCursor[ 1 ] );
				iCursor = 2;
				InvalidateRect( hDlg, &urlrect, TRUE );
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
			
			if( iMouseDown == 1 && AboutBoxUrlHit( lParam ) )
			{
				OpenBrowser( APP_HOME_URL );
			}

			iCursor = 0;
			SetCursor( hCursor[ 0 ] );
			iMouseDown = 0;
			break;
		}


		case WM_ACTIVATE:
		{
			if( wParam == WA_INACTIVE )
			{
				iMouseDown = -1;
				iCursor = 0;
				SetCursor( hCursor[ 0 ] );
				InvalidateRect( hDlg, &urlrect, TRUE );
			}
			else
			{
				iMouseDown = 0;
			}
			break;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint( hDlg, &ps );
			HFONT hMyFont = GetFontForURL( hdc );
			HFONT hOldFont = (HFONT) SelectObject( hdc, hMyFont );

			SetBkMode( hdc, TRANSPARENT );
			SetTextColor( hdc,
				iCursor == 2 ? RGB( 0xff, 0x00, 0x00 ) :
				iCursor == 1 ? RGB( 0x00, 0x80, 0x00 ) : RGB( 0x00, 0x00, 0xff ) );
			TextOut( hdc, 100, 148, APP_HOME_URL, lstrlen( APP_HOME_URL ) );
			SelectObject( hdc, hOldFont );
			DeleteFont( hMyFont );
			EndPaint( hDlg, &ps );
			break;
		}

		case WM_CTLCOLORSTATIC:
		{
			if( (HWND) lParam == GetDlgItem( hDlg, IDC_APP_NAME ) )
			{
				HDC hDC = (HDC) wParam;
				SetBkMode( hDC, TRANSPARENT );
				SetBkColor( hDC, GetSysColor( COLOR_3DFACE ) );
				SetTextColor( hDC, RGB( 0,0,0xa0 ) );
				return (BOOL) (HBRUSH) GetSysColorBrush( COLOR_3DFACE );
			}
			else return FALSE;
			break;
		}
		case WM_DESTROY:
		{
			DeleteFont( hBold );
			hBold = NULL;
			KillTimer( hDlg, TIMER_ID_ABOUT );
			break;
		}
		default:
		{
			return 0L;
		}
	}
    return 1L;
}
#undef TIMER_ID_ABOUT



int GetArgument( LPCTSTR lpszCmdLine, int iBufferSize, LPTSTR lpszMyPath,
				LPTSTR lpszTargetLongPath, LPTSTR lpszTargetLongExe )
{
	if( iBufferSize > MAX_PATH * 2 ) iBufferSize = MAX_PATH * 2;

	TCHAR lpszTargetPath[ MAX_PATH * 2 ] = _T( "" );

	TCHAR p[ 1024 ];
	lstrcpy( p, lpszCmdLine );
	int s = 0;
	while( p[ s ] == TEXT( ' ' ) )
	{
		s++;
	}

	TCHAR * p1;
	TCHAR * p2;
	if( p[ s ] == TEXT( '\"' ) )
	{
		if( lstrlen( &p[ s + 1 ] ) >= iBufferSize ) return IGNORE_ARGV;
		else lstrcpy( lpszMyPath, &p[ s + 1 ] );

		p1 = _tcschr( lpszMyPath, TEXT( '\"' ) );
		
		if( !p1 ) return IGNORE_ARGV;
		else p1[ 0 ] = TEXT( '\0' );

		if( p1[ 1 ] != TEXT( ' ' ) ) return IGNORE_ARGV;
		else p2 = &p1[ 2 ];

	}
	else
	{
		if( lstrlen( &p[ s ] ) >= iBufferSize ) return IGNORE_ARGV;
		else lstrcpy( lpszMyPath, &p[ s ] );
		p1 = _tcschr( lpszMyPath, TEXT( ' ' ) );
		if( !p1 ) return IGNORE_ARGV;
		else p1[ 0 ] = TEXT( '\0' );
		p2 = &p1[ 1 ];
	}

	WriteDebugLog( lpszMyPath );

	s = 0;
	while( p2[ s ] == TEXT( ' ' ) )
	{
		s++;
	}

	if( p2[ s ] == TEXT( '\"' ) )
	{
		lstrcpy( lpszTargetPath, &p2[ s + 1 ] );
		p1 = _tcschr( lpszTargetPath, TEXT( '\"' ) );
		
		if( !p1 ) return IGNORE_ARGV;
		else p1[ 0 ] = TEXT( '\0' );
		if( p1[ 1 ] == TEXT( ' ' ) )
		{
			p2 = &p1[ 2 ];
		}
		else
		{
			p2 = p1; // TEXT( '\0' )
		}
	}
	else
	{
		lstrcpy( lpszTargetPath, &p2[ s ] );
		p1 = _tcschr( lpszTargetPath, TEXT( ' ' ) );
		if( p1 )
		{
			p1[ 0 ] = TEXT( '\0' );
			p2 = &p1[ 1 ];
		}
		else
		{
			p2 = p1; // TEXT( '\0' )
		}
	}

	int len = lstrlen( lpszTargetPath );
	if( len < 5 ) return IGNORE_ARGV;

//	if( lstrcmpi( &lpszTargetPath[ len - 4 ], TEXT( ".exe" ) ) != 0 ) return IGNORE_ARGV;

	WriteDebugLog( lpszTargetPath );

	DWORD dwResult = 
		GetLongPathName( lpszTargetPath, lpszTargetLongPath, (DWORD) iBufferSize );
	if( dwResult == 0UL || dwResult > (DWORD) iBufferSize )
	{
		lstrcpy( lpszTargetLongPath, lpszTargetPath );
	}

	WriteDebugLog( lpszTargetLongPath );

	PathToExe( lpszTargetLongPath, lpszTargetLongExe, iBufferSize );

	WriteDebugLog( lpszTargetLongExe );

#if !defined( _UNICODE )
	lstrcpy( lpszTargetLongPath, lpszTargetLongExe );
#endif

	int iSlider = _ttoi( p2 );

	
	if( iSlider < 0 || iSlider > 99 )
	{
		iSlider = 0;
	}


	WriteDebugLog( p2 );


	return iSlider;
}


VOID Exit_CommandFromTaskbar( const HWND hWnd )
{
	ShowWindow( hWnd, SW_HIDE );
	
	NOTIFYICONDATA ni;
	DWORD dwSize = (DWORD) ( GetShell32Version() >= 5 ? sizeof( NOTIFYICONDATA ) : NOTIFYICONDATA_V1_SIZE );
	ZeroMemory( &ni, dwSize );
	ni.cbSize = sizeof( dwSize );
	ni.hWnd = hWnd;
	Shell_NotifyIcon( NIM_DELETE, &ni );

	SendMessage( hWnd, WM_COMMAND, IDM_EXIT_ANYWAY, 0L );
}

VOID SetRealTimeMode( const HWND hWnd, const BOOL bRealTime, const HANDLE * phChildThread, LPTSTR lpszWindowText )
{
//	int i;
	UNREFERENCED_PARAMETER( phChildThread );
//
	if( bRealTime )
	{
		g_bRealTime = TRUE;
		SetPriorityClass( GetCurrentProcess(), REALTIME_PRIORITY_CLASS );
//		SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL );
		CheckMenuItem( GetMenu( hWnd ), IDM_REALTIME, MF_BYCOMMAND | MFS_CHECKED );
/*
		for( i = 0; i < 4; i++ )
		{
			if( g_bHack[ i ] && phChildThread[ i ] != NULL )
			{
				SetThreadPriority( phChildThread[ i ], THREAD_PRIORITY_HIGHEST );
			}
		}
*/
		WriteDebugLog( TEXT( "Real-time mode: Yes" ) );
	}
	else
	{
		g_bRealTime = FALSE;
		SetPriorityClass( GetCurrentProcess(), HIGH_PRIORITY_CLASS );
//		SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_NORMAL );
		CheckMenuItem( GetMenu( hWnd ), IDM_REALTIME, MF_BYCOMMAND | MFS_UNCHECKED );
/*
		for( i = 0; i < 4; i++ )
		{
			if( g_bHack[ i ] && phChildThread[ i ] != NULL )
			{
				SetThreadPriority( phChildThread[ i ], THREAD_PRIORITY_NORMAL );
			}
		}
*/
		WriteDebugLog( TEXT( "Real-time mode: No" ) );
	}

	if( lpszWindowText != NULL )
	{
		wsprintf( lpszWindowText, TEXT( "%s - %s%s" ),
			APP_NAME,
			IsActive() ? TEXT( "Active" ) : TEXT( "Idle" ),
			g_bRealTime ? TEXT( " (Real-time mode)" ) : TEXT( "" )
		);
		SetWindowText( hWnd, lpszWindowText );
	}


}

VOID AboutShortcuts( HWND hWnd )
{
	TCHAR lpszMessage[ 1024 * 2 ] = TEXT( "" );
	TCHAR lpszTitle[ 1024 ] = TEXT( "" );

	lstrcat( lpszMessage,
		TEXT( "[Enter]\t\t: Select Target\r\n" )
	);
	lstrcat( lpszMessage,
		TEXT( "[T]\t\t: Select Target\r\n" )
	);
	lstrcat( lpszMessage,
		TEXT( "[Ins]\t\t: Select Target\r\n\r\n" )
	);
	lstrcat( lpszMessage,
		TEXT( "[W]\t\t: Watch\r\n\r\n" )
	);
	lstrcat( lpszMessage,
		TEXT( "[C]\t\t: Control Limiter(s)\r\n\r\n" )
	);

	lstrcat( lpszMessage,
		TEXT( "[Esc]\t\t: Unlimit All (when active)\r\n" )
	);
	lstrcat( lpszMessage,
		TEXT( "[U]\t\t: Unlimit All (when active)\r\n" )
	);
	lstrcat( lpszMessage,
		TEXT( "[Delete]\t\t: Unlimit All (when active)\r\n\r\n" )
	);

	lstrcat( lpszMessage,
		TEXT( "[Esc]\t\t: Exit (when idle)\r\n" )
	);
	lstrcat( lpszMessage,
		TEXT( "[X]\t\t: Exit (when idle)\r\n" )
	);
	lstrcat( lpszMessage,
		TEXT( "[A]\t\t: About...\r\n" )
	);
	lstrcat( lpszMessage,
		TEXT( "[F1]\t\t: This Help\r\n\r\n" )
	);
extern HHOOK g_hHook;
	if( g_hHook != NULL )
	{
		lstrcat( lpszMessage,
			TEXT( "[F5]\t\t: Refresh the List (when Target Dlg is open)\r\n\r\n" )
		);
	}

	wsprintf( lpszTitle, TEXT( "Keyborad Shortcuts - %s" ), APP_NAME );

	MessageBox(
		hWnd,
		lpszMessage,
		lpszTitle,
		MB_OK | MB_ICONINFORMATION
	);
}




VOID GShow( HWND hWnd )
{
	TCHAR lpTargetTitle[ 1024 ] = TEXT( "" );
	
	if( GetTargetTitleIni( lpTargetTitle, 1023 ) )
	{
		HWND hwndTarget = FindWindow( NULL, lpTargetTitle );
		if( hwndTarget && hwndTarget != hWnd )
		{
			if( IsIconic( hwndTarget ) )
			{
				ShowWindow( hwndTarget, SW_RESTORE );
				SetForegroundWindow( hWnd );
			}

			if( IsWindowVisible( hwndTarget ) )
			{
				ShowWindow( hwndTarget, SW_HIDE );
			}
			else
			{
				ShowWindow( hwndTarget, SW_RESTORE );
				BringWindowToTop( hwndTarget );
				SetForegroundWindow( hWnd );
			}
		}
	}
}

BOOL CheckLanguageMenuRadio( const HWND hWnd )
{
	return CheckMenuRadioItem( GetMenu( hWnd ), IDM_CHINESE_S, IDM_JAPANESEo,
				( GetLanguage() == LANG_CHINESE_T )? IDM_CHINESE_T :
				( GetLanguage() == LANG_CHINESE_S )? IDM_CHINESE_S :
				( GetLanguage() == LANG_JAPANESE )? IDM_JAPANESE :
				( GetLanguage() == LANG_JAPANESEo )? IDM_JAPANESEo :
				( GetLanguage() == LANG_SPANISH )? IDM_SPANISH :
				( GetLanguage() == LANG_FRENCH )? IDM_FRENCH :
				( GetLanguage() == LANG_FINNISH )? IDM_FINNISH : IDM_ENGLISH,
				MF_BYCOMMAND
	);
}