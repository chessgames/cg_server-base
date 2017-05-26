#ifndef CG_GAME_H
#define CG_GAME_H

#include "cg_player.h"
#include <QWebSocket>

class CG_Game
{
public:
    CG_Game(CG_Player white = CG_Player(), CG_Player black = CG_Player(), quint64 time = 60000);
    CG_Game(const CG_Game & right);
    CG_Game& operator=(const CG_Game& right);
    void addSpectator(CG_Player spec);
    void removeSpectator(CG_Player spec);
    void setFEN(QString fen);
    CG_Player& white();
    CG_Player& black();
    bool isValid();
    QWebSocket* otherSocket(QWebSocket * socket);
    QWebSocket *setReady(QWebSocket *&socket);
    QWebSocket *makeMove(QWebSocket* socket,  quint32 elapsed, QJsonObject &obj);
    int setResult(QWebSocket * socket, int result, int &elo_b, int &elo_w);
    QJsonObject serialize();
    QWebSocket * reconnectPlayer(const CG_Player &player, quint64 &id);
    static quint64 pair(quint64 w, quint64 b);

protected:
    int                 mMoveCount;
    int                 mBResult;
    int                 mWResult;
    quint64             mBClock;
    quint64             mWClock;
    QString             mCurrentState;
    QList<CG_Player>    mSpectators;
    CG_Player           mBlack;
    CG_Player           mWhite;
    bool                mValid;
};

#endif // CG_GAME_H
