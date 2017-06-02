#include "cg_game.h"
#include <math.h>
CG_Game::CG_Game(CG_Player white, CG_Player black, quint64 time)
    :mWhite(white),mBlack(black),mBClock(time),mWClock(time),
      mWResult(-5), mBResult(-5),  mWDraw(-3), mBDraw(-3), mValid(false), // -1 is valid result (loss) -2 is not
      mCurrentState("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")
{}


CG_Game::CG_Game(const CG_Game &right)
    :mBlack(right.mBlack), mWhite(right.mWhite),mBClock(right.mBClock),
     mWClock(right.mWClock), mWResult(right.mWResult), mBResult(right.mBResult),
     mWDraw(right.mWDraw), mBDraw(right.mBDraw),mValid(right.mValid), mCurrentState(right.mCurrentState)
{}


CG_Game& CG_Game::operator =(const CG_Game& right)
{
    mWhite = right.mWhite;
    mBlack = right.mBlack;
    mWClock = right.mWClock;
    mWResult = right.mWResult;
    mWDraw = right.mWDraw;
    mBDraw = right.mBDraw;
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

int CG_Game::blackResult(){
    return mBResult;
}

QWebSocket* CG_Game::makeMove(QWebSocket *socket, quint32 elapsed, QString fen)
{
    QWebSocket* out;
    if(socket == mBlack.mWebSocket){
        out = mWhite.mWebSocket;
        mWClock -= elapsed;
        //obj["time"] = double(mWClock);
    }
    else{
        out = mBlack.mWebSocket;
        mBClock -= elapsed;
        //obj["time"] = double(mBClock);
    }
    mCurrentState = fen;
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


bool CG_Game::setResult(QWebSocket *socket, int result)
{
    bool game_over(false);
    if(mWhite.mWebSocket == socket){ //set corresponding result
        mWResult = result;
        switch(result){
            case -5:{  // clearing result (game reset)
                mBResult = -5;
                mWResult = -5;
                break;
            }
            case -1:{// lost (by checkmate)
                if(mBResult == 1){
                   game_over = true;
                }
                break;
            }
            case 0:{  //  draw stalemate
                if(mBResult == 0){
                    game_over = true;
                }
                break;
            }
            case 1:{   //  won (by checkmate)
                if(mBResult == -1){
                    game_over = true;
                }
                break;
            }
            case 2:{ // white resigned
                mBResult = 2;
                mWResult = -2;
                game_over = true;
                break;
            }
            case 3:{   // draw threefold
                if(mBResult == 3){
                    game_over = true;
                }
                break;
            }
            case 4:{  // draw inssuficient material
                if(mBResult == 4){
                    game_over = true;
                }
                break;
            }
        default: qDebug() << "Bad result received game " << this->mWhite.mGameID <<  " from " << this->mWhite.mUserData.username;
        }
    }
    else{
        mBResult = result;
        switch(result){
            case -5:{  // clearing result (game reset)
                mBResult = -5;
                mWResult = -5;
                break;
            }
            case -1:{// lost (by checkmate)
                if(mWResult == 1){
                   game_over = true;
                }
                break;
            }
            case 0:{  //  draw stalemate
                if(mWResult == 0){
                    game_over = true;
                }
                break;
            }
            case 1:{   //  won (by checkmate)
                if(mWResult == -1){
                    game_over = true;
                }
                break;
            }
            case 2:{ // black resigned
                mWResult = 2;
                mBResult = -2;
                game_over = true;
                break;
            }
            case 3:{   // draw threefold
                if(mWResult == 3){
                    game_over = true;
                }
                break;
            }
            case 4:{  // draw inssuficient material
                if(mWResult == 4){
                    game_over = true;
                }
                break;
            }
        default: qDebug() << "Bad result received game " << this->mBlack.mGameID <<  " from " << this->mBlack.mUserData.username;
        }
    }
    return game_over;
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

int CG_Game::setDraw(QWebSocket *socket, int draw)
{

}

CG_Player& CG_Game::white()
{
    return mWhite;
}

int CG_Game::whiteResult()
{
    return mWResult;
}