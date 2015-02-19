#ifndef CORE_HPP_INCLUDED
#define CORE_HPP_INCLUDED
#include <SDL.h>
namespace core{
    const int JAIL_LOCATION=10;
    enum FieldType {STREET,RAILWAY,PLANT,CCHEST,CHANCE,FREEPARKING,START,JAIL,
        TO_JAIL,FEE};
    enum JailState {FREE=0,JAIL_1,JAIL_2,JAIL_3};
    enum PlayerType {UNDEF,LOCAL,REMOTE,BANKROT};
    typedef struct {
        char name[21];
        Sint64 money;
        int location;
        int jailed;
        PlayerType type;
    } Player;
    typedef struct{
        FieldType type;
        int houses_num;
        int price;
        int rent[6];
        int mortgage_val;
        int house_cost;
        int hotel_cost;
        bool mortgaged;
        int owner;
    } Field;
    enum PacketType {EVENT,DISCONNECT,ACK,PLAYER_QUERY,SERVER_FULL,UPDATE_PLAYERS,
    UPDATE_FIELD,OWNED,NOT_OWNED,LOOSER,WINNER,AUCTION,BUY_HOUSE,BUY_HOTEL,BUY_FIELD,
    SELL_FIELD} ;
    typedef struct{

        PacketType type;
        char names[168];
        Sint32 src,dst;
        Sint32 dice1,dice2;
        Sint32 curr_player;
        Uint32 pnum;
        Sint64 moneys[8];
    } PacketData;
    //extern bool running;
    extern Field board[40];
    extern Player player[8];

    void init(Field *board,Player *players);

    int valid_field(int num);

    void copy_name(const char *s,char name[20]);

}


#endif // CORE_HPP_INCLUDED
