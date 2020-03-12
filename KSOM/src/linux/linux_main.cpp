#include "../shared/types.h"
#include "../shared/app.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <GL/glx.h>


typedef void APIENTRY GLXSwapIntervalEXT( Display* display, GLXDrawable drawable, int interval );
GLXSwapIntervalEXT* _glXSwapIntervalEXT = NULL;
typedef int APIENTRY GLXSwapIntervalMESA( unsigned int interval );
GLXSwapIntervalMESA* _glXSwapIntervalMESA = NULL;
typedef int APIENTRY GLXSwapIntervalSGI( int interval );
GLXSwapIntervalSGI* _glXSwapIntervalSGI = NULL;

enum GLXSwapIntervalMode
{
    GLXSwapIntervalMode_None = 0,
    GLXSwapIntervalMode_Ext,
    GLXSwapIntervalMode_MESA,
    GLXSwapIntervalMode_SGI
};

GLXSwapIntervalMode glXSwapIntervalMode = GLXSwapIntervalMode_None;


Display*    display         = NULL;
s32         screenId        = 0;
Window      window;
GLXContext  glxContext;
Atom        wmDeleteWindow;

char*       TITLE           = "KSOM";
s32         WIDTH           = 1280;
s32         HEIGHT          = 720;
s32         COLOUR_BITS     = 32;
s32         DEPTH_BITS      = 24;
s32         STENCIL_BITS    = 8;

bool        active          = true;
bool        fullscreen      = false;


bool HasGLXExtension( char* extension )
{
    char* extensionString = ( char* ) glXQueryExtensionsString( display, screenId );

    bool result = strstr( extensionString, extension );
    return result;
}

void glXSwapInterval( s32 interval )
{
    switch( glXSwapIntervalMode )
    {
        case GLXSwapIntervalMode_Ext:   _glXSwapIntervalEXT( display, window, interval );   break;
        case GLXSwapIntervalMode_MESA:  _glXSwapIntervalMESA( interval );                   break;
        case GLXSwapIntervalMode_SGI:   _glXSwapIntervalSGI( interval );                    break;
        default: break;
    }
}

bool ProcessEvent( XEvent* xev )
{
    switch( xev->type )
    {
        case KeyPress:
        {
            return true;
        }
        case KeyRelease:
        {
            return true;
        }
        case ButtonPress:
        {
            //s32 delta = 0;

            XButtonEvent* event = ( XButtonEvent* ) xev;
            if( event->button == 1
                || event->button == 2
                || event->button == 3 )
            {
                //s32 x = event->x;
                //s32 y = event->y;
            }
            else if( event->button == 4 )
            {
                //delta = 1;
            }
            else if( event->button == 5 )
            {
                //delta = -1;
            }

            return true;
        }
        case ButtonRelease:
        {
            XButtonEvent* event = ( XButtonEvent* ) xev;
            if( event->button == 1
                || event->button == 2
                || event->button == 3 )
            {
                //s32 x = event->x;
                //s32 y = event->y;
            }

            return true;
        }
        case MotionNotify:
        {
            //XMotionEvent* event = ( XMotionEvent* ) xev;
            //s32 x = event->x;
            //s32 y = event->y;
            return true;
        }
        case UnmapNotify:
        {
            active = false;
            return true;
        }
        case MapNotify:
        {
            active = true;
            return true;
        }
        case ConfigureNotify:
        {
            XConfigureEvent* event = ( XConfigureEvent* ) xev;
            WIDTH = event->width;
            HEIGHT = event->height;
            app::resize( WIDTH, HEIGHT );
            return true;
        }
    }

    return false;
}

void DestroyWindowGL()
{
    if( !glXMakeCurrent( display, None, NULL ) )
        printf( "glXMakeCurrent( display, None, NULL ) failed\n" );

    glXDestroyContext( display, glxContext );

    XDestroyWindow( display, window );

    XCloseDisplay( display );
    display = NULL;
}

bool CreateWindowGL( bool fullscreenFlag )
{
    fullscreen = fullscreenFlag;

    display = XOpenDisplay( NULL );
    if( !display )
    {
        printf( "XOpenDisplay( NULL ) failed\n" );
        return false;
    }

    Window root = DefaultRootWindow( display );

    s32 bitsPerColour           = COLOUR_BITS / 4;
    GLint visualAttributes[]    = { GLX_RGBA,
                                    GLX_DOUBLEBUFFER,
                                    GLX_RED_SIZE,       bitsPerColour,
                                    GLX_GREEN_SIZE,     bitsPerColour,
                                    GLX_BLUE_SIZE,      bitsPerColour,
                                    GLX_ALPHA_SIZE,     bitsPerColour,
                                    GLX_DEPTH_SIZE,     DEPTH_BITS,
                                    GLX_STENCIL_SIZE,   STENCIL_BITS,
                                    None };

    XVisualInfo* visualInfo = glXChooseVisual( display, screenId, visualAttributes );

    if( !visualInfo )
    {
        printf( "glXChooseVisual( display, 0, visualAttributes ) failed\n" );
        return false;
    }

    if( fullscreen )
    {
        Screen* screen = XScreenOfDisplay( display, screenId );
        if( screen )
        {
            WIDTH = XWidthOfScreen( screen );
            HEIGHT = XHeightOfScreen( screen );
        }
        else
        {
            printf( "XScreenOfDisplay( display, screenId ) failed\n" );
            fullscreen = false;
        }
    }

    Colormap colourMap = XCreateColormap( display, root, visualInfo->visual, AllocNone );

    XSetWindowAttributes windowAttributes   = {};
    windowAttributes.event_mask             = ExposureMask;
    windowAttributes.event_mask             |= StructureNotifyMask;
    windowAttributes.event_mask             |= KeyPressMask | KeyReleaseMask;
    windowAttributes.event_mask             |= PointerMotionMask | ButtonPressMask | ButtonReleaseMask;
    windowAttributes.colormap               = colourMap;

    window = XCreateWindow( display,
                            root,
                            0, 0,
                            WIDTH, HEIGHT,
                            0,
                            visualInfo->depth,
                            InputOutput,
                            visualInfo->visual,
                            CWEventMask | CWColormap | CWCursor,
                            &windowAttributes );

    XMapWindow( display, window );
    XStoreName( display, window, TITLE );

    if( fullscreen )
    {
        Atom wmState                = XInternAtom( display, "_NET_WM_STATE", false );
        Atom wmStateFullscreen      = XInternAtom( display, "_NET_WM_STATE_FULLSCREEN", false );

        XEvent xev                  = {};
        xev.type                    = ClientMessage;
        xev.xclient.window          = window;
        xev.xclient.message_type    = wmState;
        xev.xclient.format          = 32;
        xev.xclient.data.l[0]       = 1;
        xev.xclient.data.l[1]       = wmStateFullscreen;
        xev.xclient.data.l[2]       = 0;

        XSendEvent( display,
                    root,
                    false,
                    SubstructureRedirectMask | SubstructureNotifyMask,
                    &xev );

        XFlush( display );
    }

    glxContext = glXCreateContext( display, visualInfo, NULL, GL_TRUE );
    if( !glXMakeCurrent( display, window, glxContext ) )
    {
        printf( "glXMakeCurrent( display, window, glxContext ) failed\n" );
        return false;
    }

    //*@TODO test on actual hardware not vm
    if( HasGLXExtension( "GLX_EXT_swap_control" ) )
    {
        _glXSwapIntervalEXT = ( GLXSwapIntervalEXT* ) glXGetProcAddress( ( GLubyte* ) "glXSwapIntervalEXT" );
        glXSwapIntervalMode = GLXSwapIntervalMode_Ext;
    }
    else if( HasGLXExtension( "GLX_MESA_swap_control" ) )
    {
        _glXSwapIntervalMESA = ( GLXSwapIntervalMESA* ) glXGetProcAddress( ( GLubyte* ) "glXSwapIntervalMESA" );
        glXSwapIntervalMode = GLXSwapIntervalMode_MESA;
    }
    else if( HasGLXExtension( "GLX_SGI_swap_control" ) )
    {
        _glXSwapIntervalSGI = ( GLXSwapIntervalSGI* ) glXGetProcAddress( ( GLubyte* ) "glXSwapIntervalSGI" );
        glXSwapIntervalMode = GLXSwapIntervalMode_SGI;
    }

    if( HasGLXExtension( "GLX_EXT_swap_control_tear" ) )
        glXSwapInterval( -1 );
    else
        glXSwapInterval( 1 );
    //*/

    wmDeleteWindow = XInternAtom( display, "WM_DELETE_WINDOW", false );
    XSetWMProtocols( display, window, &wmDeleteWindow, 1 );

    return true;
}

s32 main()
{
	if( !CreateWindowGL( fullscreen ) )
        return 0;

    app::init( WIDTH, HEIGHT );

    XEvent xev;
    bool done = false;
    while( !done )
    {
        while( XPending( display ) )
        {
            XNextEvent( display, &xev );

            if( !ProcessEvent( &xev ) )
            {
                if( xev.type == ClientMessage && ( u64 ) xev.xclient.data.l[0] == wmDeleteWindow )
                {
                    done = true;
                    active = false;
                    break;
                }
            }
        }

        if( active )
        {
            app::update();
            app::render();
            glXSwapBuffers( display, window );
        }
    }

	return 0;
}