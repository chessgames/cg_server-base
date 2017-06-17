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

CG_Server::CG_Server(QString db_path, QNetworkConfiguration * config, QObject *parent) :
    QObject(parent), m_db(db_path,"",""), m_server(nullptr),
    m_dbThread(nullptr), m_gameThread(nullptr), m_LobbyThread(nullptr), m_lobbyManager(), m_networkSession(*config)
{
//    m_lobbies.insert(QStringLiteral("All"),CG_PlayerList());
//    m_lobbies.insert(QStringLiteral("1 Minute"),CG_PlayerList());
//    m_lobbies.insert(QStringLiteral("5 Minute"),CG_PlayerList());
//    m_lobbies.insert(QStringLiteral("30 Minute"),CG_PlayerList());

    m_dbThread = new QThread(this);
    m_db.setToAThread(m_dbThread);


    m_gameThread = new QThread(this);
    m_gameManager.setToAThread(m_gameThread);
    makeConnections();
    m_gameThread->start();
    m_dbThread->start();
    m_reconnectTimer.setInterval(1000);
    m_reconnectTimer.setSingleShot(true);


#ifdef CG_TEST_ENABLED
    QTest::qExec(&m_db, 0, nullptr);
#endif
}


CG_Server::CG_Server(QString db_host_name, QString name, QString password, int db_port, QNetworkConfiguration *config, QObject *parent)
    :QObject(parent), m_db(db_host_name,name,password,db_port,nullptr), m_server(nullptr),
    m_dbThread(nullptr), m_LobbyThread(nullptr), m_lobbyManager(), m_networkSession(*config)
{
//    m_lobbies.insert(QStringLiteral("All"),CG_PlayerList());
//    m_lobbies.insert(QStringLiteral("1 Minute"),CG_PlayerList());
//    m_lobbies.insert(QStringLiteral("5 Minute"),CG_PlayerList());
//    m_lobbies.insert(QStringLiteral("30 Minute"),CG_PlayerList());
    makeConnections();
    m_dbThread = new QThread(this);
    m_db.setToAThread(m_dbThread);
    m_dbThread->start();
    m_reconnectTimer.setInterval(1000);
    m_reconnectTimer.setSingleShot(true);

    m_gameThread = new QThread(this);
    m_gameManager.setToAThread(m_gameThread);
    m_gameThread->start();


#ifdef CG_TEST_ENABLED
    QTest::qExec(&m_db, 0, nullptr);
#endif
}

void CG_Server::establishServer()
{
    m_server->close();
    if(m_networkSession.state() == QNetworkSession::Connected){
        if(!m_server->listen(m_serverAddress, m_serverPort)){
            m_reconnectTimer.start();
        }
        else{
            qDebug() << "Server is rebound to " << m_server->serverAddress();
        }
    }
    else{
        m_reconnectTimer.start();
    }
}


void CG_Server::makeConnections()
{
    // netowrk session
    connect(&m_networkSession, &QNetworkSession::stateChanged,this, &CG_Server::networkStateChanged);
    connect(&m_reconnectTimer, &QTimer::timeout, this, &CG_Server::establishServer);

    //connect(this, &CG_Server::notifyPlayerLeaving, &m_gameManager, &CG_GameManager::sendPlayerForfeit);
    connect(&m_db,&CG_Database::addUserReply, this,&CG_Server::sendAddUserReply);
    connect(&m_db,&CG_Database::userVerificationComplete, this, &CG_Server::userVerified);
    connect(&m_db,&CG_Database::userDataRefreshed, this, &CG_Server::refreshUserData, Qt::QueuedConnection);
    connect(&m_db,&CG_Database::userDataRefreshed, &m_gameManager, &CG_GameManager::sendPlayerUpdate, Qt::QueuedConnection);
    connect(&m_lobbyManager, &CG_LobbyManager::matchedPlayers, &m_gameManager, &CG_GameManager::matchedGame, Qt::QueuedConnection);
    connect(&m_gameManager, &CG_GameManager::notifiedMatchedGame, this, &CG_Server::sendMatchedPlayer, Qt::QueuedConnection);
    connect(&m_gameManager, &CG_GameManager::notifyPlayerChanged, this, &CG_Server::sendOpponentUpdate, Qt::QueuedConnection);
    connect(&m_gameManager, &CG_GameManager::sendPlayerMadeMove, this, &CG_Server::sendPlayerMadeMove, Qt::QueuedConnection);
    connect(&m_gameManager, &CG_GameManager::sendDrawResponse, this, &CG_Server::sendDraw, Qt::QueuedConnection);
    connect(&m_gameManager, &CG_GameManager::notifySynchronizedGame, this, &CG_Server::sendSynchronizeGame, Qt::QueuedConnection);
    connect(&m_gameManager, &CG_GameManager::notifyPlayerPostGame, this, &CG_Server::sendPlayerPostGame, Qt::QueuedConnection);
    connect(&m_gameManager, &CG_GameManager::updateLastGameDb, &m_db, &CG_Database::updateLastGame, Qt::QueuedConnection);
    connect(&m_gameManager, &CG_GameManager::updatePlayerRank, &m_db, &CG_Database::updateUserRanking, Qt::QueuedConnection);
    connect(&m_lobbyManager, &CG_LobbyManager::sendLobbyList, this, &CG_Server::sendLobbyData,Qt::QueuedConnection);
}

bool CG_Server::startToListen(QHostAddress addr, quint16 port)
{
    bool listening;
    if(m_server == nullptr){
        m_server = new QWebSocketServer("CG_Server",QWebSocketServer::NonSecureMode,this); // http connection
    }
    m_serverPort = port;
    m_serverAddress = addr;
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
    return m_gameManager.matchCount();
}

int CG_Server::getPlayerCount()
{
    return m_connected.count();
}

// Server specific protected slots

void CG_Server::checkPlayerState(QAbstractSocket::SocketError error)
{
    switch(error){ // TODO handle errors
        case QAbstractSocket::UnconnectedState: break;
        case QAbstractSocket::HostLookupState: break;
        case QAbstractSocket::ConnectingState: break;
        case QAbstractSocket::ConnectedState: break;
        case QAbstractSocket::BoundState: break;
        case QAbstractSocket::ListeningState: break;
        case QAbstractSocket::ClosingState: break;
        default:break;
    }
    // player actually disconnect?
}

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

void CG_Server::incommingPendingMessage(QByteArray message)
{
    // Login credentials
    QWebSocket* socket = qobject_cast<QWebSocket*>(sender());
    m_output = QJsonDocument::fromBinaryData(message);
    m_rootobj = m_output.object();
    if(m_rootobj.isEmpty() || m_output.isNull() || m_output.isEmpty()){
        return;
    }

    int target = m_rootobj["T"].toInt();
    QJsonArray params = m_rootobj["P"].toArray();
    switch(target)
    {
        case VERIFY_USER:{
            if(params.count() >= 2){
                QString name = params.at(0).toString();
                for(CG_Player player: m_connected){
                    if(player.mUserData.username.compare(name) == 0){
                        userVerified(socket,false); // deny already connected clients
                        return;
                    }
                }
                QString pass_str = params.at(1).toString();
                QByteArray hpass = pass_str.toLatin1();
                m_db.verifyUserCredentials(socket,name,hpass);
            }
            else{ // bad verify message
                qDebug() << "Received Bad Parmaters "
                         << params
                         << "for verify user command from Address "
                         << socket->peerAddress();
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
                m_db.addUser(socket,name,hpass,email,cg);
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
    m_output = QJsonDocument::fromBinaryData(message);
    m_rootobj = m_output.object();
    int target = m_rootobj["T"].toInt();
    QJsonArray params = m_rootobj["P"].toArray();
    CG_Player player = m_connected.value(socket);
    switch(target)
    {
        // LOBBY Messages
        case FETCH_LOBBIES:{
            // no parameters only a return
            emit fetchLobbies(socket);
            break;
        }
        case JOIN_MATCHMAKING:{
            // no parameters only a return
            if(params.count() >= 1 && (player.mWebSocket == socket))
            {
                int game_type = params.at(0).toInt();
                m_lobbyManager.joinMatchMaking(game_type,player);
            }
            break;
        }
        // GAME Messages
        case SEND_SYNC:{
            if(params.count() >= 1){
                quint64  latency(params.at(0).toDouble());
                m_gameManager.sendGameReady(socket,player.mGameID,latency);
            }
            break;
        }
        case SEND_RESULT:{
            if(params.count() >= 4){
                int      result(params.at(0).toInt());
                QJsonObject move(params.at(1).toObject());
                QString     fen(params.at(2).toString());
                QString     last(params.at(3).toString());
                m_gameManager.sendResult(socket,player.mGameID,result,move,fen,last);
            }
            break;
        }
        case SEND_MOVE:{
            if(params.count() >= 4){
                int from(params.at(0).toInt());
                int to(params.at(1).toInt());
                QString fen(params.at(2).toString());
                QString promotion(params.at(3).toString());
                int move_time(params.at(4).toInt());
                quint64 latency(params.at(5).toDouble());
                m_gameManager.makeMove(socket,player.mGameID,from,to,fen,promotion,move_time, latency);
            }
            break;
        }
        case SEND_DRAW:{
            if(params.count() >= 1){
                int draw = params.at(0).toInt();
                m_gameManager.sendDraw(socket,draw,player.mGameID);
            }
            break;
        }

        // PROFILE Messages
        case SET_USER_DATA:{
            if(params.count() >=3){
                QString name = params.at(0).toString();
                QString pass = params.at(1).toString();
                QByteArray hpass = pass.toLatin1();
                QJsonObject data = params.at(2).toObject();
                m_db.setUserData(socket,name,hpass,data);
            }
            break;
        }


        default: break;
    }


}

void CG_Server::networkStateChanged(QNetworkSession::State state)
{
    switch(state){
        case QNetworkSession::Connected:
        case QNetworkSession::Connecting:
        {
            if(!m_reconnectTimer.isActive()){
                m_reconnectTimer.start();
            }
            qDebug() << "Network Re-established, rebinding server soon.";
            break;
        }
        default:
        {
            qDebug() << "Lost Network Connection, starting reconnect timer";
            break;
        }

    }
}

void CG_Server::playerClosing()
{
    QWebSocket * socket = qobject_cast<QWebSocket*>(sender());
    CG_Player player = m_connected.take(socket);
    QWebSocketProtocol::CloseCode c = socket->closeCode();
    socket->disconnect();
    if(player.mGameID > 0){
        m_gameManager.playerDisconnected(socket,player.mGameID);
    }
    if(player.mConnectedLobbies.length() > 0){
        // something mlobby
        m_lobbyManager.leaveMatchMaking(socket);
    }
    switch(c){
        // handle different disconnect reasons.
        case QWebSocketProtocol::CloseCodeGoingAway:{ // normal logout
            break;
        }
        case QWebSocketProtocol::CloseCodeMissingExtension:{ // wrong version of client
            break;
        }
        case QWebSocketProtocol::CloseCodeAbnormalDisconnection:{ // client errored
            break;
        }
    }
}


void CG_Server::playerDropped()
{
    QWebSocket * socket = qobject_cast<QWebSocket*>(sender());
    CG_Player player = m_connected.take(socket);
    //emit notifyPlayerDropped(player.mUserData.username,player.mConnectedLobbies);
    socket->disconnect();
    m_disconnecting.insert(player.mUserData.id,player);
}


void CG_Server::sendAddUserReply(QWebSocket *socket, bool added, int reason)
{
    m_rootobj = QJsonObject();
    m_rootobj["T"] = REGISTER_USER;
    QJsonArray params;
    params.append(added);
    params.append(reason);
    m_rootobj["P"]=params;
    m_output.setObject(m_rootobj);
    socket->sendBinaryMessage(m_output.toBinaryData());
}

void CG_Server::sendConnectedToMatchMaking(QWebSocket *socket, QString type)
{

}

void CG_Server::sendLobbyData(QWebSocket *socket, QByteArray list)
{

}

void CG_Server::userDataSet(QWebSocket *socket, CG_User user)
{
    m_connected[socket].mUserData = user; // update local user
    m_rootobj = QJsonObject();
    m_rootobj["T"] = SET_USER_DATA;
    QJsonArray params;
    params.append(CG_User::serializeUser(user));
    m_rootobj["P"]=params;
    m_output.setObject(m_rootobj);
    socket->sendBinaryMessage(m_output.toBinaryData());
}


void CG_Server::sendDraw(QWebSocket *socket, int draw)
{
    m_rootobj = QJsonObject();
    m_rootobj["T"] = SEND_DRAW;
    QJsonArray params;
    params.append(draw);
    m_rootobj["P"] = params;
    m_output.setObject(m_rootobj);
    socket->sendBinaryMessage(m_output.toBinaryData());
}

void CG_Server::sendMatchedPlayer(QWebSocket *socket, QJsonObject player_data)
{
    m_rootobj = QJsonObject();
    m_rootobj["T"] = MATCHED_PLAYER;
    QJsonArray array;
    array.append(player_data);
    CG_Player player = m_connected.take(socket);
    player.mGameID = quint64(player_data.value("game_id").toDouble());
    m_connected.insert(socket,player);
    m_rootobj["P"] = array;
    m_output.setObject(m_rootobj);
    socket->sendBinaryMessage(m_output.toBinaryData());
}

void CG_Server::sendSynchronizeGame(QWebSocket *socket, int state ,quint64 time)
{
    m_rootobj = QJsonObject();
    m_rootobj["T"] = SEND_SYNC;
    QJsonArray params;
    params.append(state);
    params.append(double(time)); // update players time
    m_rootobj["P"] = params;
    m_output.setObject(m_rootobj);
    socket->sendBinaryMessage(m_output.toBinaryData());
}


void CG_Server::sendReturnMatches(QWebSocket *socket, QString match_data)
{
    m_rootobj = QJsonObject();
    m_rootobj["T"] = FETCH_GAMES;
    QJsonArray params;
    params.append(match_data);
    m_rootobj["P"]=params;
    m_output.setObject(m_rootobj);
    socket->sendBinaryMessage(m_output.toBinaryData());
}

void CG_Server::sendPlayerMadeMove(QWebSocket *socket, int from, int to, QString fen, QString promote, quint64 time)
{
    m_rootobj = QJsonObject();
    m_rootobj["T"] = SEND_MOVE;
    QJsonObject move;
    move["to"] = to;
    move["from"] = from;
    move["fen"] = fen;
    move["promote"] = promote;
    move["time"] = double(time); //opponents time
    QJsonArray array;
    array.append(move);
    m_rootobj["P"]= array;
    m_output.setObject(m_rootobj);
    QByteArray data = m_output.toBinaryData();
    socket->sendBinaryMessage(data);
}

void CG_Server::sendPlayerPostGame(QWebSocket *socket, QString post_data)
{
    m_rootobj = QJsonObject();
    m_rootobj["T"] = SEND_RESULT;
    QJsonArray params;
    params.append(post_data);
    m_rootobj["P"]=params;
    m_output.setObject(m_rootobj);
    socket->sendBinaryMessage(m_output.toBinaryData());
}

void CG_Server::sendOpponentUpdate(QWebSocket *socket, QString data)
{
    m_rootobj = QJsonObject();
    m_rootobj["T"] = OPPONENT_CHANGE;
    QJsonArray params;
    params.append(data);
    m_rootobj["P"]=params;
    m_output.setObject(m_rootobj);
    socket->sendBinaryMessage(m_output.toBinaryData());
}

void CG_Server::refreshUserData(QWebSocket *socket, QString meta, CG_User user)
{
    Q_UNUSED(user)
    m_rootobj = QJsonObject();
    m_rootobj["T"] = REFRESH_USER_DATA;
    QJsonArray params;
    params.append(meta);
    CG_Player player = m_connected.take(socket);
    player.mUserData = user;
    m_connected.insert(socket,player);
    m_rootobj["P"]=params;
    m_output.setObject(m_rootobj);
    socket->sendBinaryMessage(m_output.toBinaryData());
}

void CG_Server::userVerified(QWebSocket *socket, bool verified, QString meta, CG_User user)
{
    m_rootobj = QJsonObject();
    m_rootobj["T"] = VERIFY_USER;
    QJsonArray params;
    params.append(verified);
    if(verified ){
        CG_Player player;
        params.append(meta);
        if(m_disconnecting.contains(user.id)){
            player = m_disconnecting.take(user.id);
            player.mWebSocket = socket;
            user.loggedIn = true;
            player.mUserData = user;
            if(player.mGameID >= 0){
                m_gameManager.reconnectPlayer(player.mGameID, player);
            }
        }
        else{
            player.mWebSocket = socket;
            player.mUserData = user;

        }
        m_connected.insert(socket,player);
        socket->disconnect();
        connect(socket, &QWebSocket::binaryMessageReceived, this, &CG_Server::incommingVerifiedMessage);
        connect(socket,&QWebSocket::disconnected, this, &CG_Server::playerDropped);
        connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(checkPlayerState(QAbstractSocket::SocketError)));
        connect(socket,&QWebSocket::aboutToClose,this, &CG_Server::playerClosing);
        connect(socket,&QWebSocket::pong, this, &CG_Server::socketPong);
        socket->ping();
    }
    m_rootobj["P"]=params;
    m_output.setObject(m_rootobj);
    socket->sendBinaryMessage(m_output.toBinaryData());
}


void CG_Server::socketPong(quint64 elapsed, const QByteArray &message)
{
    QWebSocket * socket = qobject_cast<QWebSocket*>(sender());
    CG_Player player = m_connected.take(socket);
    quint64 client_latency =  message.toDouble();
    qDebug() << "Latency Check for " << player.mUserData.username <<": " << elapsed << "ms";
    qDebug() << "Client Reported: " << client_latency << "ms";
    player.mLatency =elapsed;
    m_connected.insert(socket,player);
    emit playerPing(socket,elapsed,player.mGameID,player.mUserData.username);
}



CG_Server::~CG_Server(){
    if(m_server){
        qDebug() << "Disconnecting server";
        m_server->close(); // will be deleted by CG_Server (QObject)
        m_server->deleteLater();
    }

    if(m_gameThread){
        m_gameThread->quit();
        m_gameThread->exit();
        m_gameThread->deleteLater();
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




