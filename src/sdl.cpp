#include  "sdl.hpp"
#include "client.hpp"
namespace sdl
{
    bool restart;
    bool running=true;
    int marginl=4,marginr=4,margint=3,marginb=3;
    bool fullscreen=false;
    gcn::Gui* gui;
    SDL_Thread *server=NULL;
    PlayerView Players[8];
    //core::Field Board[40];
    SDL_Surface* screen,*noowned,*boardsurf;
    TTF_Font *hugeFont,*normFont;
    // All back ends contain objects to make Guichan work on a
    // specific target - in this case SDL - and they are a Graphics
    // object to make Guichan able to draw itself using SDL, an
    // input objec to make Guichan able to get user input using SDL
    // and an ImageLoader object to make Guichan able to load images
    // using SDL.
    gcn::SDLGraphics* graphics;
    gcn::SDLInput* input;
    gcn::SDLImageLoader* imageLoader;
    SDL_mutex *lock;

    SDL_Rect get_field_rect(int num);
    SDL_Rect get_token_rect(int,int);
    SDL_Surface* bmp;
    /**
     * Initialises the SDL application. This function creates the global
     * Gui object that can be populated by various examples.
     */

     SDL_Rect get_field_rect(int num){
        SDL_Rect rect;
        int line_num=num/10,field_num=num%10;
        rect.w=fieldw;rect.h=fieldh;
        switch(line_num){
            case 2:
               rect.x=fieldw*field_num+marginl;
               rect.y=margint;
               break;
            case 3:
               rect.y=fieldh*field_num+margint;
               rect.x=boardw-marginr-fieldw;
               break;
            case 0:
               rect.x=boardw-fieldw*(field_num+1)-marginr;
               rect.y=boardh-marginb-fieldh;
               break;
            case 1:
               rect.y=boardh-fieldh*(field_num+1)-marginb;
               rect.x=marginl;
               break;
        }
        return rect;
    }
    SDL_Rect get_token_rect(int field_num,int num){
        SDL_Rect field=get_field_rect(field_num);
        field.y+=fieldh-tokenh;
        field.x+=((fieldw-tokenw)/8)*num;
        return field;
    }
    SDL_Rect get_player_rect(int num){
        SDL_Rect player;
        player.x=marginl+fieldw+groupmarg;
        player.w=boardw-marginr-groupmarg-fieldw-player.x;
        player.y=margint+fieldh+groupmarg;
        player.h=(boardh-marginb-groupmarg-fieldh-player.y)/8;
        player.y+=player.h*num;
        return player;
    }
    void redraw_board();
    void init()
    {
        // We simply initialise SDL as we would do with any SDL application.
        SDL_Init(SDL_INIT_VIDEO);
        screen = SDL_SetVideoMode(boardw, boardh, 32, SDL_HWSURFACE|SDL_DOUBLEBUF);

        // We want unicode for the SDLInput object to function properly.
        if(SDLNet_Init()==-1)
        {
            std::cout<<"SDLNet_Init: "<<SDLNet_GetError()<<"\n";
            exit(2);
        }
        IMG_Init(IMG_INIT_PNG);
        TTF_Init();
        hugeFont=TTF_OpenFont("BadScript-Regular.ttf",23);
        normFont=TTF_OpenFont("BadScript-Regular.ttf",15);
        SDL_EnableUNICODE(1);
        // We also want to enable key repeat.
        SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

        // Now it's time to initialise the Guichan SDL back end.

        imageLoader = new gcn::SDLImageLoader();
        // The ImageLoader Guichan should use needs to be passed to the Image object
        // using a static function.
        gcn::Image::setImageLoader(imageLoader);
        graphics = new gcn::SDLGraphics();
        // The Graphics object needs a target to draw to, in this case it's the
        // screen surface, but any surface will do, it doesn't have to be the screen.
        graphics->setTarget(screen);
        input = new gcn::SDLInput();

        // Now we create the Gui object to be used with this SDL application.
        gui = new gcn::Gui();
        // The Gui object needs a Graphics to be able to draw itself and an Input
        // object to be able to check for user input. In this case we provide the
        // Gui object with SDL implementations of these objects hence making Guichan
        // able to utilise SDL.
        gui->setGraphics(graphics);
        gui->setInput(input);
        bmp=IMG_Load("board.png");
        //SDL_SetAlpha(bmp,SDL_SRCALPHA,SDL_ALPHA_TRANSPARENT);
        boardsurf=SDL_CreateRGBSurface(SDL_SRCALPHA,boardw,boardh,32,
                               screen->format->Rmask,screen->format->Gmask,
                               screen->format->Bmask,screen->format->Amask);
        printf("%d,%d\n",boardsurf->clip_rect.w,boardsurf->clip_rect.h);
        bmp=IMG_Load("board.png");
        int i; char name[50];
        std::cout<<"Loading players data!\n";
        for (i=0;i<8;++i){
            sprintf(name,"token%d.png",i);
            Players[i].token=IMG_Load(name);
            sprintf(name,"tokenbig%d.png",i);
            Players[i].tokenbig=IMG_Load(name);
            sprintf(name,"owned%d.png",i);
            Players[i].owned=IMG_Load(name);
            Players[i].socket=NULL;
            Players[i].drawing=SDL_CreateSemaphore(1);

            //Players[i].turn=0;
            printf(IMG_GetError());
            SDL_SetAlpha(Players[i].token,SDL_SRCALPHA,SDL_ALPHA_TRANSPARENT);
        }
        lock=SDL_CreateMutex();
        noowned=IMG_Load("noowned.png");
        printf(IMG_GetError());
        printf("Loading fields data!\n");
        redraw_board();
    }

    /**
     * Halts the SDL application.
     */
    void halt()
    {
        delete gui;

        delete imageLoader;
        delete input;
        delete graphics;

        SDL_Quit();
    }
    void redraw_board(){
        SDL_Rect dest;
        SDL_FillRect(boardsurf,NULL,SDL_MapRGB(boardsurf->format,0xFF,0xFF,0xFF));
        int i;
        for (i=0;i<40;++i){
            if (client::board[i].type==core::STREET or
                client::board[i].type==core::RAILWAY or
                client::board[i].type==core::PLANT){
                dest=get_field_rect(i);
                //printf("%d-%d\n",i,client::board[i].owner);
                if (client::board[i].owner==-1){
                    if (SDL_BlitSurface(noowned, NULL, boardsurf, &dest))
                        printf(SDL_GetError());
                    //printf("%d-%d\n",i,Board[i].owner);
                    }
                else{
                    if (SDL_BlitSurface(Players[client::board[i].owner].owned
                            , NULL, boardsurf, &dest))
                        printf(SDL_GetError());
                    //printf("FAIL:%d-%d\n",i,Board[i].owner);
                    }
            }
        }
        dest.x = 0;
        dest.y = 0;
        dest.w = bmp->w;
        dest.h = bmp->h;
        //SDL_LockSurface(board);

        if (SDL_BlitSurface(bmp, NULL, boardsurf, NULL))
            printf(SDL_GetError());
        //SDL_BlitSurface(board, NULL, screen, NULL);
        //SDL_UnlockSurface(board);
    }
    void redraw(){
        //SDL_FillRect(screen,NULL,SDL_MapRGB(screen->format,0xFF,0xFF,0xFF));
        int i;
        SDL_BlitSurface(boardsurf,NULL,screen,NULL);
        //printf("%d,%d\n",Players[0].token_rect.x,Players[0].token_rect.y);
        SDL_Rect dest;
        for (i=0;i<client::players_count;++i){
            dest=get_token_rect(client::players[i].location,i);
            SDL_BlitSurface(Players[i].token,NULL,screen,&dest);
            dest=get_player_rect(i);
            if (i==client::current_player){
                SDL_FillRect(screen,&dest,0xAAAAAAFF);
            }
            SDL_BlitSurface(Players[i].tokenbig,NULL,screen,&dest);
            dest.x+=60;
            SDL_Color black,red;
            black.b=00;black.g=00;black.r=0;
            red.b=00;red.g=00;red.r=0xFF;
            SDL_Surface *label=TTF_RenderUTF8_Blended(hugeFont,
                    client::players[i].name,black);
            SDL_BlitSurface(label,NULL,screen,&dest);
            dest.y+=30;
            switch (client::players[i].type){
                case core::LOCAL:
                    label=TTF_RenderUTF8_Blended(normFont,"Локальный\0",black);
                    SDL_BlitSurface(label,NULL,screen,&dest);
                    break;
                case core::REMOTE:
                    label=TTF_RenderUTF8_Blended(normFont,"Удаленный\0",black);
                    SDL_BlitSurface(label,NULL,screen,&dest);
                    break;
                case core::BANKROT:
                    label=TTF_RenderUTF8_Blended(normFont,"Банкрот\0",black);
                    SDL_BlitSurface(label,NULL,screen,&dest);
                    break;
                default:
                    ;
            }
            dest.y-=20;
            dest.x+=150;
            char money[50];
            int size=sprintf(money,"Баланс:%ld$",client::players[i].money);
            money[size]='\0';
            if (client::players[i].money<0){
                label=TTF_RenderUTF8_Blended(normFont,money,red);
            }
            else {
                label=TTF_RenderUTF8_Blended(normFont,money,black);
            }
            SDL_BlitSurface(label,NULL,screen,&dest);
        }
        gui->logic();
        // Now we let the Gui object draw itself.
        gui->draw();
        // Finally we update the screen.
        SDL_Flip(screen);
    }

    /**
     * Runs the SDL application.
     */

    void run()
    {


        SDL_Event event;

        std::cout<<"Start complete\n";
        while(running&&SDL_WaitEvent(&event))
        {
        // Check user input
            //printf("event\n");
            if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    client::running=false;
                    server::running=false;
                    running = false;
                    break;
                }
                if (event.key.keysym.sym == SDLK_f)
                {
                    if (event.key.keysym.mod & KMOD_CTRL)
                    {
                        // Works with X11 only
                        SDL_mutexP(lock);
                        fullscreen=!fullscreen;
                        if (fullscreen)
                            screen = SDL_SetVideoMode(boardw, boardh, 32,
                            SDL_HWSURFACE|SDL_DOUBLEBUF|SDL_FULLSCREEN);
                        else screen = SDL_SetVideoMode(boardw, boardh, 32,
                            SDL_HWSURFACE|SDL_DOUBLEBUF);
                        SDL_mutexV(lock);
                        //SDL_WM_ToggleFullScreen(screen);
                    }
                }
            }
            else if(event.type == SDL_QUIT)
            {
                client::running=false;
                server::running=false;
                running = false;

                break;
            }

            // After we have manually checked user input with SDL for
            // any attempt by the user to halt the application we feed
            // the input to Guichan by pushing the input to the Input
            // object.
            input->pushInput(event);

            // Now we let the Gui object perform its logic.
            //gui->logic();
            SDL_mutexP(lock);
            redraw();
            SDL_mutexV(lock);
            // Finally we update the screen.

        }
        /*SDL_WaitThread(client0, NULL);
        SDL_WaitThread(client1, NULL);
        SDL_WaitThread(client2, NULL);
        SDL_WaitThread(client3, NULL);

        SDL_WaitThread(client4, NULL);
        SDL_WaitThread(client5, NULL);
        SDL_WaitThread(client6, NULL);
        SDL_WaitThread(client7, NULL);
        */
        if (server)
            SDL_WaitThread(server,NULL);

    }
}

