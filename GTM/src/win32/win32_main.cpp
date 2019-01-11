/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org>
*/

#include "../shared/app.h"
#include "../shared/platform.h"
#include "../shared/types.h"
#include "../shared/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0601
#include <windows.h>
#include <windowsx.h>
#include <GL/gl.h>

#include <atomic>
typedef std::atomic_uint au32;


namespace
{
    f64 INV_COUNTER_FREQUENCY = 1.0 / 1000.0;


    typedef char* APIENTRY WGLGetExtensionsStringARB();
    WGLGetExtensionsStringARB* wglGetExtensionsStringARB = NULL;
    typedef void APIENTRY WGLSwapIntervalEXT( int interval );
    WGLSwapIntervalEXT* wglSwapIntervalEXT = NULL;


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


    HGLRC       hRC                     = NULL;
    HDC         hDC                     = NULL;
    HWND        hWnd                    = NULL;
    HINSTANCE   hInstance               = NULL;

    char*       TITLE                   = "GTM";
    s32         WIDTH                   = 1280;
    s32         HEIGHT                  = 720;
    s32         COLOUR_BITS             = 32;
    s32         DEPTH_BITS              = 24;
    s32         STENCIL_BITS            = 8;

    bool        active                  = true;
    bool        fullscreen              = false;


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
                App::Resize( WIDTH, HEIGHT );
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
            LOG( "wglMakeCurrent( NULL, NULL ) failed\n" );

        if( hRC && !wglDeleteContext( hRC ) )
        {
            LOG( "wglDeleteContext( hRC ) failed\n" );
            hRC = NULL;
        }

        if( hDC && !ReleaseDC( hWnd, hDC ) )
        {
            LOG( "ReleaseDC( hWnd, hDC ) failed\n" );
            hDC = NULL;
        }

        if( hWnd && !DestroyWindow( hWnd ) )
        {
            LOG( "DestroyWindow( hWnd ) failed\n" );
            hWnd = NULL;
        }

        if( !UnregisterClass( TITLE, hInstance ) )
        {
            LOG( "UnregisterClass( TITLE, hInstance ) failed\n" );
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
            LOG( "RegisterClass( &wc ) failed\n" );
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
                    LOG( "ChangeDisplaySettings( &screenSettings , CDS_FULLSCREEN ) failed\n" );
                    fullscreen = false;
                }
            }
            else
            {
                LOG( "EnumDisplaySettings( NULL, ENUM_CURRENT_SETTINGS, &currentScreenSettings ) failed\n" );
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
            LOG( "CreateWindowEx() failed\n" );
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
            LOG( "GetDC( hWnd ) failed\n" );
            return false;
        }

        s32 pixelFormat = ChoosePixelFormat( hDC, &pfd );
        if( !pixelFormat )
        {
            LOG( "ChoosePixelFormat( hDC, &pfd ) failed\n" );
            return false;
        }

        if( !SetPixelFormat( hDC, pixelFormat, &pfd ) )
        {
            LOG( "SetPixelFormat( hDC, pixelFormat, &pfd ) failed\n" );
            return false;
        }

        hRC = wglCreateContext( hDC );
        if( !hRC )
        {
            LOG( "wglCreateContext( hDC ) failed\n" );
            return false;
        }

        if( !wglMakeCurrent( hDC, hRC ) )
        {
            LOG( "wglMakeCurrent( hDC, hRC ) failed\n" );
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


    struct ThreadInfo
    {
        s32 threadIndex;
        DWORD threadId;
    };

    struct WorkQueueEntry
    {
        Platform::WorkCallback* callback;
        void* data;
    };


    WorkQueueEntry workQueueEntry[ 256 ];
    const u32 QUEUE_ENTRY_MASK          = ARRAY_COUNT( workQueueEntry ) - 1;
    u32 workQueueEntryStartedCount      = 0;
    au32 workQueueEntryCompletedCount   = 0;
    au32 workQueueEntryReadIndex        = 0;
    au32 workQueueEntryWriteIndex       = 0;

    HANDLE hSemaphore;
    ThreadInfo* threadInfo;


    bool DoWork( ThreadInfo* info )
    {
        bool result = false;

        u32 readIndex = workQueueEntryReadIndex;
        if( readIndex != atomic_load( &workQueueEntryWriteIndex ) )
        {
            result = true;

            u32 nextReadIndex = ( readIndex + 1 ) & QUEUE_ENTRY_MASK;
            if( atomic_compare_exchange_weak( &workQueueEntryReadIndex, &readIndex, nextReadIndex ) )
            {
                WorkQueueEntry entry = workQueueEntry[ readIndex ];
                entry.callback( info->threadIndex, entry.data );

                ++workQueueEntryCompletedCount;
            }
        }

        return result;
    }

    DWORD WINAPI ThreadProc( LPVOID lpParameter )
    {
        ThreadInfo *info = ( ThreadInfo* ) lpParameter;

        while( true )
        {
            if( !DoWork( info ) )
                WaitForSingleObjectEx( hSemaphore, INFINITE, false );
        };
    }
}


namespace Platform
{
    void Log( char* string, ... )
    {
        const s32 OUTPUT_LENGTH = 256;
        char output[ OUTPUT_LENGTH ];

        va_list args;
        va_start( args, string );
        vsprintf_s( output, OUTPUT_LENGTH, string, args );
        va_end( args );

        OutputDebugString( output );
    }


    f64 GetTime()
    {
        f64 result = 0;

        LARGE_INTEGER performanceCounter;
        if( QueryPerformanceCounter( &performanceCounter ) != 0 )
            result = ( f64 ) performanceCounter.QuadPart * INV_COUNTER_FREQUENCY;

        return result;
    }


    s32 GetProcessorCount()
    {
        SYSTEM_INFO systemInfo;
        GetSystemInfo( &systemInfo );

        s32 result = Utils::Max( ( s32 ) systemInfo.dwNumberOfProcessors, 1 );
        return result;
    }

    void FinishWork()
    {
        ASSERT( GetCurrentThreadId() == threadInfo[ 0 ].threadId );

        while( atomic_load( &workQueueEntryCompletedCount ) < workQueueEntryStartedCount )
            DoWork( &threadInfo[ 0 ] );

        workQueueEntryStartedCount = 0;
        atomic_store( &workQueueEntryCompletedCount, 0u );
    }

    void AddWorkQueueEntry( WorkCallback* callback, void* data )
    {
        ASSERT( GetCurrentThreadId() == threadInfo[ 0 ].threadId );

        u32 nextWriteIndex = ( workQueueEntryWriteIndex + 1 ) & QUEUE_ENTRY_MASK;
        if( nextWriteIndex == atomic_load( &workQueueEntryReadIndex ) )
        {
            Log( "Work queue full!\n" );
            FinishWork();
        }

        WorkQueueEntry* entry = &workQueueEntry[ workQueueEntryWriteIndex ];
        entry->callback = callback;
        entry->data = data;

        ++workQueueEntryStartedCount;
        atomic_store( &workQueueEntryWriteIndex, nextWriteIndex );

        ReleaseSemaphore( hSemaphore, 1, 0 );
    }
}


s32 WINAPI WinMain( HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR     lpCmdLine,
                    s32       nCmdShow )
{
    LARGE_INTEGER wallClockFrequency;
    QueryPerformanceFrequency( &wallClockFrequency );
    INV_COUNTER_FREQUENCY = 1.0 / ( f64 ) wallClockFrequency.QuadPart;


    ASSERT( Utils::IsPowerOf2( ARRAY_COUNT( workQueueEntry ) ) );

    s32 threadCount             = Platform::GetProcessorCount();
    threadInfo                  = new ThreadInfo[ threadCount ];
    threadInfo[ 0 ].threadIndex = 0;
    threadInfo[ 0 ].threadId    = GetCurrentThreadId();

    hSemaphore = CreateSemaphore( 0, 0, threadCount, 0 );

    for( s32 i = 1; i < threadCount; ++i )
    {
        threadInfo[ i ].threadIndex = i;

        u32 stackSize = 0;
        HANDLE hThread = CreateThread( 0, stackSize, ThreadProc, &threadInfo[ i ], 0, &threadInfo[ i ].threadId );
        CloseHandle( hThread );
    }


    if( !CreateWindowGL( fullscreen ) )
        return 0;


    App::Init( WIDTH, HEIGHT );


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
            App::Update();
            App::Render();
            SwapBuffers( hDC );
        }
    }

    return 0;
}