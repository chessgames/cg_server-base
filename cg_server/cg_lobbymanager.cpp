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
                emit matchedPlayers(black,white,60000);
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
                emit matchedPlayers(black,white,300000);
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
                emit matchedPlayers(black,white,1800000);
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

void CG_LobbyManager::leaveMatchMaking(QWebSocket *socket){
    if(mOneMinute.size() > 0){
        if(mOneMinute.at(0).mWebSocket == socket){
            mOneMinute.takeFirst();
            return;
        }
    }
    if(mFiveMinute.size() > 0){
        if(mFiveMinute.at(0).mWebSocket == socket){
            mFiveMinute.takeFirst();
            return;
        }
    }
    if(mThirtyMinute.size() > 0){
        if(mThirtyMinute.at(0).mWebSocket == socket){
            mThirtyMinute.takeFirst();
            return;
        }
    }
}


void CG_LobbyManager::sendPlayerInformation(QWebSocket *socket, CG_Player player, bool color)
{
    QVariantMap map;
    map.insert("name",player.mUserData.username);
    map.insert("elo",player.mUserData.elo);
    map.insert("flag",player.mUserData.countryFlag);
    map.insert("avatar",player.mUserData.avatar);
    map.insert("color",color);

    QJsonObject obj = QJsonObject::fromVariantMap(map);
    QJsonDocument doc;
    doc.setObject(obj);
    QString json;
    json = QString::fromLocal8Bit(doc.toJson());
}

void CG_LobbyManager::setToAThread(QThread *thread)
{
    moveToThread(thread);
}

CG_LobbyManager::~CG_LobbyManager()
{

}
