#ifndef CG_SERVER_H
#define CG_SERVER_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QString>
#include <QHostAddress>

#include "cg_database.h"
class QWebSocketServer;
class CG_Player;
typedef QList<CG_Player*> CG_PlayerList;
typedef QMap<QString,CG_PlayerList> CG_Match;
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
    explicit CG_Server(QObject *parent = nullptr);
    bool startToListen(QHostAddress addr, quint16 port);
    int getPlayerCount();
    int getMatchCount();
    int getQueueCount();
    ~CG_Server();
signals:
    void playersReadyToBeMatched();

public slots:
    /*void clientDisconnected();
    void handleJoinQueue(TimeControl time_type);
    void queueTimerExpired();
    void removeMatch();
    void removePlayerFromQueue();*/

protected:

    CG_Database                 m_db;
    QWebSocketServer           *m_server;
    QMap<QString,CG_PlayerList> m_lobbies;
    QList<CG_Match>             m_matchList;
};

#endif // CG_SERVER_H
