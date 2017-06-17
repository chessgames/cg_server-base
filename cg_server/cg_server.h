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
#include <QTimer>
#include <QNetworkSession>
#include <QNetworkConfiguration>


class QWebSocketServer;
typedef QList<CG_Player> CG_PlayerList;
/************************************************************************
* Class: CG_Server
*
* Constructors:
*
* Slots:
*
* Methods:
*
* April 14th, 2017 Brought over the above interface from previous work
*  attempting to work closesly with existing functions. - C.Dean
*
*************************************************************************/

class CG_Server : public QObject
{
    Q_OBJECT
public:
    CG_Server(QString db_path, QNetworkConfiguration* config, QObject *parent = nullptr);
    CG_Server(QString db_host_name, QString name, QString password, int db_port, QNetworkConfiguration * config, QObject *parent = nullptr);
    bool startToListen(QHostAddress addr, quint16 port);
    int getPlayerCount();
    int getMatchCount();
    void closeServer();
    void setToAThread(QThread* thread);
    ~CG_Server();

signals:

    // LOBBY MANAGER SIGNALS
    void fetchLobbies(QWebSocket* socket);
    void notifyFetchGames(QWebSocket * socket, int index);
    void disconnectPlayer(CG_Player);
    void playerPing(QWebSocket * socket, quint64 ping, quint64 g_id, QString name);

public slots:
    // DATABASE SLOTS
    void userVerified(QWebSocket* socket, bool verified, QString meta = QString(), CG_User user = CG_User());
    void sendAddUserReply(QWebSocket * socket,bool added, int reason);
    void userDataSet(QWebSocket * socket,CG_User user);
    void refreshUserData(QWebSocket * socket,QString meta, CG_User user);

    // LOBY MANAGER SLOTS
    void sendLobbyData(QWebSocket* socket, QByteArray list);
    void sendConnectedToMatchMaking(QWebSocket * socket, QString type);

    // GAME MANAGER SLOTS
    void sendMatchedPlayer(QWebSocket * socket, QJsonObject player_data);
    void sendSynchronizeGame(QWebSocket* socket, int state, quint64 time);
    void sendPlayerMadeMove(QWebSocket * socket, int from, int to, QString fen, QString promote, quint64 time);
    void sendReturnMatches(QWebSocket* socket, QString match_data);
    void sendPlayerPostGame(QWebSocket* socket, QString post_data);
    void sendOpponentUpdate(QWebSocket* socket, QString data);
    void sendDraw(QWebSocket* socket, int draw);

    // SERVER hertbeat
    void socketPong(quint64 elapsed, const QByteArray& message);

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
    QMap<quint64, CG_Player>    m_disconnecting;
    QList<QString>              m_banned;
    QJsonDocument               m_output;
    QJsonObject                 m_rootobj;
    QTimer                      m_reconnectTimer;
    QNetworkSession             m_networkSession;
    QHostAddress                m_serverAddress;
    quint16                     m_serverPort;
    // method
    void makeConnections();


protected slots:
    // server connections
    void incommingConnection();
    void incommingPendingMessage(QByteArray message);
    void incommingVerifiedMessage(QByteArray message);


    void closePending();
    void pendingDisconnected();
    void playerClosing();
    void checkPlayerState(QAbstractSocket::SocketError error);
    void playerDropped();
    void networkStateChanged(QNetworkSession::State state);
    void establishServer();

#ifdef CG_TEST_ENABLED
private slots:
    void testStartListen();
    void testStartListen_data();
#endif

};

#endif // CG_SERVER_H
