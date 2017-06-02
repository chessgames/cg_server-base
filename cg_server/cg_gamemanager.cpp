#include "cg_gamemanager.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

CG_GameManager::CG_GameManager(QObject *parent) : QObject(parent)
{

}


void CG_GameManager::checkPendingMatch(QWebSocket * socket, quint64 id){

}

int CG_GameManager::matchCount()
{
    return mGames.count();
}


void CG_GameManager::calculateEloChange(int result, int& elo, int& op_elo){
    switch(result){

        default: break;
    }
}

void CG_GameManager::matchedGame(CG_Player black, CG_Player white, quint64 time)
{
    quint64 id = CG_Game::pair(quint64(black.mWebSocket), quint64(white.mWebSocket));
    black.mGameID = id;
    white.mGameID = id;
    black.mColor = false;
    CG_Game *game = new CG_Game(white,black,time);
    mGames.insert(id,game);

    emit notifiedMatchedGame(white.mWebSocket,black.json());
    emit notifiedMatchedGame(black.mWebSocket,white.json());
}



void CG_GameManager::fetchGames(QWebSocket *socket, int index)
{
    if( index >= 0 && index < mGames.count() )
    {
        int record_count = 0;
        QJsonObject obj;
        QJsonArray  games;
        QJsonDocument doc;
        QMap<quint64,CG_Game*>::iterator itr = mGames.end();
        // starting from the back (the highest ranked games) working forward
        for(itr-=index;(itr != mGames.begin() && record_count < 10); record_count++){
            games.append((*itr)->serialize());
            itr--;
        }
        obj["games"] = games;
        doc.setObject(obj);
        emit returnGames(socket, doc.toJson());
    }
}

CG_Game* CG_GameManager::findGame(QWebSocket *player)
{
    for(CG_Game* game : mGames.values()){
        if(game->white().mWebSocket == player || game->black().mWebSocket == player ){
            return game;
        }
    }
    return nullptr;
}


void CG_GameManager::makeMove(QWebSocket *socket, quint64 id, int from, int to, QString fen, QString promote)
{
   // QJsonDocument doc = QJsonDocument::fromJson(move_data.toLocal8Bit());
   // QJsonObject obj = doc.object();
    //quint32 elapsed_time(obj.value("time").toInt());
    CG_Game *game(mGames.value(id));

    QWebSocket * out(game->makeMove(socket, 4000, fen));
    //game.makeMove(socket,elapsed_time,obj,out);
    //doc.setObject(obj);
    emit sendPlayerMadeMove(out,from,to,fen,promote);
}


void CG_GameManager::playerDisconnected(QWebSocket *socket, quint64 id)
{

}


void CG_GameManager::sendGameReady(QWebSocket* socket, quint64 id)
{
    CG_Game *game(mGames.value(id));
    QWebSocket *other(nullptr);
    other = game->setReady(socket);
    if(other){
        emit notifySynchronizedGame(socket, 0);  // 0 == start
        emit notifySynchronizedGame(other, 0);  // 0 == start
    }
}

void CG_GameManager::chooseColor(QWebSocket * socket, quint64 id, bool color)
{

}


void CG_GameManager::reconnectPlayer(quint64 id, CG_Player player)
{
    if(mGames.contains(id)){
        CG_Game *game(mGames.take(id));
        QWebSocket * other(game->reconnectPlayer(player,id));
        mGames.insert(id,game);
        //notifyOpponentReconnect(other,game->);
        QJsonObject obj;
        obj["name"] = player.mUserData.username;
        obj["elo"] = player.mUserData.elo;
        obj["flag"] = player.mUserData.countryFlag;
        obj["avatar"] = player.mUserData.avatar;
        obj["game_id"] = double(player.mGameID);
        QJsonDocument doc;
        doc.setObject(obj);
        emit notifyOpponentReconnect(other, doc.toBinaryData());
    }
}

void CG_GameManager::sendDraw(QWebSocket *socket, int response, quint64 id)
{
    CG_Game *game(mGames.value(id));
    int draw(game->setDraw(socket,response));
    switch(draw){

    }
       // emit notifySynchronizedGame(socket, 0);  // 0 == start
        //emit notifySynchronizedGame(other, 0);  // 0 == start
}

void CG_GameManager::sendResult(QWebSocket *socket, quint64 id, int result, QJsonObject move, QString fen, QString last)
{
    if(mGames.contains(id)){
        CG_Game *game(mGames.value(id));
        int elo_w;
        int elo_b;
        quint64 date = QDateTime::currentSecsSinceEpoch();
        bool finished(game->setResult(socket,result));
        if(finished){ // tell clients of finished game
            QJsonObject result_obj;
            result_obj["move"] = move;
            result_obj["fen"] = fen;
            result_obj["game"] = last;
            elo_w = game->white().mUserData.elo;
            elo_b = game->black().mUserData.elo;
            QString result_w;
            QJsonDocument doc;
            QString result_b;
            result_obj["result"] = game->whiteResult();
            doc.setObject(result_obj);
            result_w = doc.toJson();
            result_obj["result"]  = game->blackResult();
            doc.setObject(result_obj);
            result_b = doc.toJson();
            calculateEloChange(game->whiteResult(),elo_w,elo_b);
            emit updateLastGameDb(game->white().mUserData.id, elo_w, date, result_w);
            emit updateLastGameDb(game->black().mUserData.id, elo_b, date, result_b);
            emit notifyPlayerPostGame(game->white().mWebSocket, result_w);
            emit notifyPlayerPostGame(game->black().mWebSocket, result_b);
            game = mGames.take(id);
            delete game;
        }
    }
}


void CG_GameManager::sendPlayerUpdate(QWebSocket *socket, CG_User data)
{
    CG_Game *game(findGame(socket));
    QVariantMap bmap;
    if(game == nullptr){
        return;
    }
    if(game->white().mWebSocket == socket){
        game->white().mUserData = data;
        CG_Player & white = game->white();
        bmap.insert("name",white.mUserData.username);
        bmap.insert("elo",white.mUserData.elo);
        bmap.insert("flag",white.mUserData.countryFlag);
        bmap.insert("avatar",white.mUserData.avatar);
        bmap.insert("color",false);
        bmap.insert("id",white.mUserData.id);
        QJsonDocument doc;
        doc.setObject(QJsonObject::fromVariantMap(bmap));
        emit notifyPlayerChanged(game->black().mWebSocket,doc.toJson());
    }
    else if(game->black().mWebSocket == socket){
        game->black().mUserData = data;
        CG_Player & black = game->black();
        bmap.insert("name",black.mUserData.username);
        bmap.insert("elo",black.mUserData.elo);
        bmap.insert("flag",black.mUserData.countryFlag);
        bmap.insert("avatar",black.mUserData.avatar);
        bmap.insert("color",false);
        bmap.insert("id",black.mUserData.id);
        QJsonDocument doc;
        doc.setObject(QJsonObject::fromVariantMap(bmap));
        emit notifyPlayerChanged(game->white().mWebSocket,doc.toJson());
    }

}


void CG_GameManager::setToAThread(QThread *thread)
{
    moveToThread(thread);
}


CG_GameManager::~CG_GameManager()
{
    for(CG_Game* game : mGames.values()){
        delete game;
    }
}