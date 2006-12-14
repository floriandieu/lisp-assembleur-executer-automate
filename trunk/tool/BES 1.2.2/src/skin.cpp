#include "BattleEnc.h"

BOOL HandleToMemDC( HANDLE hFile, HDC hMemDC, SIZE& PicSize );

BOOL LoadSkin( HWND hWnd, HDC& hMemDC, SIZE& PicSize )
{
	HDC hdc = GetDC( hWnd );
	hMemDC = CreateCompatibleDC( hdc );
	//SetStretchBltMode( hMemDC, HALFTONE );
	//SetBrushOrgEx( hMemDC, 0, 0, NULL );

	int iMaxX = GetSystemMetrics( SM_CXSCREEN );
	int iMaxY = iMaxX * 480 / 640;

	HBITMAP hBmp = CreateCompatibleBitmap( hdc, iMaxX, iMaxY );
	SelectObject( hMemDC, hBmp );
	
	HBRUSH hBrush = CreateSolidBrush( RGB( 0x00, 0x22, 0x44 ) );
	HBRUSH hOldBrush = SelectBrush( hMemDC, hBrush );
	Rectangle( hMemDC, 0, 0, iMaxX, iMaxY );
	SelectObject( hMemDC, hOldBrush );
	DeleteBrush( hBrush );

	DeleteBitmap( hBmp );
	ReleaseDC( hWnd, hdc );


	TCHAR szDir[ MAX_PATH + 1 ] = TEXT("");
	TCHAR szFile[ MAX_PATH + 1 ] = TEXT("");

	TCHAR szIniPath[ MAX_PATH * 2 ];
	GetIniPath( szIniPath );
	GetPrivateProfileString(
		TEXT( "Options" ),
		TEXT( "Skin" ),
		NULL,
		szFile,
		MAX_PATH,
		szIniPath
	);

	HANDLE hFile = INVALID_HANDLE_VALUE;

	// Skin is set in .ini
	if( szFile != NULL )
	{
		// Load the custom skin
		hFile = CreateFile(
			szFile,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);
	}

	// If the custom skin not loaded...
	if( hFile == INVALID_HANDLE_VALUE )
	{
		GetModuleFileName( NULL, szDir, MAX_PATH + 1UL );
		int len = lstrlen( szDir );
		for( int i = len - 1; i >= 0; i-- )
		{
			if( szDir[ i ] == TEXT( '\\' ) )
			{
				szDir[ i ] = TEXT( '\0' );
				break;
			}
		}

		if( szDir[ 0 ] == TEXT( '\0' ) )
		{
			GetCurrentDirectory( MAX_PATH, szDir );
		}

		lstrcpy( szFile, szDir );
		lstrcat( szFile, TEXT( "\\skin.jpg" ) );
		hFile = CreateFile(
			szFile,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);
	}

	if( hFile == INVALID_HANDLE_VALUE )
	{
		lstrcpy( szFile, szDir );
		lstrcat( szFile, TEXT( "\\skin.bmp" ) );
		hFile = CreateFile(
			szFile,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);
	}

	if( hFile == INVALID_HANDLE_VALUE )
	{
		lstrcpy( szFile, szDir );
		lstrcat( szFile, TEXT( "\\defskin.jpg" ) );
		hFile = CreateFile(
			szFile,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);
	}

	if( hFile == INVALID_HANDLE_VALUE )
	{
		lstrcpy( szFile, szDir );
		lstrcat( szFile, TEXT( "\\skin.gif" ) );
		hFile = CreateFile(
			szFile,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);
	}

	if( hFile == INVALID_HANDLE_VALUE )
	{
		PicSize.cx = 640L;
		PicSize.cy = 480L;
		return FALSE;
	}

	BOOL bSuccess = HandleToMemDC( hFile, hMemDC, PicSize );
	CloseHandle( hFile );

	return bSuccess;
}


BOOL ChangeSkin( HWND hWnd, HDC hMemDC, SIZE& PicSize )
{
	TCHAR szIniPath[ MAX_PATH * 2 ];
	GetIniPath( szIniPath );

	OPENFILENAME ofn;
	TCHAR szFile[ MAX_PATH + 1 ] = _T( "" );
	TCHAR szDir[ MAX_PATH + 1 ] = _T( "" );
	
	GetPrivateProfileString(
		TEXT( "Options" ),
		TEXT( "SkinDir" ),
		TEXT( "" ),
		szDir,
		MAX_PATH,
		szIniPath
	);

	if( lstrlen( szDir ) == 0 )
	{
		if( GetShell32Version() < 5 || ! SUCCEEDED(
				SHGetFolderPath( NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, szDir )
			)
		) 
		{
			GetCurrentDirectory( MAX_PATH + 1, szDir );
		}
	}
	
	ZeroMemory( &ofn, sizeof( OPENFILENAME ) );
	ofn.lStructSize = sizeof( OPENFILENAME );
	ofn.lpstrTitle = _T( "Skin..." );
	ofn.lpstrInitialDir = szDir;
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = 
		_T( "Skin Image (*.jpg, *.bmp)\0" )
			_T( "*.bmp;*.jpg;*.jpeg;*.jpe;*.wmf;*.gif;*.ico\0" )
			_T( "\0" );
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = MAX_PATH + 1;
	ofn.lpstrDefExt = _T( "jpg" );
#ifdef _UNICODE
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_READONLY | OFN_ENABLESIZING | OFN_DONTADDTORECENT;
#else
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_READONLY | OFN_ENABLESIZING;
#endif


	if( ! GetOpenFileName( &ofn ) )
	{
		return FALSE;
	}

	if( ofn.nFileOffset >= 1 )
	{
		lstrcpy( szDir, szFile );
		szDir[ ofn.nFileOffset - 1 ] = _T('\0');
		WritePrivateProfileString(
			TEXT( "Options" ), 
			TEXT( "SkinDir" ),
			szDir,
			szIniPath
		);
	}

	HANDLE hFile = CreateFile(
		szFile,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if( hFile == INVALID_HANDLE_VALUE ) return FALSE;
	

	int iMaxX = GetSystemMetrics( SM_CXSCREEN );
	int iMaxY = iMaxX * 480 / 640;


	HBITMAP hBmp = CreateCompatibleBitmap( hMemDC, iMaxX, iMaxY );
	SelectObject( hMemDC, hBmp );
	HBRUSH hBrush = CreateSolidBrush( RGB( 0x00, 0x22, 0x44 ) );
	HBRUSH hOldBrush = SelectBrush( hMemDC, hBrush );
	Rectangle( hMemDC, 0, 0, iMaxX, iMaxY );
	SelectObject( hMemDC, hOldBrush );
	DeleteBrush( hBrush );
	DeleteBitmap( hBmp );
	
	BOOL bSuccess = HandleToMemDC( hFile, hMemDC, PicSize );
	CloseHandle( hFile );

	

	InvalidateRect( hWnd, NULL, TRUE );

	if( bSuccess )
	{
		WritePrivateProfileString(
			TEXT( "Options" ), 
			TEXT( "Skin" ),
			szFile,
			szIniPath
		);
	}
	else
	{
		MessageBox( hWnd, szFile,
			_T( "Skin Changer Failed" ),
			MB_OK | MB_ICONEXCLAMATION );
	}
	return bSuccess;

}





BOOL HandleToMemDC( HANDLE hFile, HDC hMemDC, SIZE& PicSize )
{
	PicSize.cx = PicSize.cy = 0L;
	if( hFile == INVALID_HANDLE_VALUE ) return FALSE;

	HGLOBAL hGlobal = NULL;
	IStream  * pIst = NULL;
	IPicture * pIpic = NULL;

	DWORD dwFileSize = GetFileSize( hFile, NULL );
	hGlobal = GlobalAlloc( GMEM_MOVEABLE, dwFileSize );
	LPVOID lpv = GlobalLock( hGlobal );
	DWORD dwBytes;
	ReadFile( hFile, lpv, dwFileSize, &dwBytes, NULL );
	GlobalUnlock( hGlobal );

	CreateStreamOnHGlobal( hGlobal, FALSE, &pIst );
		
	if( S_OK 
		!=
		OleLoadPicture( pIst, 0L, FALSE, IID_IPicture, (LPVOID*) &pIpic )
	)
	{
		if( pIpic != NULL )
		{
			pIpic->Release();
			pIpic = NULL;
		}
		if( pIst != NULL )
		{
			pIst->Release();
			pIst = NULL;
		}
		if( hGlobal != NULL )
		{
			GlobalFree( hGlobal );
			hGlobal = NULL;
		}
		PicSize.cx = 640L;
		PicSize.cy = 480L;

		return FALSE;
	}

	long hmWidth  = 0L;
	long hmHeight = 0L;
	pIpic->get_Width( &hmWidth );
	pIpic->get_Height( &hmHeight );

	PicSize.cx = MulDiv( hmWidth,
				GetDeviceCaps( hMemDC, LOGPIXELSX ), HIMETRIC_INCH );
	PicSize.cy = MulDiv( hmHeight,
				GetDeviceCaps( hMemDC, LOGPIXELSY ), HIMETRIC_INCH );

	int iMaxX = GetSystemMetrics( SM_CXSCREEN );
	int iMaxY = iMaxX * 480 / 640;
	if( PicSize.cx > iMaxX ) PicSize.cx = iMaxX;
	if( PicSize.cy > iMaxY ) PicSize.cy = iMaxY;

	pIpic->Render( hMemDC,
				0, 0,
				PicSize.cx, PicSize.cy,
				0L, hmHeight, hmWidth, -hmHeight,
				NULL
	);

	double ar = (double) PicSize.cx / (double) PicSize.cy;
	if( PicSize.cx != 640L || PicSize.cy != 480L )
	{
		if( ar > 640.0 / 480.0 )
		{
			PicSize.cy = ( long ) floor( PicSize.cx * 480.0 / 640.0 );
		}
		else
		{
			PicSize.cx = ( long ) floor( PicSize.cy * 640.0 / 480.0 );
		}
	}

	if( pIpic != NULL )
	{
		pIpic->Release();
		pIpic = NULL;
	}
	if( pIst != NULL )
	{
		pIst->Release();
		pIst = NULL;
	}
	if( hGlobal != NULL )
	{
		GlobalFree( hGlobal );
		hGlobal = NULL;
	}
	return TRUE;
}





BOOL DrawSkin( HDC hdc, HDC hMemDC, SIZE& SkinSize )
{
	if( SkinSize.cx <= 0L || SkinSize.cy <= 0L )
	{
		return FALSE;
	}

	SetStretchBltMode( hdc, HALFTONE );
	SetBrushOrgEx( hdc, 0, 0, NULL );
	StretchBlt( hdc, 0, 0, 640, 480,
				hMemDC, 0, 0, (int) SkinSize.cx, (int) SkinSize.cy, SRCCOPY );
	return TRUE;
}



