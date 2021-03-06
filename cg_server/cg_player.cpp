#include "cg_player.h"
#include <QWebSocket>

CG_Player::CG_Player()
 : mReady(false),mWebSocket(nullptr),mGameID(-1), mColor(true), mLatency(0)
{}

CG_Player::CG_Player(const CG_Player &right):
    mConnectedLobbies(right.mConnectedLobbies), mUserData(right.mUserData),
    mReady(right.mReady), mWebSocket(right.mWebSocket), mGameID(right.mGameID),
    mColor(right.mColor)
{}

CG_Player& CG_Player::operator=(const CG_Player &right)
{
    mConnectedLobbies.clear();
    mConnectedLobbies = right.mConnectedLobbies;
    mUserData = right.mUserData;
    mReady = right.mReady;
    mWebSocket = right.mWebSocket;
    mGameID = right.mGameID;
    mColor = right.mColor;
    return *this;
}

QString CG_Player::serialize()
{
    QJsonDocument doc;
    QJsonObject obj;
    obj["name"] = mUserData.username;
    obj["flag"] = mUserData.countryFlag;
    obj["elo"] = mUserData.elo;
    obj["avatar"] = mUserData.avatar;
    obj["game_id"] = double(mGameID);
    obj["color"] = mColor;
    doc.setObject(obj);
    return doc.toJson();
}

QJsonObject CG_Player::json()
{
    QJsonObject obj;
    obj["name"] = mUserData.username;
    obj["flag"] = mUserData.countryFlag;
    obj["elo"] = mUserData.elo;
    obj["avatar"] = mUserData.avatar;
    obj["game_id"] = double(mGameID);
    obj["color"] = mColor;
    return obj;
}

CG_Player::~CG_Player()
{

}
