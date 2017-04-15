#include "cg_server.h"
#include <QWebSocketServer>
#include "cg_database.h"
CG_Server::CG_Server(QObject *parent) : QObject(parent), m_db(parent), m_server(nullptr)
{
    m_lobbies.insert(QStringLiteral("All"),CG_PlayerList());
    m_lobbies.insert(QStringLiteral("1M"),CG_PlayerList());
    m_lobbies.insert(QStringLiteral("5M"),CG_PlayerList());
    m_lobbies.insert(QStringLiteral("30M"),CG_PlayerList());
}

bool CG_Server::startToListen(QHostAddress addr, quint16 port)
{
    if(m_server == nullptr){
        m_server = new QWebSocketServer("CG_Server",QWebSocketServer::NonSecureMode,this); // http connection
    }
    Q_ASSERT(m_server);
    bool listening(m_server->listen(addr,port));
    if(!listening){
        qDebug() << "Failed to start the server for: " << addr << " @ " << port;
        Q_ASSERT(m_server->isListening());
    }
    else{
        qDebug() << "Started the server on: " << addr << " @ " << port;
    }
    return listening;
}

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
    return m_lobbies.value("All").count();
}

int CG_Server::getQueueCount()
{
    return m_lobbies.value("1M").count();
}

CG_Server::~CG_Server(){
    if(m_server){
        qDebug() << "Disconnecting server";
        m_server->close(); // will be deleted by CG_Server (QObject)
        m_server->deleteLater();
    }
    m_db.deleteLater();
}
