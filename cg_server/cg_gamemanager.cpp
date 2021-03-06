#include "cg_gamemanager.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <math.h>
CG_GameManager::CG_GameManager(QObject *parent) : QObject(parent)
{

}


void CG_GameManager::checkPendingMatch(QWebSocket * socket, quint64 id){

}

int CG_GameManager::matchCount()
{
    return mGames.count();
}


void CG_GameManager::calculateEloChange(double result, int& elo, int& op_elo)
{

    // Set K-Factor (i.e. this number determines how big the rating changes)
    int K = 45;

    // Calculate Elo from wikipedia formula
    double Ea = 1/(1 + pow(10, ( (op_elo - elo)/400 ) ));
    double Sa(result);
     // Return new Elo (also on wikipedia)
    int temp_elo = (elo + round(K*(Sa - Ea)));

    Ea = 1/(1 + pow(10, ( (elo - op_elo)/400 ) ));
    Sa = 1- Sa; // adjust for opponents elo
    op_elo = (op_elo + round(K*(Sa - Ea)));
    elo = temp_elo;
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


void CG_GameManager::makeMove(QWebSocket *socket, quint64 id, int from, int to, QString fen, QString promote,int time, quint64 latency)
{
    CG_Game *game(mGames.value(id));
    quint64 out_time(0);
    QWebSocket * out(game->makeMove(socket, time, latency, fen,out_time));
    if(out != nullptr){
        emit sendPlayerMadeMove(out,from,to,fen,promote,out_time); // move made
        emit notifySynchronizedGame(socket,1,out_time); // synchronize time
    }
}


void CG_GameManager::playerDisconnected(QWebSocket *socket, quint64 id)
{

}


void CG_GameManager::sendGameReady(QWebSocket* socket, quint64 id, quint64 latency)
{
    CG_Game *game(mGames.value(id));
    QWebSocket *other(nullptr);
    other = game->setReady(socket);
    if(other){
        emit notifySynchronizedGame(socket, 0,game->whiteClock());  // 0 == start
        emit notifySynchronizedGame(other, 0,game->whiteClock());  // 0 == start
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
    if(draw >= 0 && draw <= 2){ // send draw response to other player
        emit sendDrawResponse(game->otherSocket(socket), draw);
    }
//    else{
//        // send reset
//        emit sendDrawResponse(socket, draw);
//        emit sendDrawResponse(game->otherSocket(socket), draw);
//    }
}

void CG_GameManager::sendResult(QWebSocket *socket, quint64 id, double white_t, double black_t, int result,QString last)
{
    if(mGames.contains(id)){
        CG_Game *game(mGames.value(id));
        int elo_w;
        int elo_b;
        quint64 date = QDateTime::currentSecsSinceEpoch();
        bool was_draw(false);
        bool finished(game->setResult(socket,result,white_t,black_t,was_draw));
        if(finished){ // tell clients of finished game
            // the game is over so calculate the elo based on the result
            elo_w = game->white().mUserData.elo;
            elo_b = game->black().mUserData.elo;
            result = game->whiteResult();
            if(was_draw){
                calculateEloChange(0.5,elo_w,elo_b);
            }
            else{
                if(result > 0){
                    calculateEloChange(1,elo_w,elo_b);
                }
                else{
                    calculateEloChange(0,elo_w,elo_b);
                }
            }

            // generate the response for black and white
            QString output;
            QJsonObject result_obj; // object to fill

            // TODO generate PGNj for the game and store it
            result_obj["game"] = last; // both share the game result
            result_obj["result_w"] = game->whiteResult(); // whites result
            result_obj["elo_w"] = elo_w;
            result_obj["elo_b"] = elo_b;
            result_obj["time_w"] = double(game->whiteClock());
            result_obj["time_b"] = double(game->blackClock());
            result_obj["result_b"]  = game->blackResult();

            QJsonDocument doc;
            doc.setObject(result_obj);
            output = doc.toJson(); // serialize results

            // TODO: clean up efficiency of updating ranked match in DB
            // update on the server
            emit updateLastGameDb(game->white().mUserData.id, elo_w, date, output);
            emit updateLastGameDb(game->black().mUserData.id, elo_b, date, output);
            emit updatePlayerRank(game->white().mWebSocket,game->white().mUserData.username, elo_w);
            emit updatePlayerRank(game->black().mWebSocket,game->black().mUserData.username, elo_b);
            emit notifyPlayerPostGame(game->white().mWebSocket, output);
            emit notifyPlayerPostGame(game->black().mWebSocket, output);


            // clean up the game
            game = mGames.take(id);
            delete game;
        }
    }
}


void CG_GameManager::sendPlayerUpdate(QWebSocket *socket, QString meta, CG_User data)
{
    Q_UNUSED(meta)
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
