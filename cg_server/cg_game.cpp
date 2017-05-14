#include "cg_game.h"
#include <math.h>
CG_Game::CG_Game(CG_Player white, CG_Player black, quint64 time)
    :mBClock(time),mWClock(time), mWResult(-2), mBResult(-2) // -1 is valid result (loss) -2 is not
{}


CG_Game::CG_Game(const CG_Game &right)
    :mBlack(right.mBlack), mWhite(right.mWhite),mBClock(right.mBClock),
     mWClock(right.mWClock), mWResult(right.mWResult), mBResult(right.mBResult)
{}


CG_Game& CG_Game::operator =(const CG_Game& right)
{
    mWhite = right.mWhite;
    mBlack = right.mBlack;
    mWClock = right.mWClock;
    mWResult = right.mWResult;
    mBResult = right.mBResult;
    mBClock = right.mBClock;
}


CG_Player CG_Game::black()
{
    return mBlack;
}

void CG_Game::makeMove(QWebSocket *socket, quint32 elapsed, QJsonObject & obj, QWebSocket *& out)
{
    if(socket == mBlack.mWebSocket){
        out = mWhite.mWebSocket;
        mWClock -= elapsed;
        obj["time"] = double(mWClock);
    }
    else{
        out = mBlack.mWebSocket;
        mBClock -= elapsed;
        obj["time"] = double(mBClock);
    }
    QJsonDocument doc;
    doc.setObject(obj);
    QString move = doc.toJson();
    if(!mPgn.isEmpty()){
        mPgn.append(",");
    }
    mPgn.append(move);

}

void CG_Game::setPgn(QString pgn)
{
    mPgn = pgn;
}

QString CG_Game::serialize()
{
    QJsonObject obj;
    obj["white"] = mWhite.serialize();
    obj["black"] = mBlack.serialize();
    obj["id"] = (mWhite.mUserData.elo + mBlack.mUserData.elo);
    obj["spectators"] = mSpectators.count();
    QString pgn_out('[');
    pgn_out.append(mPgn);
    pgn_out.append(']');
    obj["pgn"] = mPgn;
    QJsonDocument doc;
    doc.setObject(obj);
    return doc.toJson();
}


QWebSocket* CG_Game::setReady(QWebSocket *&socket, QString name)
{
    if(mWhite.mUserData.username.compare(name) == 0){
        mWhite.mReady = true;
    }
    else{
        mBlack.mReady = true;
    }
    if(mBlack.mReady && mWhite.mReady){
        socket = mWhite.mWebSocket;
        return mBlack.mWebSocket;
    }
    return nullptr;
}


bool CG_Game::setResult(QWebSocket *socket, QString result, quint64 date, QString &post_datab, QString &post_dataw)
{
    int iresult(-1);
    QJsonDocument doc = QJsonDocument::fromJson(result.toLocal8Bit());
    QJsonObject obj = doc.object();

    iresult = obj.value("result").toInt();
    date = QDateTime::currentMSecsSinceEpoch(); // set the date

    if(mWhite.mWebSocket == socket){ //set corresponding result
        mWResult = iresult;
    }
    else{
        mBResult = iresult;
    }

    if(mWResult < -2 || mWResult > 1)
    {
        qDebug() << "Errored Result White client " << "Result: " << mWResult << " for " << mWhite.mUserData.username  <<" @ " << mWhite.mWebSocket->peerAddress();
       mWResult = mBResult;
    }

    if(mBResult < -2 || mBResult > 1)
    {
        qDebug() << "Errored Result Black client " << "Result: " << mBResult << " for " << mBlack.mUserData.username  <<" @ " << mBlack.mWebSocket->peerAddress();
        mBResult = mWResult;
    }
    if(abs(mWResult) == abs(mBResult) ){ // both 1 or both 0
        // somebody won or it is a draw
        if(mWResult == 0){ // draw
            // draw so the sent data is correct for both
            obj["id"] = int(mWhite.mUserData.total++); // set id to number of games white
            obj["result"] = 0;
            obj["eloc"] = 0;
            doc.setObject(obj);
            post_dataw = doc.toJson();
            obj["id"] = int(mBlack.mUserData.total++); // set id to number of games black
            obj["result"] = 0;
            obj["eloc"] = 0;
            doc.setObject(obj);
            post_datab = doc.toJson();
        }
        else{
            if(mWResult == 1){ //white wone

            }
            else{  // black won

            }
        }
        return true;
    }
    else{ // check for an error?

    }
    return false;
}


CG_Player CG_Game::white()
{
    return mWhite;
}
