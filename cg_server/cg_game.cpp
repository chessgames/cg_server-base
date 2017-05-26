#include "cg_game.h"
#include <math.h>
CG_Game::CG_Game(CG_Player white, CG_Player black, quint64 time)
    :mWhite(white),mBlack(black),mBClock(time),mWClock(time),
      mWResult(-5), mBResult(-5), mValid(false), // -1 is valid result (loss) -2 is not
      mCurrentState("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")
{}


CG_Game::CG_Game(const CG_Game &right)
    :mBlack(right.mBlack), mWhite(right.mWhite),mBClock(right.mBClock),
     mWClock(right.mWClock), mWResult(right.mWResult), mBResult(right.mBResult),
     mValid(right.mValid), mCurrentState(right.mCurrentState)
{}


CG_Game& CG_Game::operator =(const CG_Game& right)
{
    mWhite = right.mWhite;
    mBlack = right.mBlack;
    mWClock = right.mWClock;
    mWResult = right.mWResult;
    mBResult = right.mBResult;
    mBClock = right.mBClock;
    mValid = right.mValid;
    mCurrentState = right.mCurrentState;
    return *this;
}


CG_Player& CG_Game::black()
{
    return mBlack;
}

QWebSocket* CG_Game::makeMove(QWebSocket *socket, quint32 elapsed, QJsonObject & obj)
{
    QWebSocket* out;
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
    mCurrentState = obj["fen"].toString();
    return out;
}


quint64 CG_Game::pair(quint64 w, quint64 b)
{
    // perform cantor pairing to generate unique id
    quint64 out(0);
    out += (.5 * (w+b) * (w+b+1) + b);
    return out;
}

bool CG_Game::isValid()
{
    return mValid;
}

void CG_Game::setFEN(QString fen)
{
    mCurrentState = fen;
}

QJsonObject CG_Game::serialize()
{
    QJsonObject obj;
    obj["white"] = mWhite.serialize();
    obj["black"] = mBlack.serialize();
    obj["spectators"] = mSpectators.count();
    obj["id"] = double(pair(quint64(mWhite.mWebSocket), quint64(mBlack.mWebSocket)));
    obj["fen"] = mCurrentState;
    return obj;
}


QWebSocket* CG_Game::otherSocket(QWebSocket* socket)
{
    if(mWhite.mWebSocket == socket){
        return mBlack.mWebSocket;
    }
    else{
        return mWhite.mWebSocket;
    }
}

QWebSocket* CG_Game::setReady(QWebSocket *&socket)
{
    if(mWhite.mWebSocket == socket){
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


int CG_Game::setResult(QWebSocket *socket, int result,  int &elo_b, int &elo_w)
{
    if(mWhite.mWebSocket == socket){ //set corresponding result
        mWResult = result;
    }
    else{
        mBResult = result;
    }
    if(mWResult == -5){ // not set
        return -5;
    }
    if(mBResult == -5){ // not set
        return -5;
    }
    if(mWResult < -1 || mWResult > 1)
    {
        qDebug() << "Errored Result White client " << "Result: " << mWResult << " for " << mWhite.mUserData.username  <<" @ " << mWhite.mWebSocket->peerAddress();
       mWResult = mBResult;
    }

    if(mBResult < -1 || mBResult > 1)
    {
        qDebug() << "Errored Result Black client " << "Result: " << mBResult << " for " << mBlack.mUserData.username  <<" @ " << mBlack.mWebSocket->peerAddress();
        mBResult = mWResult;
    }
    if(abs(mWResult) == abs(mBResult) ){ // both 1 or both 0
        // somebody won or it is a draw
        if(mWResult == 0){ // draw
            elo_b = 0;
            elo_w = 0;
            return 0;
        }
        else{
            if(mWResult == 1){ //white wone
                elo_w = 15;
                elo_b = -15;
                return 1;
            }
            else{  // black won
                elo_w = -15;
                elo_b = 15;
                return -1;
            }
        }
    }
    return -5;
}

QWebSocket * CG_Game::reconnectPlayer(const CG_Player &player,quint64 & id)
{
    if(player.mUserData.id == mWhite.mUserData.id){
        mWhite = player;
        id = pair(quint64(mWhite.mWebSocket), quint64(mBlack.mWebSocket));
        return mBlack.mWebSocket;
    }
    else
    {
        mBlack = player;
        id = pair(quint64(mWhite.mWebSocket), quint64(mBlack.mWebSocket));
        return mWhite.mWebSocket;
    }
}


CG_Player& CG_Game::white()
{
    return mWhite;
}
