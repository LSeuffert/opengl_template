struct win32_game_code
{
    HMODULE game_code_dll;
    game_update_and_render *UpdateAndRender;
    FILETIME dll_last_timestamp;
    b32 valid;
};