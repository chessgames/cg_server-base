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
    CG_Player(const CG_Player &right);
    CG_Player & operator=(const CG_Player &right);
    ~CG_Player();

    QString serialize();

    QList<QString>  mConnectedLobbies;
    CG_User         mUserData;
    bool            mReady;
    bool            mColor;
    QWebSocket*     mWebSocket;
    quint64         mGameID;
};

#endif // CG_PLAYER_H
