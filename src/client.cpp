#include "sdl.hpp"
#include "client.hpp"
#include <SDL_net.h>
namespace client{
    bool running=true;
    core::Field board[40];
    core::Player players[8];
    TCPsocket psocks[8];
    SDL_mutex *drawlock;
    int current_player=-1,players_count=0;
    Uint32 pnum=0;

    void init(){
        core::init(client::board,client::players);
        drawlock=SDL_CreateMutex();
        current_player=-1;players_count=0;
        pnum=0;
    }

    void move_player(int player,int src,int dst,int step){
        while(client::running&&src!=dst){
            src=core::valid_field(src+step);
            players[player].location=src;
            SDL_mutexP(sdl::lock);
            sdl::redraw();
            SDL_mutexV(sdl::lock);
            SDL_Delay(75);

        }
    }

    int client_thread(void *data){
        printf("%p\n",data);
        ClientStartData param=*((ClientStartData*)data);
         printf("%p\n",data);
        srand(time(NULL));
         printf("%p\n",data);
        //SDL_Delay(rand()%300+2500);
        IPaddress server;
        printf("host resolving:%s;%u\n",param.host,param.port);
        SDLNet_ResolveHost(&server,param.host,param.port);
        printf("host resolved:%ud;%ud\n",server.host,server.port);
        TCPsocket socket=SDLNet_TCP_Open(&server);
        bool running=true;
        while(client::running&&!socket){
            printf("Connection error\n");
            SDL_Delay(rand()%100+400);
            socket=SDLNet_TCP_Open(&server);
        }
        *param.socket=socket;
        if (!client::running)
            return 0;
        printf("Connected!\n");
        core::PacketData pdata;
        //SDLNet_TCP_Send(socket,&pdata,sizeof(pdata));
        int dst=0,src=0,player;
        SDLNet_SocketSet sock=SDLNet_AllocSocketSet(1);
        SDLNet_TCP_AddSocket(sock,socket);
        int i;
        while(client::running&&running){
            while(client::running&&running&&(SDLNet_CheckSockets(sock,500)==-1||
                  !SDLNet_SocketReady(socket)));
            if(client::running&&running&&SDLNet_TCP_Recv(socket,&pdata,
                                                        sizeof(pdata))>=1){
                switch (pdata.type){
                    case core::ACK:
                        player=pdata.curr_player;
                        players[player].type=core::LOCAL;
                        printf("Socket saved for %d\n",player);
                        psocks[player]=socket;
                        break;
                    case core::WINNER:
                        if (player==pdata.curr_player){
                            printf("I'm winner!\n");
                            running=false;
                        }
                        break;
                    case core::LOOSER:
                        if (player==pdata.curr_player){
                            printf("I'm looser!\n");
                            running=false;
                        }

                        players[pdata.curr_player].type=core::BANKROT;
                        break;
                    case core::AUCTION:
                        SDL_mutexP(drawlock);
                        if (pnum<pdata.pnum){
                            printf("update%d\n",pdata.dst);
                            pnum=pdata.pnum;

                            if (!widgets::awindow){
                                printf("new\n");
                                widgets::awindow=new widgets::AuctWindow("Аукцион");
                                printf("new\n");
                            }
                            widgets::awindow->timer=pdata.dst;
                            for (i=0;i<pdata.curr_player;i++){
                                widgets::awindow->price[i]=pdata.moneys[i];
                            }

                            widgets::awindow->show();

                        } else printf("not updated");
                        SDL_mutexV(drawlock);
                        break;

                    case core::DISCONNECT:
                    case core::SERVER_FULL:

                        printf("Server disconnected!\n");
                        running=false;
                        break;
                    case core::UPDATE_PLAYERS:

                        printf("Players update!\n");
                        SDL_mutexP(drawlock);
                        printf("%d,%d\n",pnum,pdata.pnum);
                        if (pnum<pdata.pnum){
                            pnum=pdata.pnum;
                            printf("Updated!%ld\n",sizeof(pdata));
                            for (i=0;i<pdata.curr_player;i++){
                                core::copy_name(pdata.names+21*i,players[i].name);
                                players[i].money=pdata.moneys[i];
                                if (players[i].type==core::UNDEF){
                                    if (psocks[i])
                                        players[i].type=core::LOCAL;
                                    else
                                        players[i].type=core::REMOTE;
                                }
                            }
                            players_count=pdata.dst;

                            SDL_mutexP(sdl::lock);
                            if (pdata.curr_player==players_count)
                                widgets::pwindow->setVisible(false);
                            sdl::redraw();
                            SDL_mutexV(sdl::lock);

                        } else {
                            //SDL_mutexV(drawlock);
                            //printf("Not updated!\n");
                        }
                        SDL_mutexV(drawlock);
                        break;
                    case core::OWNED:
                        SDL_mutexP(sdl::lock);
                        if (pdata.pnum>pnum){
                            pnum=pdata.pnum;
                            board[pdata.dst].owner=pdata.curr_player;
                            sdl::redraw_board();
                            sdl::redraw();
                        }
                        SDL_mutexV(sdl::lock);
                        break;
                    case core::EVENT:
                        current_player=pdata.curr_player;
                        //printf("Packet received player %d!\n",player);
                        dst=pdata.dst;src=pdata.src;
                        //printf("%d\n",SDL_SemValue(drawlock));
                        SDL_mutexP(drawlock);
                        if (pdata.pnum>pnum){
                            pnum=pdata.pnum;
                            printf("%d\n",pdata.pnum);
                            SDL_mutexV(drawlock);
                            //printf("%d\n",SDL_SemValue(drawlock));
                            printf("I paint!%d\n",player);
                            current_player=pdata.curr_player;
                            printf("CLIENT:%d\n",pdata.dst);
                            if (src==core::JAIL_LOCATION&&dst==core::JAIL_LOCATION)
                                players[current_player].jailed=
                                        players[current_player].jailed-1;
                            else{
                                move_player(current_player,src,dst,1);

                                if (board[dst].type==core::TO_JAIL){
                                    int step;
                                    players[current_player].jailed=core::JAIL_3;
                                    if (core::JAIL_LOCATION<dst)
                                        step=-1;
                                    else step=1;
                                    src=dst;
                                    dst=core::JAIL_LOCATION;
                                    move_player(current_player,src,dst,step);

                                }
                            }
                            //SDL_SemPost(drawlock);
                            if (players[pdata.curr_player].type==core::LOCAL){
                                    printf("location: %d\n",pdata.dst);
                                widgets::bwindow->show(players[
                                            pdata.curr_player].location);
                                SDL_mutexP(sdl::lock);
                                sdl::redraw();
                                SDL_mutexV(sdl::lock);
                            }
                        } else SDL_mutexV(drawlock);

                        break;
                    default:
                        printf("client: Unknown packet type!\n");
                }
            } else {
                running=false;
            }
        }
        if (socket){
            pdata.type=core::DISCONNECT;
            SDLNet_TCP_Send(socket,&pdata,sizeof(pdata));
            SDLNet_TCP_Close(socket);
        }
        printf("Client exited!\n");
        return 0;
    }
}
