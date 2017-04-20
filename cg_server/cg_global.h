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
    int      countryFlag = 0;
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

#endif // CG_GLOBAL_H
