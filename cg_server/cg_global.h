#ifndef CG_GLOBAL_H
#define CG_GLOBAL_H

#include <QString>
#include <QDebug>
typedef struct CG_User
{
    bool     loggedIn = false;
    bool     banned = false;
    QString  username = "";
    int      elo = 0;
    QString  countryFlag = "United States";
    int      pieceSet = 0;
    int      language = 0;
    bool     sound = false;
    bool     coordinates = false;
    bool     arrows = false;
    bool     autoPromote = false;
    QString  boardTheme = "";
    quint32  cgbitfield = 0;
    bool     isValid = false;
}CG_User;


static QDebug operator<<(QDebug dbg, const CG_User &user)
{
    if(user.isValid){
        dbg.nospace() << "CG_User{";
        dbg.nospace() << "\nName: " << user.username;
        dbg.nospace() << "\nArrows: " << user.arrows;
        dbg.nospace() << "\nAutoPromote: " << user.autoPromote;
        dbg.nospace() << "\nBanned: " << user.banned;
        dbg.nospace() << "\nBoardTheme: " << user.boardTheme;
        dbg.nospace() << "\nBitField: " << user.cgbitfield;
        dbg.nospace() << "\nCoordinates: " << user.coordinates;
        dbg.nospace() << "\nFlag: " << user.countryFlag;
        dbg.nospace() << "\nElo: " << user.elo;
        dbg.nospace() << "\nLanguage: " << user.language;
        dbg.nospace() << "\nLoggedIn: " << user.loggedIn;
        dbg.nospace() << "\nPieceSet: " << user.pieceSet;
        dbg.nospace() << "\nSound: " << user.sound;
        dbg.nospace() << "\n}";
    }
    return dbg.maybeSpace();
}



// server globals
// Login
static const int VERIFY_USER =  453;
static const int REGISTER_USER =  543;


// Profile
static const int SET_USER_DATA =  6895;

// Lobby
static const int SEND_MESSAGE =  4512;
static const int JOIN_LOBBY =  4321;
static const int LEAVE_LOBBY =  4231;
static const int FETCH_LOBBIES   =  4555;

// Game
static const int JOIN_MATCHING =  5541;
static const int CANCEL_MATCHING =  5451;
static const int SEND_GAME_MESSAGE =  5421;
static const int CHOOSE_COLOR =  5114;
static const int SEND_MOVE =  5345;
static const int SEND_READY_STATUS =  5123;
static const int EXIT_MATCHING =  5231;
static const int FORFEIT_MATCH =  5899;



#endif // CG_GLOBAL_H
