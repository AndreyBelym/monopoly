#include "widgets.hpp"
#include "sdl.hpp"
#include "client.hpp"
#include <sstream>
namespace widgets
{
    std::string intToString(int i)
    {
        std::stringstream ss;
        std::string s;
        ss << i;
        s = ss.str();

        return s;
    }
    void start_client();
    //gcn::ImageFont* font;
    gcn::SDLUTF8TrueTypeFont *font;
    gcn::Container* top,*field;
    gcn::Button* button,*exitbutton,*discbutton;
    LoginWindow *window;
    PlayersWindow *pwindow;
    FieldWindow *bwindow;
    AuctWindow *awindow;
    gcn::Icon *field_info[40];
    class ButtonListener: public gcn::ActionListener
    {
        void action(const gcn::ActionEvent &actionEvent)
        {
            if (client::psocks[client::current_player]){
                bwindow->setVisible(false);
                core::PacketData pdata;
                pdata.type=core::ACK;
                printf("player:%d\n",client::current_player);
                SDLNet_TCP_Send(client::psocks[client::current_player],
                        &pdata,sizeof(pdata));
            }
        }

    } buttonListener;

    class CreateListener: public gcn::ActionListener
    {
        void action(const gcn::ActionEvent &actionEvent)
        {
            window->client_num->setVisible(window->create->isSelected());
            window->client_numl->setVisible(window->create->isSelected());
            window->serv->setEnabled(!window->create->isSelected());
            if (window->create->isSelected()){
                window->serv->setText("localhost");
            }

        }
    } *createListener;
    class ConnectListener: public gcn::ActionListener
    {
        void action(const gcn::ActionEvent &actionEvent)
        {
            if (window->create->isSelected()){
                server::running=true;
                printf("1\n");
                server::ServerStartData *param=new server::ServerStartData;
                param->port=atoi(window->port->getText().c_str());
                param->pCount=atoi(window->client_num->getText().c_str());
                printf("2\n");
                if (sdl::server){
                    //SDL_WaitThread(sdl::server,NULL);
                    sdl::server=NULL;
                }
                sdl::server=SDL_CreateThread(server::main_server_thread,param);
                printf("3\n");
            };
            client::running=true;
            start_client();
            printf("client started");
            window->setVisible(false);
            pwindow->setVisible(true);


        }
    } *connectListener;

    class AddPlayerListener: public gcn::ActionListener{
        void action(const gcn::ActionEvent &actionEvent)
        {
            core::PacketData pdata;
            core::copy_name(pwindow->newplayer->getText().c_str(),pdata.names);
            printf("%p,%p,%lu\n",&pdata.pnum,&pdata.names[1],sizeof(pdata));
            pdata.type=core::ACK;
            SDLNet_TCP_Send(pwindow->socket,&pdata,sizeof(pdata));
            printf("transmitted!\n");
            start_client();

        }
    } *addplayerListener;

    class BuyYesListener: public gcn::ActionListener
    {
        void action(const gcn::ActionEvent &actionEvent)
        {
            if (client::psocks[client::current_player]){
                core::PacketData pdata;
                pdata.type=core::OWNED;
                //printf("player:%d\n",sdl::current_player);
                SDLNet_TCP_Send(client::psocks[client::current_player],
                        &pdata,sizeof(pdata));
                bwindow->setVisible(false);
            }
        }
    } *buyyesListener;

    class BuyNoListener: public gcn::ActionListener
    {
        void action(const gcn::ActionEvent &actionEvent)
        {
            if (client::psocks[client::current_player]){
                core::PacketData pdata;
                pdata.type=core::NOT_OWNED;
                SDLNet_TCP_Send(client::psocks[client::current_player],
                        &pdata,sizeof(pdata));
            }
            bwindow->setVisible(false);

        }
    } *buynoListener;

    class SubmitAuctListener: public gcn::ActionListener
    {
        void action(const gcn::ActionEvent &actionEvent)
        {

            int    player=atoi(actionEvent.getId().c_str());
            printf("action%d\n",player);
            if (client::psocks[player]){
                core::PacketData pdata;
                pdata.type=core::AUCTION;
                pdata.moneys[0]=atoi(awindow->tprice[player]->getText().c_str());
                SDLNet_TCP_Send(client::psocks[player],&pdata,sizeof(pdata));
            } else printf("no socks\n");
        }
    } *submitAuctListener;

    class CloseFieldListener: public gcn::ActionListener
    {
        void action(const gcn::ActionEvent &actionEvent)
        {

            bwindow->setVisible(false);
        }
    } *closeFieldListener;

    class ExitListener: public gcn::ActionListener
    {
        void action(const gcn::ActionEvent &actionEvent)
        {
            client::running=false;
            server::running=false;
            sdl::running=false;
        }
    } *exitListener;

    class DisconnListener: public gcn::ActionListener
    {
        void action(const gcn::ActionEvent &actionEvent)
        {
            client::running=false;
            server::running=false;
            sdl::restart=true;
            sdl::running=false;
        }
    } *disconnListener;

    class FieldClickListener: public gcn::MouseListener{
        void mouseClicked(gcn::MouseEvent& mouseEvent)
        {
            printf("clicked\n");
            int x=mouseEvent.getX();
            int y=mouseEvent.getY();
            if (y>sdl::boardh-sdl::marginb-sdl::fieldh){
                bwindow->show(10-(x-sdl::marginl)/sdl::fieldw);
                printf("%d\n",10-(x-sdl::marginl)/sdl::fieldw);
            }
             else if (x>sdl::boardw - sdl::marginr-sdl::fieldw){
                bwindow->show(30+(y-sdl::margint)/sdl::fieldh);
                printf("%d\n",30+(y-sdl::margint)/sdl::fieldh);
            } else if (y<sdl::margint+sdl::fieldh){
                bwindow->show(20+(x-sdl::marginl)/sdl::fieldw);
                printf("%d\n",20+(x-sdl::marginl)/sdl::fieldw);
            } else if (x<sdl::marginl+sdl::fieldw){
                bwindow->show(20-(y-sdl::margint)/sdl::fieldh);
                printf("%d\n",20-(y-sdl::margint)/sdl::fieldh);
            }
        }

    } *fieldClickListener;

    class ListListener: public gcn::WidgetListener
    {
        void widgetResized(const gcn::Event& event) {
            pwindow->newplayer->setY(10+pwindow->players->getHeight());
            pwindow->addplayer->setY(10+pwindow->players->getHeight());
            pwindow->resizeToContent();
            pwindow->setX((sdl::boardw-pwindow->getWidth())/2);
            pwindow->setY((sdl::boardh-pwindow->getHeight())/2);
            //this->add(addplayer,80,10+players->getHeight());
        }
    } *listListener;

    class PlayersList: public gcn::ListModel
    {
        int getNumberOfElements(){
            return client::players_count;
        }

        std::string getElementAt(int i){
            return std::string(client::players[i].name);
        }
    } *playersList;


    /**
     * Initialises the widgets example by populating the global Gui
     * object.
     */

    void start_client(){
        client::ClientStartData *param=new client::ClientStartData;
        param->host=window->serv->getText().c_str();
        param->port=atoi(window->port->getText().c_str());
        param->socket=&pwindow->socket;
        printf("starting client\n");
        SDL_CreateThread(client::client_thread,param);
    }
    void init()
    {
        // We first create a container to be used as the top widget.
        // The top widget in Guichan can be any kind of widget, but
        // in order to make the Gui contain more than one widget we
        // make the top widget a container.
        int i;
        fieldClickListener=new FieldClickListener;
        top = new gcn::Container();
        top->setOpaque(false);
        // We set the dimension of the top container to match the screen.
        top->setDimension(gcn::Rectangle(0, 0, sdl::boardw, sdl::boardh));
        // Finally we pass the top widget to the Gui object.
        sdl::gui->setTop(top);

        field=new gcn::Container();
        field->setSize(sdl::boardw,sdl::boardh);
        field->addMouseListener(fieldClickListener);
        field->setOpaque(false);
        top->add(field,0,0);
        // Widgets may have a global font so we don't need to pass the
        // font object to every created widget. The global font is static.
        font=new gcn::SDLUTF8TrueTypeFont("OpenSans-Regular.ttf",12);
        gcn::Widget::setGlobalFont(font);

        // Now we create the widgets

        button = new gcn::Button("Завершить ход");
        button->setWidth(3*sdl::fieldw-sdl::groupmarg);
        button->setHeight(2*sdl::fieldh-sdl::groupmarg);
        button->addActionListener(&buttonListener);

        exitbutton = new gcn::Button("Выход");
        exitbutton->setWidth(1*sdl::fieldw);
        exitbutton->setHeight(1*sdl::fieldh);
        exitListener=new ExitListener;
        exitbutton->addActionListener(exitListener);

        discbutton = new gcn::Button("Отключиться");
        discbutton->setWidth(2*sdl::fieldw-sdl::groupmarg);
        discbutton->setHeight(1*sdl::fieldh);
        disconnListener=new DisconnListener;
        discbutton->addActionListener(disconnListener);

        window = new LoginWindow("Окно входа");
        window->setBaseColor(gcn::Color(255, 150, 200, 190));

        pwindow=new PlayersWindow("Список игроков");
        pwindow->setBaseColor(gcn::Color(255, 150, 200, 190));
        pwindow->setVisible(false);

        bwindow=new FieldWindow("Информация о поле");
        bwindow->setBaseColor(gcn::Color(255, 150, 200, 190));
        bwindow->setVisible(false);

        awindow=NULL;
        for(i=0;i<40;i++){
            char name[30];
            int len=sprintf(name,"groupinfo%d.bmp",i);
            name[len]='\0';
            field_info[i]=new gcn::Icon(gcn::Image::load(name));
        }
        //window->resizeToContent();
        top->add(button, sdl::boardw-4*sdl::fieldw-sdl::marginl,
                sdl::boardh-3*sdl::fieldh-sdl::marginb);
        top->add(exitbutton, sdl::boardw-4*sdl::fieldw-sdl::marginl,
                sdl::boardh-4*sdl::fieldh-sdl::marginb);
        top->add(discbutton, sdl::boardw-3*sdl::fieldw-sdl::marginl,
                sdl::boardh-4*sdl::fieldh-sdl::marginb);
        top->add(window,(sdl::boardw-window->getWidth())/2,
                (sdl::boardh-window->getHeight())/2);
        top->add(pwindow,(sdl::boardw-pwindow->getWidth())/2,
                (sdl::boardh-pwindow->getHeight())/2);
        top->add(bwindow,(sdl::boardw-bwindow->getWidth())/2,
                (sdl::boardh-bwindow->getHeight())/2);


    }

    /**
     * Halts the widgets example.
     */
    void halt()
    {
        delete font;
        delete top;
        delete button;
        delete window;
    }
    LoginWindow::LoginWindow(const std::string& caption)
        :Window(caption)
    {
        play=new gcn::Button("Подключиться");
        connectListener=new ConnectListener;
        play->addActionListener(connectListener);
        create=new gcn::CheckBox("Создать сервер",true);
        serv=new gcn::TextField("127.000.000.001");servl=new gcn::Label("Сервер: ");
        serv->setEnabled(false);
        port=new gcn::TextField("7887");portl=new gcn::Label("Порт: ");
        client_num=new gcn::TextField("8");client_numl=new gcn::Label("Игроков: ");
        createListener=new CreateListener;
        create->addActionListener(createListener);
        this->add(play,30,90);
        this->add(create,5,50);
        this->add(servl,5,5);this->add(serv,10,20);
        this->add(portl,120,5);this->add(port,125,20);
        this->add(client_numl,27,68);this->add(client_num,90,65);

        this->resizeToContent();
        serv->setText("localhost");


    }
    LoginWindow::~LoginWindow()
    {
        delete play;
        delete create;
    }
    PlayersWindow::PlayersWindow(const std::string& caption)
        :Window(caption)
        {
        playersList=new PlayersList;
        players=new gcn::ListBox(playersList);
        addplayer=new gcn::Button("Добавить");
        newplayer=new gcn::UTF8TextField("Имярек   ");
        addplayerListener=new AddPlayerListener;
        addplayer->addActionListener(addplayerListener);
        listListener= new ListListener;
        players->addWidgetListener(listListener);
        this->setMovable(false);
        this->add(players,5,5);
        this->add(newplayer,5,10+players->getHeight());
        this->add(addplayer,80,10+players->getHeight());

        this->resizeToContent();
        //this->setHeight(300);
    }
    PlayersWindow::~PlayersWindow()
    {

    }
    FieldWindow::FieldWindow(const std::string& caption)
        :gcn::Window(caption)
        {

            int i;
            closeFieldListener=new CloseFieldListener;
            yes=new gcn::Button("Да");
            no=new gcn::Button("Нет");
            info1=new gcn::Label("");
            info2=new gcn::Label("");
            close1=new gcn::Button("Закрыть"); close1->addActionListener(closeFieldListener);
            close2=new gcn::Button("Закрыть"); close2->addActionListener(closeFieldListener);
            buyhouse=new gcn::Button("Купить дом");
            sellhouse=new gcn::Button("Продать дом");
            mortgage=new gcn::Button("Заложить");
            sellfield=new gcn::Button("Продать");
            buyfield=new gcn::Button("Купить");

            this->add(info1,0,300);
            info1->setWidth(220);
            info1->setAlignment(gcn::Graphics::CENTER);

            this->add(info2,0,315);
            info2->setWidth(220);
            info2->setAlignment(gcn::Graphics::CENTER);

            noowned= new gcn::Container();
            noowned->setSize(100,30);
            noowned->setOpaque(false);
            noowned->setVisible(false);
            owned= new gcn::Container();
            owned->setOpaque(false);
            owned->setSize(180,100);
            owned->setVisible(false);
            foreign= new gcn::Container();
            foreign->setSize(130,30);
            foreign->setOpaque(false);
            foreign->setVisible(false);

            nope= new gcn::Container();
            nope->setSize(65,30);
            nope->setOpaque(false);
            nope->setVisible(false);


            noowned->add(yes,20,5);
            noowned->add(no,50,5);


            owned->add(buyhouse,5,5);
            owned->add(sellhouse,80,5);
            owned->add(sellfield,52,35);
            owned->add(mortgage,48,65);

            foreign->add(buyfield,5,5);
            foreign->add(close1,60,5);

            nope->add(close2,5,5);

            buyyesListener=new BuyYesListener;
            buynoListener=new BuyNoListener;
            yes->addActionListener(buyyesListener);
            no->addActionListener(buynoListener);
            this->resizeToContent();
            this->setWidth(225);
            this->add(noowned,(this->getWidth()-noowned->getWidth())/2,330);
            this->add(owned,(this->getWidth()-owned->getWidth())/2,330);
            this->add(foreign,(this->getWidth()-foreign->getWidth())/2,330);
            this->add(nope,(this->getWidth()-nope->getWidth())/2,330);
            this->resizeToContent();
            this->setWidth(225);
            //this->add(info,5,5);
        }
    FieldWindow::~FieldWindow(){
    }
    void FieldWindow::show(int field){
        int i;
        if(client::current_player==-1) {
            info1->setCaption("");
            info2->setCaption("");
            nope->setVisible(true);
            bwindow->setHeight(350+nope->getHeight());
            noowned->setVisible(false);
            owned->setVisible(false);
            foreign->setVisible(false);
        } else if (client::board[field].owner==-1
            &&(client::board[field].type==core::STREET||
               client::board[field].type==core::PLANT||
               client::board[field].type==core::RAILWAY)){
            noowned->setVisible(true);
            info1->setCaption("Это ничейная собственность.");
            info2->setCaption("Хотите купить?");
            bwindow->setHeight(350+noowned->getHeight());
            owned->setVisible(false);
            foreign->setVisible(false);
            nope->setVisible(false);
        }
        else if (client::board[field].owner==client::current_player){
            info1->setCaption("Это ваша собственность.");
            info2->setCaption("");
            owned->setVisible(true);
            bwindow->setHeight(350+owned->getHeight());
            noowned->setVisible(false);
            foreign->setVisible(false);
            nope->setVisible(false);
        }
        else if (client::board[field].type==core::STREET||
               client::board[field].type==core::PLANT||
               client::board[field].type==core::RAILWAY){
            foreign->setVisible(true);
            info1->setCaption("Это чужая собственность.");
            info2->setCaption("");
            bwindow->setHeight(350+foreign->getHeight());
            printf("foreign\n");
            noowned->setVisible(false);
            owned->setVisible(false);
            nope->setVisible(false);
        } else {
            info1->setCaption("");
            info2->setCaption("");
            nope->setVisible(true);
            bwindow->setHeight(350+nope->getHeight());
            noowned->setVisible(false);
            owned->setVisible(false);
            foreign->setVisible(false);
        }
        this->add(field_info[field]);
        this->setVisible(true);
    }
    AuctWindow::AuctWindow(const std::string&caption)
        :gcn::Window(caption)
        {
            int i;
            for(i=0;i<client::players_count;i++){

                submit[i]=new gcn::Button("Принять");
                submit[i]->setActionEventId(intToString(i));
                lprice[i]=new gcn::Label("0");
                lprice[i]->setWidth(55);
                tprice[i]=new gcn::TextField("0");
                tprice[i]->setWidth(55);
                players[i]=new gcn::Label("");
                players[i]->setWidth(55);
                price[i]=0;
            }
            ltimer=new gcn::Label("9");
            submitAuctListener=new SubmitAuctListener;

            for(i=0;i<client::players_count;i++){
                //price[i]=0;
                players[i]->setCaption(std::string(client::players[i].name));
                this->add(players[i],60*i+5,5);

                this->add(submit[i],60*i+5,45);
                submit[i]->addActionListener(submitAuctListener);
                this->add(tprice[i],60*i+5,20);
                this->add(lprice[i],60*i+5,20);
                players[i]->setCaption(std::string(client::players[i].name));
                players[i]->setVisible(true);
                if (client::players[i].type==core::LOCAL){
                    tprice[i]->setText(intToString(price[i]));
                    submit[i]->setVisible(true);
                    tprice[i]->setVisible(true);
                    lprice[i]->setVisible(false);
                } else {
                    lprice[i]->setCaption(intToString(price[i]));
                    submit[i]->setVisible(false);
                    tprice[i]->setVisible(false);
                    lprice[i]->setVisible(true);
                }

            }

            timer=9;
            this->resizeToContent();
            this->add(ltimer,(this->getWidth()-ltimer->getWidth())/2,80);
            ltimer->setCaption(intToString(timer));
            this->resizeToContent();
            this->setBaseColor(gcn::Color(255, 150, 200, 190));
            this->setVisible(false);
            top->add(this,(sdl::boardw-this->getWidth())/2,
                (sdl::boardh-this->getHeight())/2);
            printf("xx\n");

        }

    void AuctWindow::show(){
        int i;
        for(i=0;i<client::players_count;i++){
            if (client::players[i].type==core::REMOTE){
                lprice[i]->setCaption(intToString(price[i]));

            }
        }
        ltimer->setCaption(intToString(timer));
        if(!timer){
            delete this;
            awindow=NULL;
        }
        this->setVisible(true);
        SDL_mutexP(sdl::lock);
        sdl::redraw();
        SDL_mutexV(sdl::lock);
    }
}
