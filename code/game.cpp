#include "game_platform.h"
#include <windows.h>

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    OutputDebugStringA("Hello, world.\n");
}