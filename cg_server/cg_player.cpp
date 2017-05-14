#include "cg_player.h"
#include <QWebSocket>

CG_Player::CG_Player()
{

}

QString CG_Player::serialize()
{
    QJsonDocument doc;
    QJsonObject obj;
    obj["name"] = mUserData.username;
    obj["country"] = mUserData.countryFlag;
    obj["elo"] = mUserData.elo;
    obj["avatar"] = mUserData.avatar;
    doc.setObject(obj);
    return doc.toJson();
}


CG_Player::~CG_Player()
{

}
