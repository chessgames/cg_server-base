#ifndef CG_PLAYER_H
#define CG_PLAYER_H

#include <QList>
#include <QString>
#include "cg_global.h"
class QWebSocket;
class CG_Player
{
public:
    CG_Player();
    ~CG_Player();

    QList<QString>  mConnectedLobbies;
    CG_User         mUserData;
    QWebSocket*     mWebSocket;
};

#endif // CG_PLAYER_H
