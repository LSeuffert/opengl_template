#define WGL_CONTEXT_MAJOR_VERSION_ARB             0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB             0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB               0x2093
#define WGL_CONTEXT_FLAGS_ARB                     0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126

#define WGL_CONTEXT_DEBUG_BIT_ARB                 0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB    0x0002

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

#define WGL_DRAW_TO_WINDOW_ARB                    0x2001
#define WGL_ACCELERATION_ARB                      0x2003
#define WGL_SUPPORT_OPENGL_ARB                    0x2010
#define WGL_DOUBLE_BUFFER_ARB                     0x2011
#define WGL_PIXEL_TYPE_ARB                        0x2013
#define WGL_COLOR_BITS_ARB                        0x2014

#define WGL_TYPE_RGBA_ARB                         0x202B
#define WGL_FULL_ACCELERATION_ARB                 0x2027

#define WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB          0x20A9

#define WGL_RED_BITS_ARB                          0x2015
#define WGL_GREEN_BITS_ARB                        0x2017
#define WGL_BLUE_BITS_ARB                         0x2019
#define WGL_ALPHA_BITS_ARB                        0x201B
#define WGL_DEPTH_BITS_ARB                        0x2022


typedef BOOL WINAPI wgl_choose_pixel_format_arb(HDC hdc,
                                                const int *piAttribIList,
                                                const FLOAT *pfAttribFList,
                                                UINT nMaxFormats,
                                                int *piFormats,
                                                UINT *nNumFormats);
typedef HGLRC WINAPI wgl_create_context_attribs_arb(HDC hDC, HGLRC hShareContext,
                                                    const int *attribList);
typedef BOOL WINAPI wgl_swap_interval_ext(int interval);

global_var wgl_choose_pixel_format_arb *wglChoosePixelFormatARB;
global_var wgl_create_context_attribs_arb *wglCreateContextAttribsARB;
global_var wgl_swap_interval_ext *wglSwapIntervalEXT;

global_var HGLRC global_gl_rc;

// TODO(Luke): Probably return success/error.
internal void
Win32LoadWGLExtensions()
{
    WNDCLASSA window_class = {};
    
    window_class.lpfnWndProc = DefWindowProcA;
    window_class.hInstance = GetModuleHandleA(0);
    window_class.lpszClassName = "BeeboWGLLoader";
    
    if(RegisterClassA(&window_class))
    {
        HWND temporary_window = CreateWindowExA(
                                                0,
                                                window_class.lpszClassName,
                                                "Beebo",
                                                0,
                                                CW_USEDEFAULT,
                                                CW_USEDEFAULT,
                                                CW_USEDEFAULT,
                                                CW_USEDEFAULT,
                                                0,
                                                0,
                                                window_class.hInstance,
                                                0);
        
        HDC window_dc = GetDC(temporary_window);
        
        int format_index = 0;
        PIXELFORMATDESCRIPTOR desired_format = {};
        desired_format.nSize = sizeof(desired_format);
        desired_format.nVersion = 1;
        desired_format.iPixelType = PFD_TYPE_RGBA;
        desired_format.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
        desired_format.cColorBits = 32;
        desired_format.cAlphaBits = 8;
        
        format_index = ChoosePixelFormat(window_dc, &desired_format);
        
        PIXELFORMATDESCRIPTOR format_received;
        SetPixelFormat(window_dc, format_index, &format_received);
        HGLRC glrc = wglCreateContext(window_dc);
        
        if(wglMakeCurrent(window_dc, glrc))
        {
            wglChoosePixelFormatARB = (wgl_choose_pixel_format_arb *)wglGetProcAddress("wglChoosePixelFormatARB");
            wglCreateContextAttribsARB = (wgl_create_context_attribs_arb *)wglGetProcAddress("wglCreateContextAttribsARB");
            wglSwapIntervalEXT = (wgl_swap_interval_ext *)wglGetProcAddress("wglSwapIntervalEXT");
            
            wglMakeCurrent(0, 0);
        }
        
        wglDeleteContext(glrc);
        ReleaseDC(temporary_window, window_dc);
        DestroyWindow(temporary_window);
    }
}

internal void
Win32SetupPixelFormat(HDC window_dc)
{
    int format_index = 0;
    UINT extended_pick = 0;
    if(wglChoosePixelFormatARB)
    {
        // NOTE(Luke): We might want to do SRGB. In that case 
        //             WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB is what 
        //             we are looking for.
        int attributes_list[] = {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
            WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
            0
        };
        
        wglChoosePixelFormatARB(window_dc, attributes_list, 0, 1, &format_index, &extended_pick);
    }
    
    if(!extended_pick)
    {
        PIXELFORMATDESCRIPTOR desired_format = {};
        desired_format.nSize = sizeof(desired_format);
        desired_format.nVersion = 1;
        desired_format.iPixelType = PFD_TYPE_RGBA;
        desired_format.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
        desired_format.cColorBits = 32;
        desired_format.cAlphaBits = 8;
        
        format_index = ChoosePixelFormat(window_dc, &desired_format);
    }
    
    PIXELFORMATDESCRIPTOR received_format;
    SetPixelFormat(window_dc, format_index, &received_format);
}

internal HGLRC
Win32InitOpenGL(HWND window)
{
    HDC window_dc = GetDC(window);
    Win32LoadWGLExtensions();
    Win32SetupPixelFormat(window_dc);
    
    b32 modern_context = true;
    HGLRC glrc = 0;
    int opengl_attributes[] =
    {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        0,
    };
    
    
    if(wglCreateContextAttribsARB)
    {
        glrc = wglCreateContextAttribsARB(window_dc, 0, opengl_attributes);
    }
    
    if(!glrc)
    {
        modern_context = false;
        glrc = wglCreateContext(window_dc);
    }
    
    wglMakeCurrent(window_dc, glrc);
    
    ReleaseDC(window, window_dc);
    
    return glrc;
}
