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
    void setPgn(QString pgn);
    CG_Player white();
    CG_Player black();
    QWebSocket* socket();

    QWebSocket *setReady(QWebSocket *&socket, QString name);
    void makeMove(QWebSocket* socket,  quint32 elapsed, QJsonObject &obj, QWebSocket *&out);
    bool setResult(QWebSocket * socket, QString result, quint64 date, QString &post_datab, QString &post_dataw);
    QString serialize();

protected:
    int                 mMoveCount;
    int                 mBResult;
    int                 mWResult;
    quint64             mBClock;
    quint64             mWClock;
    QString             mPgn;
    QList<CG_Player>    mSpectators;
    CG_Player           mBlack;
    CG_Player           mWhite;
};

#endif // CG_GAME_H
