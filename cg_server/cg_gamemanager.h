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
    ~CG_GameManager();
    void setToAThread(QThread* thread);
    int  matchCount();
signals:
    void returnGames(QWebSocket * socket, QString game_data); //server
    void sendPlayerMadeMove(QWebSocket * socket,int from, int to, QString fen, QString promote, quint64 time); // server
    void sendDrawResponse(QWebSocket* socket, int response);
    void notifySynchronizedGame(QWebSocket * socket, int state, quint64 time); // server
    void notifiedMatchedGame(QWebSocket * socket, QJsonObject data); // server
    void notifyPlayerChoseColor(QWebSocket * socket, QString data); // server
    void updateLastGameDb(double id, int elo_change, double date, QString game_data);  //lobby
    void updatePlayerRank(QWebSocket * socket, QString name, int elo);
    void notifyPlayerPostGame(QWebSocket* socket,  QString post_data);
    void notifyPlayerChanged(QWebSocket * socket, QString data);
    void notifyOpponentReconnect(QWebSocket* socket, QString data);
    void notifyMatchPending(QWebSocket * socket, QString match_info);

public slots:
    void matchedGame(CG_Player black, CG_Player white, quint64 time);  // lobby
    void sendDraw(QWebSocket * socket,int response, quint64 id);
    void fetchGames(QWebSocket * socket,int index); // server request
    void makeMove(QWebSocket * socket, quint64 id, int from, int to, QString fen, QString promote, int time, quint64 latency); // server request
    void sendGameReady(QWebSocket* socket, quint64 id, quint64 latency); // server request
    void chooseColor(QWebSocket * socket, quint64 id, bool color);
    void sendResult(QWebSocket * socket, quint64 id, double white_t, double black_t, int result, QString last);
    void sendPlayerUpdate(QWebSocket * socket, QString meta, CG_User data);
    void checkPendingMatch(QWebSocket * socket, quint64 id);
    void playerDisconnected(QWebSocket * socket, quint64 id);
    void reconnectPlayer(quint64 id, CG_Player player);

protected:
    CG_Game *findGame(QWebSocket * player);
    QMap<quint64, CG_Game*>   mGames; // stored by their game rank (combined elo)


    void calculateEloChange(double result, int& elo, int& op_elo);

};

#endif // CG_GAMEMANAGER_H
