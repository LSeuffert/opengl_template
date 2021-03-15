#include <windows.h>
#include <gl/gl.h>
#include <cstdint>
#include <stdio.h>
#include <dsound.h>
#include "definitions.h"
// NOTE(Luke): All globals should be put here. There's really no need to chase them.
// TODO(Luke): Figure out where to put these globals.
global_var b32 global_run = true;
global_var u32 global_texture_handle = 1;
global_var LPDIRECTSOUNDBUFFER global_secondary_buffer;
// TODO(Luke): Move this out of here
struct Image
{
    i32  width;
    i32  height;
    u32 *data;
    u32  handle;
};

#include "game_platform.h"
#include "win32_gltemplate.h"
#include "win32_opengl.cpp"
#include "game_opengl.cpp"
#include "miniz.c"
#include "aseprite.h"
#include "aseprite.cpp"


// TODO(Luke): Temporary
global_var b32 move_left;
global_var b32 move_right;
global_var b32 move_up;
global_var b32 move_down;

struct game_button_state
{
    int half_transitions;
    b32 ended_down;
};

// TODO(Luke): Remove this at some point.
b32 internal
AddCStrings(char *left, int left_size, char* right, int right_size, char* result, int result_size)
{
    int result_index = 0;
    
    for(int left_index = 0; 
        (left_index < left_size) && (left[left_index] != 0); 
        left_index++, result_index++)
    {
        if(result_index >= result_size - 1) return false;
        result[result_index] = left[left_index];
    }
    
    for(int right_index = 0; 
        (right_index < right_size) && (right[right_index] != 0); 
        right_index++, result_index++)
    {
        if(result_index >= result_size - 1) return false;
        result[result_index] = right[right_index];
    }
    
    result[result_index + 1] = 0;
    
    return true;
}


LRESULT CALLBACK
Win32MainWindowCallback(HWND window,
                        UINT message,
                        WPARAM wparam,
                        LPARAM lparam)
{
    LRESULT result = 0;
    
    switch(message)
    {
        case WM_QUIT:
        case WM_DESTROY:
        case WM_CLOSE:
        {
            global_run = false;
        }break;
        
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        {
            Assert("Mouse/keyboard input came from elsewhere.");
        }break;
        
        default:
        {
            result = DefWindowProcA(window, message, wparam, lparam);
        };
    }
    
    return result;
}

// TODO(Luke): Check out and probably replace with XAudio2
#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevic, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);
internal void
Win32InitDSound(HWND window, i32 samples_per_second, i32 buffer_size)
{
    HMODULE dsound_library = LoadLibraryA("dsound.dll");
    if(dsound_library)
    {
        direct_sound_create *DirectSoundCreate = (direct_sound_create *)GetProcAddress(dsound_library, "DirectSoundCreate");
        
        LPDIRECTSOUND direct_sound;
        if(DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &direct_sound, 0)))
        {
            WAVEFORMATEX wave_format = {};
            wave_format.wFormatTag = WAVE_FORMAT_PCM;
            wave_format.nChannels = 2;
            wave_format.nSamplesPerSec = samples_per_second;
            wave_format.wBitsPerSample = 16;
            wave_format.nBlockAlign = (wave_format.nChannels*wave_format.wBitsPerSample) / 8;
            wave_format.nAvgBytesPerSec = wave_format.nSamplesPerSec*wave_format.nBlockAlign;
            wave_format.cbSize = 0;
            
            if(SUCCEEDED(direct_sound->SetCooperativeLevel(window, DSSCL_PRIORITY)))
            {
                DSBUFFERDESC buffer_description = {};
                buffer_description.dwSize = sizeof(buffer_description);
                buffer_description.dwFlags = DSBCAPS_PRIMARYBUFFER;
                
                LPDIRECTSOUNDBUFFER primary_buffer;
                if(SUCCEEDED(direct_sound->CreateSoundBuffer(&buffer_description, &primary_buffer, 0)))
                {
                    HRESULT error = primary_buffer->SetFormat(&wave_format);
                    if(SUCCEEDED(error))
                    {
                        OutputDebugStringA("Primary buffer format was set.\n");
                    }
                }
            }
            
            DSBUFFERDESC buffer_description = {};
            buffer_description.dwSize = sizeof(buffer_description);
            // NOTE(Luke): Could use this for older DSBCAPS_GETCURRENTPOSITION2;
            buffer_description.dwFlags = DSBCAPS_TRUEPLAYPOSITION;
#if BEEBO_DEV
            buffer_description.dwFlags |= DSBCAPS_GLOBALFOCUS;
#endif
            buffer_description.dwBufferBytes = buffer_size;
            buffer_description.lpwfxFormat = &wave_format;
            HRESULT error = direct_sound->CreateSoundBuffer(&buffer_description, &global_secondary_buffer, 0);
            if(SUCCEEDED(error))
            {
                OutputDebugStringA("Secondary buffer createed successfully.\n");
            }
        }
    }
}

internal void
Win32ProcessMessages()
{
    MSG message = {};
    for(;;)
    {
        BOOL got_message = FALSE;
        
        got_message = PeekMessage(&message, 0, 0, 0, PM_REMOVE);
        
        if(!got_message)
        {
            break;
        }
        
        switch(message.message)
        {        
            case WM_QUIT:
            case WM_DESTROY:
            case WM_CLOSE:
            {
                global_run = false;
            }break;
            
            case WM_LBUTTONDOWN:
            {
                // OutputDebugStringA("Left mouse button pressed\n");
            }break;
            
            case WM_LBUTTONUP:
            {
                // OutputDebugStringA("Left mouse button released\n");
            }break;
            
            case WM_RBUTTONDOWN:
            {
                // OutputDebugStringA("Right mouse button pressed\n");
            }break;
            
            case WM_RBUTTONUP:
            {
                // OutputDebugStringA("Right mouse button released\n");
            }break;
            
            // TODO(Luke): I think we want to be able to handle any keypress.
            //             I really like it when people are allowed to have
            //             unique keybinds!
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                u32 vk_code = (u32)message.wParam;
                b32 alt_pressed = (message.lParam & (1 << 29));
                b32 shift_pressed = (message.lParam & (1 << 15));
                b32 was_down = ((message.lParam & (1 << 30)) != 0);
                b32 is_down = ((message.lParam & (1 << 31)) == 0);
                b32 altf4_pressed = is_down && alt_pressed && (vk_code == VK_F4);
                
                if(altf4_pressed)
                {
                    global_run = false;
                }
                
                if(vk_code == 'W')
                {
                    // OutputDebugStringA("W pressed\n");
                    if(is_down)
                    {
                        move_up = true;
                    }
                    else
                    {
                        move_up = false;
                    }
                }
                else if(vk_code == 'A')
                {
                    // OutputDebugStringA("A pressed\n");
                    if(is_down)
                    {
                        move_left = true;
                    }
                    else
                    {
                        move_left = false;
                    }
                }
                else if(vk_code == 'S')
                {
                    // OutputDebugStringA("S pressed\n");
                    if(is_down)
                    {
                        move_down = true;
                    }
                    else
                    {
                        move_down = false;
                    }
                }
                else if (vk_code == 'D')
                {
                    // OutputDebugStringA("D pressed\n");
                    if(is_down)
                    {
                        move_right = true;
                    }
                    else
                    {
                        move_right = false;
                    }
                }
            }break;
            
            default:
            {
                TranslateMessage(&message);
                DispatchMessageA(&message);
            }break;
            
        }
    }
}

int WinMain(HINSTANCE instance,
            HINSTANCE prev_instance,
            LPSTR     command_line,
            int       window_size_parameter)
{
    LARGE_INTEGER performance_frequency_result;
    QueryPerformanceFrequency(&performance_frequency_result);
    i64 performance_frequency = performance_frequency_result.QuadPart;
    b32 granular_sleep = (timeBeginPeriod(1) == TIMERR_NOERROR);
    
    // TODO(Luke): Initialize temporary and persistent memory.
    
    // NOTE(Luke): To add an icon use 'window_class.hIcon'.
    WNDCLASS window_class = {};
    window_class.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
    window_class.lpfnWndProc = Win32MainWindowCallback;
    window_class.hInstance = instance;
    window_class.lpszClassName = "BeeboWindowClass";
    
    // TODO(Luke): Set resolution
    
    // TODO(Luke): Move this out into its own function.
    char dll_file_path[MAX_PATH];
    DWORD module_name_result = GetModuleFileNameA(0, dll_file_path, sizeof(dll_file_path));
    Assert(module_name_result != ERROR_INSUFFICIENT_BUFFER);
    for(DWORD right = module_name_result; right >= 1; right--)
    {
        if(dll_file_path[right - 1] == '\\') 
        {
            dll_file_path[right] = 0;
            break;
        }
    }
    Assert(module_name_result > 0);
    
    // TODO(Luke): Pull this out into its own function.
    win32_game_code Game = {};
    WIN32_FILE_ATTRIBUTE_DATA ignored;
    char lock_file_name[MAX_PATH] = "lock.tmp";
    char lock_file_path[MAX_PATH];
    char game_dll_name[MAX_PATH] = "beebo.dll";
    char game_dll_path[MAX_PATH];
    char temp_dll_name[MAX_PATH] = "beebo_temp.dll";
    char temp_dll_path[MAX_PATH];
    b32 created_lock_string = AddCStrings(dll_file_path, sizeof(dll_file_path), lock_file_name, sizeof(lock_file_name), lock_file_path, sizeof(lock_file_path));
    b32 created_game_dll_string = AddCStrings(dll_file_path, sizeof(dll_file_path), game_dll_name, sizeof(game_dll_name), game_dll_path, sizeof(game_dll_path));
    b32 created_temp_dll_string = AddCStrings(dll_file_path, sizeof(dll_file_path), temp_dll_name, sizeof(temp_dll_name), temp_dll_path, sizeof(temp_dll_path));
    Assert(created_lock_string && created_game_dll_string && created_temp_dll_string);
    
    if(!GetFileAttributesEx(lock_file_path, GetFileExInfoStandard, &ignored))
    {
        // TODO(Luke): This needs to be filled out.
        // Game.dll_last_timestamp = Win32GetLastWriteTime(SourceDLLName);
        
        CopyFile(game_dll_path, temp_dll_path, FALSE);
        
        Game.game_code_dll = LoadLibraryA(temp_dll_path);
        if(Game.game_code_dll)
        {
            Game.UpdateAndRender = (game_update_and_render *)
                GetProcAddress(Game.game_code_dll, "GameUpdateAndRender");
            
            Game.valid = Game.UpdateAndRender != 0;
        }
    }
    
    if(!Game.valid)
    {
        Game.UpdateAndRender = 0;
    }
    
    if(RegisterClassA(&window_class))
    {
        HWND window = CreateWindowEx(0,
                                     window_class.lpszClassName,
                                     "BEEBO",
                                     WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                                     CW_USEDEFAULT,
                                     CW_USEDEFAULT,
                                     CW_USEDEFAULT,
                                     CW_USEDEFAULT,
                                     0,
                                     0,
                                     instance,
                                     0);
        
        // TODO(Luke): Have this linked to monitor's refresh rate.
        f32 game_hz = 30.0f;
        f32 frame_period = 1.0f / game_hz;
        LARGE_INTEGER end_time;
        LARGE_INTEGER work_time;
        LARGE_INTEGER start_time;
        f32 time_took;
        if(window)
        {
            Win32InitDSound(window, 48000, 48000*sizeof(i16)*2);
            
            if(Win32InitOpenGL(window))
            {
                // TODO IMPORTANT(Luke): This is just a simple test of the aseprite reader.
                Image beebo = ReadAseprite("egg.aseprite");
                
                int x = 100;
                int y = 100;
                QueryPerformanceCounter(&start_time);
                while(global_run)
                {
                    // TODO(Luke): Add game layer code
                    Win32ProcessMessages();
                    
                    //Game.UpdateAndRender();
                    
                    char s[256];
                    _snprintf_s(s, sizeof(s), "%i, %i\n", x, y);
                    OutputDebugStringA(s);
                    
                    if(move_left) x -= 5;
                    if(move_right) x += 5;
                    if(move_down) y -= 5;
                    if(move_up) y += 5;
                    
                    RECT window_rect;
                    GetClientRect(window, &window_rect);
                    i32 window_width = window_rect.right - window_rect.left;
                    i32 window_height = window_rect.bottom - window_rect.top;
                    OpenGLDrawBitmap(window, window_width, window_height, &beebo, (f32)x, (f32)y);
                    
                    QueryPerformanceCounter(&work_time);
                    f32 seconds_of_work = ((f32)(work_time.QuadPart - start_time.QuadPart)) / ((f32)performance_frequency);
                    if(seconds_of_work < frame_period)
                    {
                        // TODO(Luke): No clue if this is actually correct.
                        if(granular_sleep)
                        {
                            DWORD sleep_ms = (DWORD(1000.0f * (frame_period - seconds_of_work)));
                            if(sleep_ms > 0) 
                            {
                                Sleep(sleep_ms);
                            }
                        }
                        
                        while(seconds_of_work < frame_period)
                        {
                            QueryPerformanceCounter(&work_time);
                            seconds_of_work = ((f32)(work_time.QuadPart - start_time.QuadPart)) / ((f32)performance_frequency);
                        }
                    }
                    
                    QueryPerformanceCounter(&end_time);
                    time_took = ((f32)(end_time.QuadPart - start_time.QuadPart)) / ((f32)performance_frequency);
                    
                    _snprintf_s(s, sizeof(s), "Time took: %f\n seconds\n", time_took);
                    OutputDebugStringA(s);
                    start_time = end_time;
                }
            }
        }
    }
}
