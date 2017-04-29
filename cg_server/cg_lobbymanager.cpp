#include "cg_lobbymanager.h"
#include <QStringList>
CG_LobbyManager::CG_LobbyManager(QObject *parent) : QObject(parent)
{

}


void CG_LobbyManager::fetchLobbyList(QWebSocket *socket)
{
    QStringList lobbies;
}


void CG_LobbyManager::joinMatchMaking(int type, CG_Player black)
{
    switch(type){
        case 0:{
            if(mOneMinute.count() > 0)
            {
                CG_Player white = mOneMinute.takeFirst();
            }
            else{
                mOneMinute.append(black);
            }
            break;
        }
        case 1: {
            if(mFiveMinute.count() > 0)
            {
                CG_Player white = mFiveMinute.takeFirst();
            }
            else{
                mFiveMinute.append(black);
            }
        break;
            break;
        }
        case 2: {
            if(mThirtyMinute.count() > 0)
            {
                CG_Player white = mThirtyMinute.takeFirst();

            }
            else{
                mThirtyMinute.append(black);
            }
        break;
           break;
        }
        default:break;
    }
}

void CG_LobbyManager::matchPlayers(CG_Player black, CG_Player white)
{

}

CG_LobbyManager::~CG_LobbyManager()
{

}
