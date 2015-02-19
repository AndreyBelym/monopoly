#ifndef SDL_HPP_INCLUDED
#define SDL_HPP_INCLUDED
/*
 * Code that sets up an SDL application with Guichan using the
 * Guichan SDL back end.
 */
#include "server.hpp"
#include <guichan.hpp>
#include <guichan/sdl.hpp>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <SDL_net.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_rwops.h>
#include "widgets.hpp"
namespace sdl{
    const int fieldw=72;
    const int fieldh=54;
    const int boardw=800;
    const int boardh=600;
    const int tokenw=19;
    const int tokenh=19;
    const int groupmarg=32;
    extern int marginl,marginr,margint,marginb;

    typedef struct {
        SDL_Surface *owned,*token,*tokenbig;
        TCPsocket socket;
        SDL_sem *drawing;
    } PlayerView;


    extern gcn::Gui* gui;
    void init();
    void halt();
    void run();
    int client_thread(void *data);


    void redraw();

    void redraw_board();
    //extern int current_player,players_count;
    extern SDL_Thread *server;
    extern SDL_mutex *lock;
    extern bool running;
    extern bool restart;
    //extern SDL_mutex *drawlock;
}

#endif
