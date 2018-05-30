#include <SDL2/SDL.h>

int main(int argc, char** argv)
{
    int devcnt = SDL_GetNumVideoDrivers();
    int aucnt = SDL_GetNumAudioDrivers();
    int i;
    printf("Num video drivers: %d\n", devcnt);
    for (i = 0; i < devcnt; ++i)
    {
        const char* name = SDL_GetVideoDriver(i);
        printf("%d: %s\n", i, name);
    }
    printf("Num audio drivers: %d\n", aucnt);
    for (i = 0; i < aucnt; ++i)
    {
        const char* name = SDL_GetAudioDriver(i);
        printf("%d: %s\n", i, name);
    }
}

