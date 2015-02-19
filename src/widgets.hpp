#ifndef WIDGETS_HPP_INCLUDED
#define WIDGETS_HPP_INCLUDED
/**
 * Code to populate a global Gui object with all the widgets
 * of Guichan.
 */
#include "sdlutf8truetypefont.hpp"
#include "utf8textfield.hpp"
#include "core.hpp"
#include <string>
#include <SDL_net.h>

namespace widgets
{

    class LoginWindow: public gcn::Window
    {
        public:


        LoginWindow(const std::string& caption);
        ~LoginWindow();
        gcn::Button *play;
        gcn::CheckBox *create;
        gcn::TextField *serv;
        gcn::TextField *port;
        gcn::TextField *client_num;
        gcn::Label *servl;
        gcn::Label *portl;
        gcn::Label *client_numl;
    };

    class PlayersWindow: public gcn::Window
    {
        public:
            PlayersWindow(const std::string& caption);
            ~PlayersWindow();
            gcn::ListBox *players;
            gcn::UTF8TextField *newplayer;
            gcn::Label *newplayerl;
            gcn::Button *addplayer;
            TCPsocket socket;
    };
    class FieldWindow: public gcn::Window
    {
        public:
            FieldWindow(const std::string& caption);
            ~FieldWindow();
            gcn::Container *noowned,*owned,*foreign,*nope;
            gcn::Label *info1,*info2;
            gcn::Button *yes;
            gcn::Button *no;
            gcn::Button *close1,*close2;
            gcn::Button *buyhouse,*sellhouse,*mortgage,*sellfield,*buyfield;
            void show(int field);

    };
    class AuctWindow:public gcn::Window
    {
        public:
            AuctWindow(const std::string& caption);
            gcn::Label * players[8];
            gcn::Label * lprice[8];
            gcn::Label * ltimer;
            gcn::TextField * tprice[8];
            gcn::Button * submit[8];
            Sint64 price[8];
            int timer;
            void show();
    };
    extern LoginWindow *window;
    extern PlayersWindow *pwindow;
    extern FieldWindow *bwindow;
    extern AuctWindow *awindow;
    void init();
    void halt();
}
#endif
