#include "cg_server.h"
#include <QWebSocketServer>
#include "cg_database.h"
#include <QWebSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#ifdef CG_TEST_ENABLED
#include <QtTest/QTest>
#endif

CG_Server::CG_Server(QString db_path,QObject *parent) :
    QObject(parent), m_db(db_path,nullptr), m_server(nullptr),
    m_dbThread(nullptr), m_LobbyThread(nullptr), m_lobbyManager()
{
//    m_lobbies.insert(QStringLiteral("All"),CG_PlayerList());
//    m_lobbies.insert(QStringLiteral("1 Minute"),CG_PlayerList());
//    m_lobbies.insert(QStringLiteral("5 Minute"),CG_PlayerList());
//    m_lobbies.insert(QStringLiteral("30 Minute"),CG_PlayerList());

    m_dbThread = new QThread(this);
    m_db.setToAThread(m_dbThread);
    m_dbThread->start();

    connect(this,&CG_Server::verifyPlayer, &m_db, &CG_Database::verifyUserCredentials);
    connect(&m_db,&CG_Database::userVerificationComplete, this, &CG_Server::userVerified);
    connect(&m_lobbyManager, &CG_LobbyManager::sendLobyList, this, &CG_Server::sendVerifiedPlayerMessage);
#ifdef CG_TEST_ENABLED
    QTest::qExec(&m_db, 0, nullptr);
#endif
}

void CG_Server::sendVerifiedPlayerMessage(QWebSocket * socket,  QByteArray message)
{

}

bool CG_Server::startToListen(QHostAddress addr, quint16 port)
{
    bool listening;
    if(m_server == nullptr){
        m_server = new QWebSocketServer("CG_Server",QWebSocketServer::NonSecureMode,this); // http connection
    }
    connect(m_server, &QWebSocketServer::newConnection, this, &CG_Server::incommingConnection);
    listening = m_server->listen(addr,port);
    if(!listening){
        qDebug() << "Failed to start the server for: " << addr << " @ " << port;
        Q_ASSERT(m_server->isListening());
    }
    else{
        qDebug() << "Started the server on: " << addr << " @ " << port;
    }
    return listening;
}
#ifdef CG_TEST_ENABLED
void CG_Server::testStartListen()
{
    QFETCH(QString, ip);
    QFETCH(int, port);
    bool listening  = startToListen(QHostAddress(ip),port);
    QCOMPARE(listening,true);
}

void CG_Server::testStartListen_data()
{
    QTest::addColumn<QString>("ip");
    QTest::addColumn<int>("port");
    QTest::newRow("Local") <<  "127.0.0.1" << 5442;
}
#endif
void CG_Server::closeServer()
{
    if(m_server){
        m_server->close(); // will be deleted by CG_Server (QObject)
        m_server->deleteLater();
    }
    m_db.deleteLater();
    deleteLater();
}

int CG_Server::getMatchCount()
{
    return m_matchList.count();
}

int CG_Server::getPlayerCount()
{
    return m_connected.count();
}

int CG_Server::getQueueCount()
{
    //return m_lobbies.value("1M").count();
}


// Server specific protected slots



void CG_Server::closePending()
{
    QWebSocket* socket = qobject_cast<QWebSocket*>(sender());
    m_pending.removeAll(socket);
    socket->abort();
    socket->deleteLater();
}

void CG_Server::pendingDisconnected()
{
    QWebSocket* socket = qobject_cast<QWebSocket*>(sender());
    m_pending.removeAll(socket);
    socket->deleteLater();
}

void CG_Server::incommingConnection()
{
    // get the websocket
    QWebSocket* socket = qobject_cast<QWebSocket*>(sender());
    QString ip = socket->peerAddress().toString();
    if(m_banned.contains(ip)){
        socket->close(QWebSocketProtocol::CloseCodePolicyViolated,"GTFO Bro");
    }
    connect(socket, &QWebSocket::binaryMessageReceived, this, &CG_Server::incommingPendingMessage);
    connect(socket,&QWebSocket::aboutToClose,this, &CG_Server::closePending);
    connect(socket,&QWebSocket::disconnected, this, &CG_Server::pendingDisconnected);
}


void CG_Server::incommingDBReply(QString player, QByteArray data)
{

}

void CG_Server::incommingLobbyReply(QString lobby, QByteArray data)
{

}

void CG_Server::incommingMatchReply(QString player, QByteArray data)
{

}

void CG_Server::incommingPendingMessage(QByteArray message)
{
    // Login credentials
    QWebSocket* socket = qobject_cast<QWebSocket*>(sender());
    QJsonDocument doc = QJsonDocument::fromBinaryData(message);
    QJsonObject obj = doc.object();
    if(obj.isEmpty() || doc.isNull() || doc.isEmpty()){
        return;
    }

    int target = obj["T"].toInt();
    QJsonArray params = obj["P"].toArray();
    switch(target)
    {
        case VERIFY_USER:{
            if(params.count() > 2){
                QString name = params.at(0).toString();
                QString pass_str = params.at(1).toString();
                QByteArray hpass = pass_str.toLatin1();
                emit verifyPlayer(socket,name,hpass);
            }
            else{
                qDebug() << "Received verify user command for " << socket->peerAddress();
            }
        }
        default: break;

    }
}

void CG_Server::incommingVerifiedMessage(QByteArray message)
{
    QWebSocket * socket = qobject_cast<QWebSocket*>(sender());
    // route a verified player message
    QJsonDocument doc = QJsonDocument::fromBinaryData(message);
    QJsonObject obj = doc.object();
    int target = obj["T"].toInt();
    QJsonArray params = obj["P"].toArray();
    switch(target)
    {
        case FETCH_LOBBIES:{
            // no parameters only a return
            emit fetchLobbies(socket);
            break;
        }
        default: break;
    }


}

void CG_Server::playerClosing()
{
    QWebSocket * socket = qobject_cast<QWebSocket*>(sender());
    CG_Player player = m_connected.take(socket);
    emit notifyPlayerLeaving(player.mUserData.username,player.mConnectedLobbies);
    emit disconnectPlayer(player);
    socket->close();
    socket->abort();
    socket->deleteLater();
}


void CG_Server::playerDropped()
{
    QWebSocket * socket = qobject_cast<QWebSocket*>(sender());
    CG_Player player = m_connected.take(socket);
    emit notifyPlayerDropped(player.mUserData.username,player.mConnectedLobbies);
    socket->disconnect();
    m_disconnecting.insert(socket,player);
}


void CG_Server::userVerified(QWebSocket *socket, bool verified, CG_User data)
{
    m_pending.removeAll(socket);
    if(verified && (!data.username.isEmpty())){
        CG_Player player;
        player.mWebSocket = socket;
        socket->disconnect();
        connect(socket, &QWebSocket::binaryMessageReceived, this, &CG_Server::incommingPendingMessage);
        connect(socket,&QWebSocket::aboutToClose,this, &CG_Server::closePending);
        connect(socket,&QWebSocket::disconnected, this, &CG_Server::pendingDisconnected);
        player.mUserData = data;
        m_connected.insert(socket,player);
    }
    else{
        socket->close(QWebSocketProtocol::CloseCodeBadOperation);
        socket->abort();
        socket->deleteLater();
    }
}


CG_Server::~CG_Server(){
    if(m_server){
        qDebug() << "Disconnecting server";
        m_server->close(); // will be deleted by CG_Server (QObject)
        m_server->deleteLater();
    }
    if(m_dbThread){
        m_dbThread->quit();
        m_dbThread->exit();
        m_dbThread->deleteLater();
    }
    if(m_LobbyThread){
        m_LobbyThread->quit();
        m_LobbyThread->exit();
        m_LobbyThread->deleteLater();
    }
}




