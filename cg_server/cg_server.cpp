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
    QObject(parent), m_db(db_path,"",""), m_server(nullptr),
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
    connect(this,&CG_Server::addUser, &m_db, &CG_Database::addUser);
    connect(&m_db,&CG_Database::addUserReply, this,&CG_Server::sendAddUserReply);
    connect(&m_db,&CG_Database::userVerificationComplete, this, &CG_Server::userVerified);
    connect(&m_lobbyManager, &CG_LobbyManager::sendLobyList, this, &CG_Server::sendLobbyData);
#ifdef CG_TEST_ENABLED
    QTest::qExec(&m_db, 0, nullptr);
#endif
}


CG_Server::CG_Server(QString db_host_name, QString name, QString password, int db_port, QObject *parent)
    :QObject(parent), m_db(db_host_name,name,password,db_port,nullptr), m_server(nullptr),
    m_dbThread(nullptr), m_LobbyThread(nullptr), m_lobbyManager()
{
//    m_lobbies.insert(QStringLiteral("All"),CG_PlayerList());
//    m_lobbies.insert(QStringLiteral("1 Minute"),CG_PlayerList());
//    m_lobbies.insert(QStringLiteral("5 Minute"),CG_PlayerList());
//    m_lobbies.insert(QStringLiteral("30 Minute"),CG_PlayerList());

    m_dbThread = new QThread(this);
    m_db.setToAThread(m_dbThread);

    connect(this,&CG_Server::verifyPlayer, &m_db, &CG_Database::verifyUserCredentials,Qt::QueuedConnection);
    connect(this,&CG_Server::addUser, &m_db, &CG_Database::addUser,Qt::QueuedConnection);
    connect(&m_db,&CG_Database::userDataSet, this, &CG_Server::userDataSet,Qt::QueuedConnection);
    connect(&m_db,&CG_Database::addUserReply, this,&CG_Server::sendAddUserReply,Qt::QueuedConnection);
    connect(&m_db,&CG_Database::userVerificationComplete, this, &CG_Server::userVerified,Qt::QueuedConnection);
    connect(&m_lobbyManager, &CG_LobbyManager::sendLobyList, this, &CG_Server::sendLobbyData,Qt::QueuedConnection);
    connect(&m_lobbyManager, &CG_LobbyManager::sendMatchedPlayer, this, &CG_Server::sendMatchedPlayer,Qt::QueuedConnection);

    m_dbThread->start();
#ifdef CG_TEST_ENABLED
    QTest::qExec(&m_db, 0, nullptr);
#endif
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
}

void CG_Server::pendingDisconnected()
{
    QWebSocket* socket = qobject_cast<QWebSocket*>(sender());
    m_pending.removeAll(socket);
}

void CG_Server::incommingConnection()
{
    // get the websocket
    QWebSocket* socket = m_server->nextPendingConnection();
    QString ip = socket->peerAddress().toString();
    qDebug() << "Incomming connection from " << ip;
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
            if(params.count() >= 2){
                QString name = params.at(0).toString();
                QString pass_str = params.at(1).toString();
                QByteArray hpass = pass_str.toLatin1();
                emit verifyPlayer(socket,name,hpass);
            }
            else{
                qDebug() << "Received verify user command for " << socket->peerAddress();
            }
            break;
        }
        case REGISTER_USER:{
            if(params.count() >= 3){
                QString name = params.at(0).toString();
                QString pass_str = params.at(1).toString();
                QByteArray hpass = pass_str.toLatin1();
                QString email = params.at(2).toString();
                QString cg;
                if(params.count() > 3){
                    cg= params.at(3).toString();
                }
                emit addUser(socket,name,hpass,email,cg);
            }
            break;
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
    CG_Player player = m_connected.value(socket);
    switch(target)
    {
        case FETCH_LOBBIES:{
            // no parameters only a return
            emit fetchLobbies(socket);
            break;
        }
        case JOIN_MATCHING:{
            // no parameters only a return

            if(params.count() >= 1 && (player.mWebSocket == socket))
            {
                int game_type = params.at(0).toInt();
                m_lobbyManager.joinMatchMaking(game_type,player);
            }
            break;
        }
        case SET_USER_DATA:{
            if(params.count() >=3){
                QString name = params.at(0).toString();
                QString pass = params.at(1).toString();
                QByteArray hpass = pass.toLatin1();
                QString data = params.at(2).toString();
                m_db.setUserData(socket,name,hpass,data);
            }
            break;
        }
        case SEND_MOVE:{
            if(params.count())
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
}


void CG_Server::playerDropped()
{
    QWebSocket * socket = qobject_cast<QWebSocket*>(sender());
    CG_Player player = m_connected.take(socket);
    emit notifyPlayerDropped(player.mUserData.username,player.mConnectedLobbies);
    socket->disconnect();
    m_disconnecting.insert(socket,player);
}


void CG_Server::sendAddUserReply(QWebSocket *socket, bool added, int reason)
{
    QJsonObject obj;
    obj["T"] = REGISTER_USER;
    QJsonArray params;
    params.append(added);
    params.append(reason);
    QJsonDocument doc;
    if(added ){
        obj["P"]=params;
        doc.setObject(obj);
        socket->sendBinaryMessage(doc.toBinaryData());
    }
    else{
        obj["P"]=params;
        doc.setObject(obj);
        socket->sendBinaryMessage(doc.toBinaryData());
    }
}

void CG_Server::sendConnectedToMatchMaking(QWebSocket *socket, QString type)
{

}

void CG_Server::sendLobbyData(QWebSocket *socket, QByteArray list)
{

}

void CG_Server::userDataSet(QWebSocket *socket, bool set)
{
    QJsonObject obj;
    obj["T"] = SET_USER_DATA;
    QJsonArray params;
    params.append(set);
    QJsonDocument doc;
    obj["P"]=params;
    doc.setObject(obj);
    socket->sendBinaryMessage(doc.toBinaryData());
}

void CG_Server::sendMatchedPlayer(QWebSocket *socket, QString player_data)
{
    QJsonObject obj;
    obj["T"] = MATCHED_PLAYER;
    QJsonArray params;
    params.append(player_data);
    QJsonDocument doc;
    obj["P"]=params;
    doc.setObject(obj);
    socket->sendBinaryMessage(doc.toBinaryData());
}
void CG_Server::userVerified(QWebSocket *socket, bool verified, CG_User data)
{
    QJsonObject obj;
    obj["T"] = VERIFY_USER;
    QJsonArray params;
    params.append(verified);
    QJsonDocument doc;
    if(verified ){
        CG_Player player;
        player.mWebSocket = socket;
        socket->disconnect();
        connect(socket, &QWebSocket::binaryMessageReceived, this, &CG_Server::incommingVerifiedMessage);
        connect(socket,&QWebSocket::aboutToClose,this, &CG_Server::playerClosing);
        connect(socket,&QWebSocket::disconnected, this, &CG_Server::playerDropped);
        player.mUserData = data;
        m_connected.insert(socket,player);
        params.append(CG_Database::serializeUser(data));
        data.loggedIn = true;
    }
    obj["P"]=params;
    doc.setObject(obj);
    socket->sendBinaryMessage(doc.toBinaryData());

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




