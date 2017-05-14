#ifndef CG_GAMEMANAGER_H
#define CG_GAMEMANAGER_H

#include <QObject>
#include <QMultiMap>
#include "cg_game.h"
#include "cg_player.h"
#include <QThread>

class QWebSocket;
class CG_GameManager : public QObject
{
    Q_OBJECT
public:
    explicit CG_GameManager(QObject *parent = nullptr);
    void setToAThread(QThread* thread);
    int  matchCount();
signals:
    void returnGames(QWebSocket * socket, QString game_data); //server
    void sendPlayerMadeMove(QWebSocket * socket, QString move_data); // server
    void notifySynchronizedGame(QWebSocket * socket, int state); // server
    void notifiedMatchedGame(QWebSocket * socket, QString data); // server
    void notifyPlayerChoseColor(QWebSocket * socket, QString data); // server
    void updateLastGameDb(int id, quint64 date, QString game_data);  //lobby
    void notifyPlayerPostGame(QWebSocket* socket,  QString post_data);

public slots:
    void matchedGame(CG_Player black, CG_Player white, quint64 time);  // lobby
    void fetchGames(QWebSocket * socket,int index); // server request
    void makeMove(QWebSocket * socket, QString move_data); // server request
    void sendGameReady(QWebSocket* socket, quint64 id, QString name); // server request
    void chooseColor(QWebSocket * socket, quint64 id, bool color);
    void notifySendResult(QWebSocket * socket, quint64 id, QString result);

protected:

    QMap<quint64, CG_Game>   mGames; // stored by their game rank (combined elo)
};

#endif // CG_GAMEMANAGER_H
