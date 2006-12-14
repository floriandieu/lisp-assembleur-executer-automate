#include "BattleEnc.h"

extern BYTE g_Slider[ 4 ];
extern BOOL g_bHack[ 4 ];
extern DWORD g_dwTargetProcessId[ 4 ];

int InitNotifyIconData( const HWND hWnd, NOTIFYICONDATA * lpni, TARGETINFO * ti )
{
	TCHAR str[ 1024 ];
	TCHAR tmpstr[ 1000 ];
	int i;

	int iDllVersion = GetShell32Version();

	DWORD dwSize = (DWORD) ( iDllVersion >= 5 ? sizeof( NOTIFYICONDATA ) : NOTIFYICONDATA_V1_SIZE );
	ZeroMemory( lpni, dwSize );
	lpni->cbSize = dwSize;
	lpni->hWnd = hWnd;
	lpni->uID = 0U;
	lpni->hIcon = (HICON) GetClassLong( hWnd, GCL_HICONSM );

	if( IsActive() )
	{
		lstrcpy( lpni->szTip, TEXT( "BES - Active" ) );

		int lines = 1;
		int maxlen = ( iDllVersion >= 5 ) ? 127 : 63 ;
		for( i = 0; i < 3; i++ )
		{
			if( g_bHack[ i ] ) lines++;
		}

		if( lines != 0 ) maxlen = maxlen / lines - 5;

		for( i = 0; i < 3; i++ )
		{
			if( g_bHack[ i ] )
			{
				lstrcpy( tmpstr, ti[ i ].szExe );

				TCHAR * pos2 = _tcsstr( tmpstr, TEXT( ".ex" ) );
				if( pos2 == NULL ) pos2 = _tcsstr( tmpstr, TEXT( ".EX" ) );
				if( pos2 == NULL ) pos2 = _tcsstr( tmpstr, TEXT( ".e" ) );
				if( pos2 == NULL ) pos2 = _tcsstr( tmpstr, TEXT( ".E" ) );

				if( pos2 != NULL )
				{
					pos2[ 0 ] = TEXT( '\0' );
				}
				if( lstrlen( tmpstr ) > maxlen )
				{
#ifdef _UNICODE
					WCHAR wc = tmpstr[ maxlen - 3 ];
					if( SURROGATE_LO( wc ) ) tmpstr[ maxlen - 4 ] = TEXT( '\0' );
					else tmpstr[ maxlen - 3 ] = TEXT( '\0' );
					lstrcat( tmpstr, L"\x2026" ); // 'HORIZONTAL ELLIPSIS' (U+2026)
#else
					tmpstr[ maxlen - 3 ] = TEXT( '\0' );
					lstrcat( tmpstr, "..." );
#endif
					
				}

				wsprintf( str, TEXT( "\r\n%s %d" ), tmpstr, g_Slider[ i ] );
				lstrcat( lpni->szTip, str );
			}
		}
	}
	else
	{
		lstrcpy( lpni->szTip, TEXT( "BES - Idle" ) );
	}

	if( g_bHack[ 3 ] )
	{
		if( g_dwTargetProcessId[ 2 ] == WATCHING_IDLE )
		{
			wsprintf( str, TEXT( "Watching %s...\r\n\r\n(Target not found)" ), ti[ 2 ].szExe );
		}
		else
		{
			wsprintf( str, TEXT( "Watching %s...\r\n\r\nTarget found!" ), ti[ 2 ].szExe );
		}
	}

	lstrcpy( lpni->szInfo, str );
	lpni->uTimeout = 10U * 1000U;
	lstrcpy( lpni->szInfoTitle, APP_NAME );
	lpni->dwInfoFlags = NIIF_INFO;

	lpni->uCallbackMessage = WM_USER_NOTIFYICON;
	lpni->uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;

	return iDllVersion;
}


