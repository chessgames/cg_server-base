#include "cg_lobbymanager.h"
#include <QStringList>
#include <QJsonObject>
#include <QJsonDocument>
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
                matchPlayers(black,white);
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
                matchPlayers(black,white);
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
                matchPlayers(black,white);
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
    sendPlayerInformation(white.mWebSocket,black,true);
    sendPlayerInformation(black.mWebSocket,white,false);
}


void CG_LobbyManager::sendPlayerInformation(QWebSocket *socket, CG_Player player, bool color)
{
    QVariantMap map;
    map.insert("name",player.mUserData.username);
    map.insert("elo",player.mUserData.elo);
    map.insert("flag",player.mUserData.countryFlag);
    map.insert("avatar","http://chessgames.com/av/face_320.gif");
    map.insert("color",color);
    QJsonObject obj = QJsonObject::fromVariantMap(map);
    QJsonDocument doc;
    doc.setObject(obj);
    QString json;
    json = QString::fromLocal8Bit(doc.toJson());
    emit sendMatchedPlayer(socket,json);
}

CG_LobbyManager::~CG_LobbyManager()
{

}
