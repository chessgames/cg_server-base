#ifndef CG_LOBBYMANAGER_H
#define CG_LOBBYMANAGER_H

#include <QObject>
#include <QWebSocket>
#include "cg_lobby.h"
#include "cg_player.h"
#include <QThread>

class CG_LobbyManager : public QObject
{
    Q_OBJECT
public:
    explicit CG_LobbyManager(QObject *parent = nullptr);
    void setToAThread(QThread *thread);
    ~CG_LobbyManager();
signals:
    void sendLobbyList(QWebSocket* socket, QByteArray list);
    void matchedPlayers(CG_Player black, CG_Player white,quint64 time);

public slots:
    void fetchLobbyList(QWebSocket* socket);
    void joinMatchMaking(int type, CG_Player black);

protected:
    QMap<QString,CG_Lobby>   mLobbies;
    QList<CG_Player>         mOneMinute;
    QList<CG_Player>         mFiveMinute;
    QList<CG_Player>         mThirtyMinute;
    void sendPlayerInformation(QWebSocket *socket, CG_Player player, bool color);

};

#endif // CG_LOBBYMANAGER_H
