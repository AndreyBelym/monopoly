#ifndef CLIENT_HPP_INCLUDED
#define CLIENT_HPP_INCLUDED
namespace client{
    int client_thread(void *data);
    typedef struct {
        const char *host;
        Uint16 port;
        TCPsocket *socket;
    } ClientStartData;
    extern core::Field board[40];
    extern core::Player players[8];
    extern TCPsocket psocks[8];
    extern int players_count,current_player;
    extern bool running;

    void init();
}


#endif // CLIENT_HPP_INCLUDED
