#include "server.hpp"
#include <time.h>
namespace server{
    Uint32 pnum;
    bool running=true;
    core::Field board[40];
    core::Player players[9];

    TCPsocket psocks[9];
    TCPsocket watchers[16];

    int auct_timer;
    int auct_prices[8];

    SDL_cond *startgame,*nextturn;

    SDL_mutex *startgamem,*nextturnm;
    SDL_mutex *socketsm,*auctm;

    SDLNet_SocketSet clients=SDLNet_AllocSocketSet(2*8);
    int clients_count;int curr_player;
    ServerStartData param;

    void init(){
        core::init(server::board,server::players);
        clients_count=0;curr_player=0;
        pnum=1;
        clients=SDLNet_AllocSocketSet(2*8);
        startgame=SDL_CreateCond();
        nextturn=SDL_CreateCond();
        startgamem=SDL_CreateMutex();
        nextturnm=SDL_CreateMutex();
        socketsm=SDL_CreateMutex();
        auctm=SDL_CreateMutex();
        SDL_mutexP(startgamem);
        SDL_mutexP(nextturnm);
        for(int i=0;i<9;i++){
            psocks[i]=NULL;
        }
    }

    int send_broadcast(core::PacketData pdata){
    int i,failed=0;
    SDL_mutexP(socketsm);
    for (i=0;i<clients_count;i++){
            if(psocks[i]){
                if(SDLNet_TCP_Send(psocks[i],&pdata,sizeof(pdata))<sizeof(pdata))
                    {failed++;
                    printf("failed\n");}

            } else failed++;
        }
    SDL_mutexV(socketsm);
    return failed;
    }

    void send_to_watchers(core::PacketData pdata){
    int i;
    for (i=0;i<16;i++){
            if(watchers[i]){
                SDLNet_TCP_Send(watchers[i],&pdata,sizeof(pdata));
            }
    }
    }
    int auct_thread(void *data){
        core::PacketData pdata;
        int i;
        auct_timer=9;
        for(i=0;i<clients_count;i++){
            auct_prices[i]=0;
        }
        SDL_mutexP(socketsm);
        pdata.pnum=pnum;
        pnum++;
        SDL_mutexV(socketsm);
        pdata.type=core::AUCTION;
        pdata.dst=auct_timer;
        pdata.curr_player=clients_count;
        for (i=0;i<clients_count;i++){
            pdata.moneys[i]=auct_prices[i];
        }
        send_broadcast(pdata);
        while (server::running){
            for (i=0;i<clients_count;i++){
                if(auct_prices[i]!=0){
                    SDL_mutexP(auctm);
                    auct_timer--;
                    SDL_mutexP(socketsm);
                    pdata.pnum=pnum;
                    pnum++;
                    SDL_mutexV(socketsm);
                    pdata.type=core::AUCTION;
                    pdata.dst=auct_timer;
                    pdata.curr_player=clients_count;
                    for (i=0;i<clients_count;i++){
                        pdata.moneys[i]=auct_prices[i];
                    }
                    send_broadcast(pdata);
                    printf("auction server%d\n",pdata.dst);
                    SDL_mutexV(auctm);
                    break;
                }
            }
            SDL_Delay(1000);

            if(auct_timer==0)
                break;
        }
        if(server::running){
            int max=0,maxi=0;
            for (i=0;i<clients_count;i++){
                if(auct_prices[i]>max){
                    max=auct_prices[i];
                    maxi=i;
                }
            }

            board[players[curr_player].location].owner=maxi;

            SDL_mutexP(socketsm);
            pdata.pnum=pnum;
            pnum++;
            SDL_mutexV(socketsm);
            pdata.type=core::OWNED;
            pdata.dst=players[curr_player].location;
            pdata.curr_player=maxi;
            send_broadcast(pdata);

            players[maxi].money-=auct_prices[maxi];

            SDL_mutexP(socketsm);
            pdata.pnum=pnum;
            pnum++;
            SDL_mutexV(socketsm);
            pdata.type=core::UPDATE_PLAYERS;
            pdata.dst=param.pCount;
            pdata.curr_player=clients_count;
            for (i=0;i<clients_count;i++){
                core::copy_name(players[i].name,pdata.names+21*i);
                pdata.moneys[i]=players[i].money;
            }
            send_broadcast(pdata);
        }
        return 0;
    }
    int service_thread(void *data){
        TCPsocket socket=*((TCPsocket*)data),newsocket;
        core::PacketData pdata;
        //int turn=1;
        int i;
        for (i=0;i<8;++i){
            psocks[i]=NULL;
        }
        for (i=0;i<16;++i){
            watchers[i]=NULL;
        }

        while (server::running){
            newsocket=SDLNet_TCP_Accept(socket);
            while (server::running&&!newsocket){
                SDL_Delay(500);
                newsocket=SDLNet_TCP_Accept(socket);
            }
            printf("%d\n",param.pCount);
            if(clients_count<param.pCount){
                if(server::running){
                    for (i=0;i<16;++i){
                        if(!watchers[i]){
                            SDLNet_TCP_AddSocket(clients,newsocket);
                            watchers[i]=newsocket;
                            printf("New client!\n");

                            int j;
                            SDL_mutexP(socketsm);
                            pdata.pnum=pnum;
                            pnum++;
                            pdata.type=core::UPDATE_PLAYERS;
                            pdata.dst=param.pCount;
                            pdata.curr_player=clients_count;
                            for (j=0;j<clients_count;j++){
                                core::copy_name(players[j].name,pdata.names+21*j);
                                pdata.moneys[j]=players[j].money;
                            }
                            SDLNet_TCP_Send(newsocket,&pdata,sizeof(pdata));
                            SDL_mutexV(socketsm);
                            break;
                        }
                    }
                    if (i==16){
                         pdata.type=core::SERVER_FULL;
                         printf("Server full!\n");
                         SDLNet_TCP_Send(newsocket,&pdata,sizeof(pdata));
                         SDLNet_TCP_Close(newsocket);
                    };
                     //++curr_player;
                }
                int i;
                if (SDLNet_CheckSockets(clients,500)!=-1){

                    for (i=0;i<16;++i){
                        if (watchers[i]&&SDLNet_SocketReady(watchers[i])){

                            if(server::running&&SDLNet_TCP_Recv(watchers[i],
                                &pdata,sizeof(pdata))>0)
                            {
                                core::copy_name(pdata.names,
                                        players[clients_count].name);
                                printf("Accepted %s!\n",pdata.names);
                                pdata.curr_player=clients_count;
                                pdata.type=core::ACK;
                                psocks[clients_count]=watchers[i];
                                watchers[i]=NULL;
                                printf("1!\n");
                                SDLNet_TCP_Send(psocks[clients_count],
                                                &pdata,sizeof(pdata));

                                int* x=new int(clients_count);

                                SDL_CreateThread(server_thread,x);
                                clients_count++;
                                int j;
                                SDL_mutexP(socketsm);
                                pdata.pnum=pnum;
                                pnum++;
                                SDL_mutexV(socketsm);
                                pdata.type=core::UPDATE_PLAYERS;
                                pdata.dst=param.pCount;
                                pdata.curr_player=clients_count;
                                for (j=0;j<clients_count;j++){
                                    core::copy_name(players[j].name,pdata.names+21*j);
                                    pdata.moneys[j]=players[j].money;
                                }
                                send_broadcast(pdata);
                                send_to_watchers(pdata);
                                printf("2!%ld\n",sizeof(pdata));
                                if (clients_count==param.pCount){
                                    SDL_CondSignal(startgame);
                                }
                            } else {
                                SDLNet_TCP_DelSocket(clients,watchers[i]);
                                SDLNet_TCP_Close(watchers[i]);
                                watchers[i]=NULL;
                            }

                        }
                    }
                }

            } else {
                pdata.type=core::SERVER_FULL;
                for (i=0;i<16;++i){

                    if (watchers[i]){
                        SDLNet_TCP_Send(watchers[i],&pdata,sizeof(pdata));
                        SDLNet_TCP_DelSocket(clients,watchers[i]);
                        SDLNet_TCP_Close(watchers[i]);
                        watchers[i]=NULL;
                    }
                }
                if (newsocket){
                    SDLNet_TCP_Send(newsocket,&pdata,sizeof(pdata));
                    SDLNet_TCP_DelSocket(clients,newsocket);
                    SDLNet_TCP_Close(newsocket);
                    newsocket=NULL;
                }
            }
        }
        pdata.type=core::DISCONNECT;
        for (i=0;i<16;++i){
            if (watchers[i]){
                SDLNet_TCP_Send(watchers[i],&pdata,sizeof(pdata));
                SDLNet_TCP_DelSocket(clients,watchers[i]);
                SDLNet_TCP_Close(watchers[i]);
                watchers[i]=NULL;
            }
        }
        if (newsocket){
            SDLNet_TCP_Send(newsocket,&pdata,sizeof(pdata));
            SDLNet_TCP_DelSocket(clients,newsocket);
            SDLNet_TCP_Close(newsocket);
            newsocket=NULL;
        }
        SDLNet_TCP_Close(socket);
        SDL_CondSignal(startgame);
        SDL_CondSignal(nextturn);
        printf("Service thread terminated\n");
        return 0;
    }
    int server_thread(void *data){
        bool running=true;
        int player=*((int*)data);
        core::PacketData pdata;
        printf("Server thread %d started!\n",player);
        while (server::running&&running){
            while(server::running&&running&&
                (SDLNet_CheckSockets(clients,500)==-1||
                    !SDLNet_SocketReady(psocks[player])));
                //SDL_Delay(500);
               // printf("Nope\n");
              //printf("Socket not ready\n");
            if (running) printf("P...\n");
            if(server::running&&running){
            if(SDLNet_TCP_Recv(psocks[player],&pdata,sizeof(pdata))==sizeof(pdata)){
                printf("%d\n",pdata.type);
                switch (pdata.type){
                    case core::ACK:
                        if (player==curr_player){
                            printf("Acknowledged!\n");
                            SDL_CondSignal(nextturn);
                        }

                        break;
                    case core::AUCTION:
                        auct_prices[player]=pdata.moneys[0];
                        SDL_mutexP(auctm);
                        auct_timer=10;
                        SDL_mutexV(auctm);
                        break;
                    case core::OWNED:
                        if (player==curr_player&&
                           (board[players[player].location].type==core::STREET||
                           board[players[player].location].type==core::RAILWAY||
                           board[players[player].location].type==core::PLANT)
                            &&board[players[player].location].owner==-1){

                            printf("OWNED:%d\n",
                                server::board[players[player].location].price);
                            players[player].money-=
                                server::board[players[player].location].price;
                            server::board[players[player].location].owner=player;
                            SDL_mutexP(socketsm);
                            pdata.pnum=pnum;
                            pnum++;
                            SDL_mutexV(socketsm);
                            pdata.type=core::OWNED;
                            pdata.dst=players[player].location;
                            pdata.curr_player=player;
                            send_broadcast(pdata);

                            int j;
                            SDL_mutexP(socketsm);
                            pdata.pnum=pnum;
                            pnum++;
                            SDL_mutexV(socketsm);
                            pdata.type=core::UPDATE_PLAYERS;
                            pdata.dst=param.pCount;
                            pdata.curr_player=clients_count;
                            for (j=0;j<clients_count;j++){
                                core::copy_name(players[j].name,pdata.names+21*j);
                                pdata.moneys[j]=players[j].money;
                            }
                            send_broadcast(pdata);

                        }

                        break;
                    case core::DISCONNECT:
                        printf("Client disconnected!\n");
                        SDL_mutexP(socketsm);
                        SDLNet_TCP_DelSocket(clients,psocks[player]);
                        SDLNet_TCP_Close(psocks[player]);
                        psocks[player]=NULL;
                        SDL_mutexV(socketsm);
                        running=false;
                        break;
                    case core::NOT_OWNED:
                        SDL_CreateThread(auct_thread,NULL);
                        break;
                    default:
                        printf("server: Unknown packet type!\n");
                }
            } else {
                printf("Error!");
                running=false;
            }
            }
        }
        if (psocks[player]){
            pdata.type=core::DISCONNECT;
            SDLNet_TCP_Send(psocks[player],&pdata,sizeof(pdata));
            SDL_mutexP(socketsm);
            SDLNet_TCP_DelSocket(clients,psocks[player]);
            SDLNet_TCP_Close(psocks[player]);
            psocks[player]=NULL;
            SDL_mutexV(socketsm);
        }
        SDL_CondSignal(startgame);
        SDL_CondSignal(nextturn);
        printf("Server thread terminated\n");
        return 0;
    }
    int main_server_thread(void *data){
        core::PacketData pdata;
        param=*((ServerStartData*)data);

        server::init();
        srand(time(NULL));
        IPaddress address;
        SDLNet_ResolveHost(&address,INADDR_ANY,param.port);
        TCPsocket socket=SDLNet_TCP_Open(&address);
        if (socket){
            printf("Server started!\n");
            SDL_CreateThread(service_thread,&socket);
            SDL_CondWait(startgame,startgamem);

            printf("Game started!\n");
            int players_failed=0;
            while(server::running){
                if (psocks[curr_player]){
                    SDL_mutexP(socketsm);
                    pdata.pnum=pnum;
                    pnum++;
                    SDL_mutexV(socketsm);
                    pdata.type=core::EVENT;

                    pdata.curr_player=curr_player;
                    if (players_failed==param.pCount-1){
                        pdata.type=core::WINNER;
                        printf("Winner is founded\n");
                        send_broadcast(pdata);
                        SDL_mutexP(socketsm);
                        SDLNet_TCP_DelSocket(clients,psocks[curr_player]);
                        SDLNet_TCP_Close(psocks[curr_player]);
                        psocks[curr_player]=NULL;
                        SDL_mutexV(socketsm);
                        server::running=false;
                        break;
                    }

                    printf("cp-%d\n",curr_player);
                    pdata.dice1=rand()%6+1;
                    pdata.dice2=rand()%6+1;
                    if (!players[curr_player].jailed){
                        pdata.src=players[curr_player].location;
                        pdata.dst=core::valid_field(pdata.src+pdata.dice1+
                            pdata.dice2);
                    } else {
                        pdata.src=core::JAIL_LOCATION;
                        pdata.dst=core::JAIL_LOCATION;
                        players[curr_player].jailed--;
                    }

                    printf("SERVER:%d\n",pdata.dst);
                    if (players[curr_player].location>pdata.dst){
                        players[curr_player].money+=200;
                    }
                    players[curr_player].location=pdata.dst;
                    if (board[players[curr_player].location].type==core::TO_JAIL){
                        players[curr_player].location=core::JAIL_LOCATION;
                        players[curr_player].jailed=core::JAIL_3;
                    }
                    printf("%d\n",board[players[curr_player].location].owner);
                    if(board[players[curr_player].location].owner!=-1){
                            players[curr_player].money-=server::board[players[
                                    curr_player].location].rent[board[players[
                                    curr_player].location].houses_num];
                            players[board[players[curr_player].location].owner].money+=
                            board[players[curr_player].location].rent[board[
                                players[curr_player].location].houses_num];
                            printf("Payed\n");
                    }
                    switch (board[players[curr_player].location].type){
                        case core::FEE:
                            players[curr_player].money-=server::board[players[
                                curr_player].location].price;
                            break;
                        default:;
                    }
                    if(send_broadcast(pdata)==param.pCount){
                        printf("All clients are dead!\n");
                        server::running=false;
                        break;
                    }

                    int j;
                    SDL_mutexP(socketsm);
                    pdata.pnum=pnum;
                    pnum++;
                    SDL_mutexV(socketsm);
                    pdata.type=core::UPDATE_PLAYERS;
                    pdata.dst=param.pCount;
                    pdata.curr_player=clients_count;
                    for (j=0;j<clients_count;j++){
                        core::copy_name(players[j].name,pdata.names+21*j);
                        pdata.moneys[j]=players[j].money;
                    }
                    if(send_broadcast(pdata)==param.pCount){
                        printf("All clients are dead!\n");
                        server::running=false;
                        break;
                    }
                    SDL_CondWait(nextturn,nextturnm);
                    printf("money:%ld\n",players[curr_player].money);
                    if (players[curr_player].money<0){
                        pdata.type=core::LOOSER;
                        send_broadcast(pdata);
                        SDL_mutexP(socketsm);
                        SDLNet_TCP_DelSocket(clients,psocks[curr_player]);
                        SDLNet_TCP_Close(psocks[curr_player]);
                        psocks[curr_player]=NULL;
                        SDL_mutexV(socketsm);
                        players_failed+=1;
                    } else players_failed=0;
                } else {players_failed+=1;}
                if (players_failed==param.pCount)
                    server::running=false;
                curr_player++;
                if(curr_player>=param.pCount)
                    curr_player%=param.pCount;
            };
        } else {
            printf("Server not created\n");
        }
        printf("Server exited!\n");
        return 0;
    }
}
