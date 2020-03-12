#include "../shared/types.h"
#include "../shared/app.h"

#include <stdlib.h>
#include <string.h>

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0601
#include <windows.h>
#include <windowsx.h>
#include <GL/gl.h>


typedef char* APIENTRY WGLGetExtensionsStringARB();
WGLGetExtensionsStringARB* wglGetExtensionsStringARB = NULL;
typedef void APIENTRY WGLSwapIntervalEXT( int interval );
WGLSwapIntervalEXT* wglSwapIntervalEXT = NULL;


HGLRC       hRC             = NULL;
HDC         hDC             = NULL;
HWND        hWnd            = NULL;
HINSTANCE   hInstance       = NULL;

char*       TITLE           = "KSOM";
s32         WIDTH           = 1280;
s32         HEIGHT          = 720;
s32         COLOUR_BITS     = 32;
s32         DEPTH_BITS      = 24;
s32         STENCIL_BITS    = 8;

bool        active          = true;
bool        fullscreen      = false;


bool HasGLExtension( char* extension )
{
    char* extensionString = ( char* ) glGetString( GL_EXTENSIONS );

    bool result = strstr( extensionString, extension );
    return result;
}

bool HasWGLExtension( char* extension )
{
    if( wglGetExtensionsStringARB )
    {
        char* extensionString = ( char* ) wglGetExtensionsStringARB();

        bool result = strstr( extensionString, extension );
        return result;
    }

    return false;
}

LRESULT CALLBACK WndProc( HWND    hWnd,
                          UINT    uMsg,
                          WPARAM  wParam,
                          LPARAM  lParam )
{
    switch( uMsg )
    {
        case WM_SIZE:
        {
            WIDTH = LOWORD( lParam );
            HEIGHT = HIWORD( lParam );
            app::resize( WIDTH, HEIGHT );
            return 0;
        }
        case WM_ACTIVATE:
        {
            active = !HIWORD( wParam );
            return 0;
        }
        case WM_CLOSE:
        {
            PostQuitMessage( 0 );
            return 0;
        }
        case WM_KEYDOWN:
        {
            return 0;
        }
        case WM_KEYUP:
        {
            return 0;
        }
        case WM_SYSCOMMAND:
        {
            switch( wParam )
            {
                case SC_SCREENSAVE:
                case SC_MONITORPOWER:
                    return 0;
            }

            break;
        }
        case WM_MOUSEMOVE:
        {
            s32 x = GET_X_LPARAM( lParam );
            s32 y = GET_Y_LPARAM( lParam );
            return 0;
        }
        case WM_LBUTTONDOWN:
        {
            s32 x = GET_X_LPARAM( lParam );
            s32 y = GET_Y_LPARAM( lParam );
            return 0;
        }
        case WM_LBUTTONUP:
        {
            s32 x = GET_X_LPARAM( lParam );
            s32 y = GET_Y_LPARAM( lParam );
            return 0;
        }
        case WM_RBUTTONDOWN:
        {
            s32 x = GET_X_LPARAM( lParam );
            s32 y = GET_Y_LPARAM( lParam );
            return 0;
        }
        case WM_RBUTTONUP:
        {
            s32 x = GET_X_LPARAM( lParam );
            s32 y = GET_Y_LPARAM( lParam );
            return 0;
        }
        case WM_MBUTTONDOWN:
        {
            s32 x = GET_X_LPARAM( lParam );
            s32 y = GET_Y_LPARAM( lParam );
            return 0;
        }
        case WM_MBUTTONUP:
        {
            s32 x = GET_X_LPARAM( lParam );
            s32 y = GET_Y_LPARAM( lParam );
            return 0;
        }
        case WM_MOUSEWHEEL:
        {
            s32 delta = GET_WHEEL_DELTA_WPARAM( wParam );
            return 0;
        }
    }
    
    return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

void DestroyWindowGL()
{
    if( fullscreen )
    {
        ChangeDisplaySettings( NULL, 0 );
        ShowCursor( true );
    }
    
    if( !wglMakeCurrent( NULL, NULL ) )
        OutputDebugString( "wglMakeCurrent( NULL, NULL ) failed\n" );

    if( hRC && !wglDeleteContext( hRC ) )
    {
        OutputDebugString( "wglDeleteContext( hRC ) failed\n" );
        hRC = NULL;
    }

    if( hDC && !ReleaseDC( hWnd, hDC ) )
    {
        OutputDebugString( "ReleaseDC( hWnd, hDC ) failed\n" );
        hDC = NULL;
    }

    if( hWnd && !DestroyWindow( hWnd ) )
    {
        OutputDebugString( "DestroyWindow( hWnd ) failed\n" );
        hWnd = NULL;
    }

    if( !UnregisterClass( TITLE, hInstance ) )
    {
        OutputDebugString( "UnregisterClass( TITLE, hInstance ) failed\n" );
        hInstance = NULL;
    }
}

bool CreateWindowGL( bool fullscreenFlag )
{
    fullscreen  = fullscreenFlag;
    hInstance   = GetModuleHandle( NULL );

    WNDCLASS wc         = {};
    wc.style            = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc      = ( WNDPROC ) WndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = hInstance;
    wc.hIcon            = LoadIcon( hInstance, NULL );
    wc.hCursor          = LoadCursor( hInstance, IDC_ARROW );
    wc.hbrBackground    = NULL;
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = TITLE;

    if( !RegisterClass( &wc ) )
    {
        OutputDebugString( "RegisterClass( &wc ) failed\n" );
        return false;
    }

    if( fullscreen )
    {
        s32 displayHz = 0;
        DEVMODE currentScreenSettings       = {};
        currentScreenSettings.dmSize        = sizeof( currentScreenSettings );
        currentScreenSettings.dmDriverExtra = 0;
        if( EnumDisplaySettings( NULL, ENUM_CURRENT_SETTINGS, &currentScreenSettings ) )
        {
            WIDTH       = currentScreenSettings.dmPelsWidth;
            HEIGHT      = currentScreenSettings.dmPelsHeight;
            displayHz   = currentScreenSettings.dmDisplayFrequency;

            DEVMODE screenSettings              = {};
            screenSettings.dmSize               = sizeof( screenSettings );
            screenSettings.dmPelsWidth          = WIDTH;
            screenSettings.dmPelsHeight         = HEIGHT;
            screenSettings.dmBitsPerPel         = COLOUR_BITS;
            screenSettings.dmDisplayFrequency   = displayHz;
            screenSettings.dmFields             = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;

            if( ChangeDisplaySettings( &screenSettings , CDS_FULLSCREEN ) != DISP_CHANGE_SUCCESSFUL )
            {
                OutputDebugString( "ChangeDisplaySettings( &screenSettings , CDS_FULLSCREEN ) failed\n" );
                fullscreen = false;
            }
        }
        else
        {
            OutputDebugString( "EnumDisplaySettings( NULL, ENUM_CURRENT_SETTINGS, &currentScreenSettings ) failed\n" );
            fullscreen = false;
        }
    }

    DWORD dwStyle;
    DWORD dwExStyle;

    if( fullscreen )
    {
        dwStyle     = WS_POPUP;
        dwExStyle   = WS_EX_APPWINDOW;
    }
    else
    {
        dwStyle     = WS_OVERLAPPEDWINDOW;
        dwExStyle   = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    }

    RECT windowRect     = {};
    windowRect.left     = ( s64 ) 0;
    windowRect.right    = ( s64 ) WIDTH;
    windowRect.top      = ( s64 ) 0;
    windowRect.bottom   = ( s64 ) HEIGHT;
    AdjustWindowRectEx( &windowRect, dwStyle, false, dwExStyle );

    hWnd = CreateWindowEx( dwExStyle,
                           TITLE,
                           TITLE,
                           WS_CLIPSIBLINGS | WS_CLIPCHILDREN | dwStyle,
                           0, 0,
                           windowRect.right - windowRect.left,
                           windowRect.bottom - windowRect.top,
                           NULL,
                           NULL,
                           hInstance,
                           NULL );
    if( !hWnd )
    {
        OutputDebugString( "CreateWindowEx() failed\n" );
        return false;
    }

    static PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof( PIXELFORMATDESCRIPTOR ),
        1,
        PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL,
        PFD_TYPE_RGBA,
        ( BYTE ) COLOUR_BITS,
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        ( BYTE ) DEPTH_BITS,
        ( BYTE ) STENCIL_BITS,
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    hDC = GetDC( hWnd );
    if( !hDC )
    {
        OutputDebugString( "GetDC( hWnd ) failed\n" );
        return false;
    }

    s32 pixelFormat = ChoosePixelFormat( hDC, &pfd );
    if( !pixelFormat )
    {
        OutputDebugString( "ChoosePixelFormat( hDC, &pfd ) failed\n" );
        return false;
    }

    if( !SetPixelFormat( hDC, pixelFormat, &pfd ) )
    {
        OutputDebugString( "SetPixelFormat( hDC, pixelFormat, &pfd ) failed\n" );
        return false;
    }

    hRC = wglCreateContext( hDC );
    if( !hRC )
    {
        OutputDebugString( "wglCreateContext( hDC ) failed\n" );
        return false;
    }

    if( !wglMakeCurrent( hDC, hRC ) )
    {
        OutputDebugString( "wglMakeCurrent( hDC, hRC ) failed\n" );
        return false;
    }

    wglGetExtensionsStringARB = ( WGLGetExtensionsStringARB* ) wglGetProcAddress( "wglGetExtensionsStringARB" );
    if( wglGetExtensionsStringARB )
    {
        char* swapControlExt = "WGL_EXT_swap_control";
        if( HasWGLExtension( swapControlExt ) || HasGLExtension( swapControlExt ) )
        {
            wglSwapIntervalEXT = ( WGLSwapIntervalEXT* ) wglGetProcAddress( "wglSwapIntervalEXT" );

            char* adaptiveVSyncExt = "WGL_EXT_swap_control_tear";
            if( HasWGLExtension( adaptiveVSyncExt ) )
                wglSwapIntervalEXT( -1 );
            else
                wglSwapIntervalEXT( 1 );
        }
    }

    ShowWindow( hWnd, SW_SHOW );
    SetForegroundWindow( hWnd );
    SetFocus( hWnd );

    return true;
}

s32 WINAPI WinMain( HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR     lpCmdLine,
                    s32       nCmdShow )
{
    if( !CreateWindowGL( fullscreen ) )
        return 0;

    app::init( WIDTH, HEIGHT );

    MSG msg;
    bool done = false;
    while( !done )
    {
        while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            if( msg.message != WM_QUIT )
            {
                TranslateMessage( &msg );
                DispatchMessage( &msg );
            }
            else
            {
                done = true;
                active = false;
                break;
            }
        }
        
        if( active )
        {
            app::update();
            app::render();
            SwapBuffers( hDC );
        }
    }

    return 0;
}