#ifndef CG_LOBBYMANAGER_H
#define CG_LOBBYMANAGER_H

#include <QObject>
#include <QWebSocket>
#include "cg_lobby.h"
#include "cg_player.h"

class CG_LobbyManager : public QObject
{
    Q_OBJECT
public:
    explicit CG_LobbyManager(QObject *parent = nullptr);
    ~CG_LobbyManager();
signals:
    void sendLobyList(QWebSocket* socket, QByteArray list);
    void sendMatchedPlayer(QWebSocket* socket, QString data);

public slots:
    void fetchLobbyList(QWebSocket* socket);
    void joinMatchMaking(int type, CG_Player black);

protected:
    QMap<QString,CG_Lobby>   mLobbies;
    QList<CG_Player>         mOneMinute;
    QList<CG_Player>         mFiveMinute;
    QList<CG_Player>         mThirtyMinute;
    void matchPlayers(CG_Player black, CG_Player white);
    void sendPlayerInformation(QWebSocket *socket, CG_Player player, bool color);
   // QMap<QString,CG_PlayerList> m_lobbies;
};

#endif // CG_LOBBYMANAGER_H
