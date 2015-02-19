#ifndef SERVER_HPP_INCLUDED
#define SERVER_HPP_INCLUDED
//#include "core.hpp"
#include <SDL_net.h>
#include <iostream>
#include "server.hpp"
#include "core.hpp"
namespace server{

    typedef struct {
        Uint16 port;
        int pCount;
    } ServerStartData;



    int send_broadcast(core::PacketData pdata);

    void send_to_watchers(core::PacketData pdata);

    int service_thread(void *data);

    int server_thread(void *data);

    int main_server_thread(void *data);


    extern bool running;
    extern core::Field board[40];
    extern core::Player players[9];

}



#endif // SERVER_HPP_INCLUDED
