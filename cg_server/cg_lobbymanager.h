#ifndef CG_LOBBYMANAGER_H
#define CG_LOBBYMANAGER_H

#include <QObject>
#include <QWebSocket>
#include "cg_lobby.h"
class CG_LobbyManager : public QObject
{
    Q_OBJECT
public:
    explicit CG_LobbyManager(QObject *parent = nullptr);
    ~CG_LobbyManager();
signals:
    void sendLobyList(QWebSocket* socket, QByteArray list);
public slots:
    void fetchLobbyList(QWebSocket* socket);

protected:
    QMap<QString,CG_Lobby>   mLobbies;
   // QMap<QString,CG_PlayerList> m_lobbies;
};

#endif // CG_LOBBYMANAGER_H
