#ifdef __cplusplus
    #include <cstdlib>
#else
    #include <stdlib.h>
#endif
#ifdef __APPLE__
#include <SDL/SDL.h>
#else
#include <SDL.h>
#include <SDL_net.h>
#endif
#include "../server.hpp"
#include <stdio.h>
int main ( int argc, char** argv )
{
    // initialize SDL video
    if ( SDL_Init(SDL_INIT_TIMER) < 0 )
    {
        printf( "Unable to init SDL: %s\n", SDL_GetError() );
        return 1;
    }
    SDLNet_Init();
    // make sure SDL cleans up before exit
    atexit(SDL_Quit);
    server::ServerStartData param;
    param.pCount=8;
    param.port=7887;
    SDL_Thread *server=SDL_CreateThread(server::main_server_thread,&param);
    while(getchar()!='\n');
    printf("Exiting...\n");
    server::running=false;
    SDL_WaitThread(server,NULL);
    // all is well ;)
    printf("Exited cleanly\n");
    return 0;
}
