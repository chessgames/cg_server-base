#ifndef CG_SERVER_H
#define CG_SERVER_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QString>
#include <QHostAddress>
#include <QThread>
#include "cg_database.h"
#include "cg_player.h"
#include "cg_lobbymanager.h"
#include "cg_gamemanager.h"

class QWebSocketServer;
typedef QList<CG_Player> CG_PlayerList;
/************************************************************************
* Class: CG_Server
*
* Constructors:
*   explicit CG_Server(QObject *parent = 0);
*
* Slots:
*   void clientDisconnected()
*       Detects that a client disconnected from the server
*       and removes the client.
*
* Methods:
*   void StartServer()
*       Starts the server.
*   void incomingConnection(qintptr socketDescriptor)
*       Accepts any pending connections.
*   void sendMove(QTcpSocket *client)
*       Sends the move to the client
*   void addPlayerConnection(QTcpSocket *chessPlayer)
*       Adds a player connection to the list of connections.
*   void startOneMinuteMatch()
*       Starts a one minute match, when there are two or more players
*   void removeAllClientConnections(QTcpSocket *client)
*       Removes all clients from the connection's list.
*   void writeSocketDescriptorToSocket(QTcpSocket *client, qintptr socketDescriptor)
*       Follows a series of conversions to write socketDescriptor
*       to the socket.
*
*
*
* April 14th, 2017 Brought over the above interface from previous work
*  attempting to work closesly with existing functions. - C.Dean
*
*************************************************************************/

class CG_Server : public QObject
{
    Q_OBJECT
public:
    CG_Server(QString db_path, QObject *parent = nullptr);
    CG_Server(QString db_host_name, QString name, QString password, int db_port, QObject *parent = nullptr);
    bool startToListen(QHostAddress addr, quint16 port);
    int getPlayerCount();
    int getMatchCount();
    int getQueueCount();
    void closeServer();
    void setToAThread(QThread* thread);
    ~CG_Server();

signals:
    // DATABASE SIGNALS
    void verifyPlayer(QWebSocket * socket, QString name, QByteArray hpass);
    void addUser(QWebSocket * socket, QString name, QByteArray hpass,QString email, QString cg_data);
    void requestSetUserData(QWebSocket * socket, QString name, QByteArray hpass, QString data);
    void requestAddUser(QString user, QByteArray password, QString email);

    // LOBBY MANAGER SIGNALS
    void fetchLobbies(QWebSocket* socket);
    void notifyFetchGames(QWebSocket * socket, int index);
    void notifyPlayerDropped(QString name, QStringList lobbies);
    void notifyPlayerLeaving(QString name, QStringList lobbies);
    void disconnectPlayer(CG_Player);

    // GAME MANAGER SIGNALS
    void notifyPlayerReady(QWebSocket * socket, quint64 id, QString name);
    void notifyMakeMove(QWebSocket * socket, QString move_data);
    void notifyGameResult(QWebSocket *socket, QString result);

public slots:
    // DATABASE SLOTS
    void userVerified(QWebSocket* socket, bool verified, QString data);
    void sendAddUserReply(QWebSocket * socket,bool added, int reason);
    void userDataSet(QWebSocket * socket,bool set);

    // LOBY MANAGER SLOTS
    void sendLobbyData(QWebSocket* socket, QByteArray list);
    void sendConnectedToMatchMaking(QWebSocket * socket, QString type);

    // GAME MANAGER SLOTS
    void sendMatchedPlayer(QWebSocket * socket, QString player_data);
    void sendSynchronizeGame(QWebSocket* socket, int state);
    void sendPlayerMadeMove(QWebSocket * socket, QString move_data);
    void sendReturnMatches(QWebSocket* socket, QString match_data);
    void sendPlayerPostGame(QWebSocket* socket, QString post_data);


protected:
    QThread *                   m_dbThread;
    QThread *                   m_LobbyThread;
    QThread *                   m_gameThread;
    CG_Database                 m_db;
    CG_LobbyManager             m_lobbyManager;
    CG_GameManager              m_gameManager;
    QWebSocketServer           *m_server;
    QMap<QWebSocket*, CG_Player>m_connected;
    QList<QWebSocket*>          m_pending;
    QMap<QWebSocket*, CG_Player>m_disconnecting;
    QList<QString>              m_banned;



protected slots:
    // server connections
    void incommingConnection();
    void incommingPendingMessage(QByteArray message);
    void incommingVerifiedMessage(QByteArray message);


    void closePending();
    void pendingDisconnected();
    void playerClosing();
    void playerDropped();

#ifdef CG_TEST_ENABLED
private slots:
    void testStartListen();
    void testStartListen_data();
#endif

};

#endif // CG_SERVER_H
