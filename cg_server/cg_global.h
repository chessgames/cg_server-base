#ifndef CG_GLOBAL_H
#define CG_GLOBAL_H

#include <QString>

typedef struct CG_User
{
    bool     loggedIn;
    bool     banned;
    QString  username;
    int      elo;
    int      countryFlag;
    int      pieceSet;
    int      language;
    bool     sound;
    bool     coordinates;
    bool     arrows;
    bool     autoPromote;
    QString  boardTheme;
}CG_User;

#endif // CG_GLOBAL_H
