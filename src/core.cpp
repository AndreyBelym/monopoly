#include "core.hpp"
namespace core{
    bool running=true;
    //Field board[40];

    void copy_name(const char *s,char name[20]){
        int i;
        for (i=0;i<20;i++){
            if (!s[i]) break;
            name[i]=s[i];
        }
        name[i]='\0';
    }

    int valid_field(int num){
        return num%40;
    }

    void init(Field *board,Player *players){
        int i;
        for (i=0;i<40;++i){
            switch (i){
                case 0: board[i].type=core::START; break;
                case 2: board[i].type=core::CCHEST; break;
                case 4: board[i].type=core::FEE; board[i].price = 200; break;
                case 5: board[i].type=core::RAILWAY; board[i].price = 200; break;
                case 7: board[i].type=core::CHANCE; break;
                case 10: board[i].type=core::JAIL; break;
                case 12: board[i].type=core::PLANT; board[i].price = 150; break;
                case 15: board[i].type=core::RAILWAY; board[i].price = 200; break;
                case 17: board[i].type=core::CCHEST; break;
                case 20: board[i].type=core::FREEPARKING; break;
                case 22: board[i].type=core::CHANCE; break;
                case 25: board[i].type=core::RAILWAY; board[i].price = 200; break;
                case 28: board[i].type=core::PLANT; board[i].price = 150; break;
                case 30: board[i].type=core::TO_JAIL; break;
                case 33: board[i].type=core::CCHEST; break;
                case 35: board[i].type=core::RAILWAY; board[i].price = 200; break;
                case 36: board[i].type=core::CHANCE; break;
                case 38: board[i].type=core::FEE; board[i].price = 100; break;
                default:
                    board[i].type=core::STREET;
            }
            board[i].owner=-1;
            if (board[i].type==core::STREET){
                //printf("/%d/\n",i);
                board[i].mortgaged=false;
                board[i].houses_num=0;
                char filename[30];
                int l=sprintf(filename,"field%d.txt",i);
                filename[l]='\0';
                FILE* f=fopen(filename,"r");
                fscanf(f,"%d",&board[i].price);
                //printf("price:%d\n",board[i].price);
                int j;
                for(j=0;j<6;j++){
                    fscanf(f,"%d",&board[i].rent[j]);
                    //printf("rent%d:%d\n",j,board[i].rent[j]);
                }
                fscanf(f,"%d",&board[i].house_cost);
                //printf("house_cost:%d\n",board[i].house_cost);
                fscanf(f,"%d",&board[i].hotel_cost);
                //printf("hotel_cost:%d\n",board[i].hotel_cost);
                fscanf(f,"%d",&board[i].mortgage_val);
                //printf("mortgage_val:%d\n",board[i].mortgage_val);
                fclose(f);

            }

        }
        for(i=0;i<8;++i){
            core::copy_name("Нет игрока\0",players[i].name);
            players[i].money=1500;
            players[i].location=0;
            players[i].jailed=0;
            players[i].type=UNDEF;
        }
    }
}
