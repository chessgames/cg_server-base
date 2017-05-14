#include "cg_gamemanager.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

CG_GameManager::CG_GameManager(QObject *parent) : QObject(parent)
{

}


int CG_GameManager::matchCount()
{
    return mGames.count();
}


void CG_GameManager::matchedGame(CG_Player black, CG_Player white, quint64 time)
{
    quint64 id = quint64(black.mUserData.elo);
    id += quint64(white.mUserData.elo);
    CG_Game game(white,black,time);
    mGames.insert(id,game);
    QVariantMap bmap;
    bmap.insert("name",black.mUserData.username);
    bmap.insert("elo",black.mUserData.elo);
    bmap.insert("flag",black.mUserData.countryFlag);
    bmap.insert("avatar",black.mUserData.avatar);
    bmap.insert("color",false);
    bmap.insert("id",id);
    QVariantMap wmap;
    wmap.insert("name",white.mUserData.username);
    wmap.insert("elo",white.mUserData.elo);
    wmap.insert("flag",white.mUserData.countryFlag);
    wmap.insert("avatar",white.mUserData.avatar);
    wmap.insert("color",false);
    bmap.insert("id",id);

    QJsonObject obj = QJsonObject::fromVariantMap(bmap);
    QJsonDocument doc;
    doc.setObject(obj);

    QJsonObject wobj = QJsonObject::fromVariantMap(bmap);
    QJsonDocument doc2;
    doc2.setObject(wobj);

    emit notifiedMatchedGame(white.mWebSocket,doc.toJson());
    emit notifiedMatchedGame(black.mWebSocket,doc2.toJson());
}

void CG_GameManager::fetchGames(QWebSocket *socket, int index)
{
    if( index >= 0 && index < mGames.count() )
    {
        int record_count = 0;
        QJsonObject obj;
        QJsonArray  games;
        QJsonDocument doc;
        QMap<quint64,CG_Game>::iterator itr = mGames.end();
        // starting from the back (the highest ranked games) working forward
        for(itr-=index;(itr != mGames.begin() && record_count < 10); record_count++){
            games.append((*itr).serialize());
            itr--;
        }
        obj["games"] = games;
        doc.setObject(obj);
        emit returnGames(socket, doc.toJson());
    }
}


void CG_GameManager::makeMove(QWebSocket *socket, QString move_data)
{
    QJsonDocument doc = QJsonDocument::fromJson(move_data.toLocal8Bit());
    QJsonObject obj = doc.object();
    quint64 id(obj.value("id").toDouble());
    quint32 elapsed_time(obj.value("time").toInt());
    CG_Game game(mGames.value(id));
    QWebSocket * out(nullptr);
    game.makeMove(socket,elapsed_time,obj,out);
    doc.setObject(obj);
    emit sendPlayerMadeMove(out,doc.toJson());
}



void CG_GameManager::sendGameReady(QWebSocket* socket, quint64 id, QString name)
{
    CG_Game game(mGames.value(id));
    QWebSocket *other(game.setReady(socket,name));
    if(other){
        emit notifySynchronizedGame(socket, 0);  // 0 == start
        emit notifySynchronizedGame(other, 0);  // 0 == start
    }
}

void CG_GameManager::chooseColor(QWebSocket * socket, quint64 id, bool color)
{

}

void CG_GameManager::notifySendResult(QWebSocket * socket, quint64 id,QString result)
{
    CG_Game game(mGames.value(id));
    QString result_w;
    QString result_b;
    quint64 date;
    bool finished(game.setResult(socket,result,date,result_b,result_w));
    if(finished){ // tell clients of finished game
        emit updateLastGameDb(game.white().mUserData.id, date, result_w);
        emit updateLastGameDb(game.white().mUserData.id, date, result_w);
        emit notifyPlayerPostGame(game.white().mWebSocket, result_w);
        emit notifyPlayerPostGame(game.black().mWebSocket, result_b);
    }
}




void CG_GameManager::setToAThread(QThread *thread)
{
    moveToThread(thread);
}
