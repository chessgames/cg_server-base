#include "cg_lobbymanager.h"
#include <QStringList>
CG_LobbyManager::CG_LobbyManager(QObject *parent) : QObject(parent)
{

}


void CG_LobbyManager::fetchLobbyList(QWebSocket *socket)
{
    QStringList lobbies;
}


CG_LobbyManager::~CG_LobbyManager()
{

}
