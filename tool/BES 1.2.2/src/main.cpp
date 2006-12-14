#include "BattleEnc.h"

BOOL g_bRealTime = FALSE;
HHOOK g_hHook = NULL;

LRESULT CALLBACK HookProc( int nCode, WPARAM wParam, LPARAM lParam );

int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	UNREFERENCED_PARAMETER( hPrevInstance );
	UNREFERENCED_PARAMETER( lpCmdLine );

	HWND hwndFound = FindWindow( APP_CLASS, NULL );
	if( hwndFound )
	{
		if( IsIconic( hwndFound ) )
		{
			ShowWindow( hwndFound, SW_RESTORE );
		}
		SetForegroundWindow( hwndFound );
		return 0;
	}

	OleInitialize( NULL );

	TCHAR lptstrIniPath[ MAX_PATH * 2 ];
	GetIniPath( lptstrIniPath );
	g_bRealTime =
		!! GetPrivateProfileInt( TEXT( "Options" ), TEXT( "RealTime" ), FALSE, lptstrIniPath );

	if( g_bRealTime )
	{
		SetPriorityClass( GetCurrentProcess(), REALTIME_PRIORITY_CLASS );
		SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL );
	}
	else
	{
		SetPriorityClass( GetCurrentProcess(), HIGH_PRIORITY_CLASS );
		SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL );
	}

	ReadIni(); // Logging, IFF
	OpenDebugLog();
	InitSWIni();

	WNDCLASSEX wcex;
	ZeroMemory( &wcex, sizeof( WNDCLASSEX ) );
	wcex.cbSize = sizeof( WNDCLASSEX ); 
	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc	= (WNDPROC) WndProc;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon( hInstance, (LPCTSTR) IDI_IDLE );
	wcex.hIconSm		= wcex.hIcon;
	wcex.hCursor		= NULL;
	wcex.hbrBackground	= (HBRUSH) GetStockObject( NULL_BRUSH );
	
	wcex.lpszMenuName	= (LPCTSTR) IDC_BATTLEENC;
	wcex.lpszClassName	= APP_CLASS;

	RegisterClassEx( &wcex );

	POINT pt;
	GetWindowPosIni( &pt );

	HWND hWnd = CreateWindow( APP_CLASS, APP_NAME,
	  WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
	  pt.x, pt.y,
	  640 + GetSystemMetrics( SM_CXBORDER ) * 2,
	  480 + GetSystemMetrics( SM_CYMENU ) + GetSystemMetrics( SM_CYSIZE )
		+  + GetSystemMetrics( SM_CYBORDER ) * 2,
	  NULL, NULL, hInstance, (LPVOID) NULL );

	if ( ! hWnd ) return FALSE;
/*
	DirectSSTP( hWnd, "\\c\\s[2]BES?",
		"\\w8Battle Encode Shirase!" );
*/
	g_hHook = SetWindowsHookEx( WH_MSGFILTER, (HOOKPROC) HookProc,
		(HINSTANCE) NULL,
		GetCurrentThreadId()
	);



	TCHAR * p;
	if( ( p = _tcsstr( GetCommandLine(), _T( "--minimize" ) ) ) != NULL )
	{
		TCHAR c = *( p + lstrlen( _T( "--minimize" ) ) );
		if( c == _T( '\0' ) || _istspace( c ) )
		{
			WriteDebugLog( _T( "SW_HIDE" ) );
			nCmdShow = SW_HIDE;
		}
		else
		{
			WriteDebugLog( _T( "*** If you mean \"--minimize\", type so ***" ) );
		}
	}

	ShowWindow( hWnd, nCmdShow );
	UpdateWindow( hWnd );

	HACCEL hAccelTable = LoadAccelerators( hInstance, (LPCTSTR) IDC_BATTLEENC );

	MSG msg;
	while ( GetMessage( &msg, NULL, 0U, 0U ) )
	{
		if( msg.hwnd != hWnd )
		{
			if( msg.message == WM_LBUTTONUP )
			{
				SendMessage( hWnd, WM_USER_BUTTON, (WPARAM) msg.hwnd, 0L );
			}
		}
	
		if ( !TranslateAccelerator( msg.hwnd, hAccelTable, &msg ) ) 
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
	}

	WriteIni();

	CloseDebugLog();


	OleUninitialize();

	if( g_hHook != NULL ) UnhookWindowsHookEx( g_hHook );

	return (int) msg.wParam;
}


