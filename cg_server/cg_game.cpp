#include "cg_game.h"
#include <math.h>
CG_Game::CG_Game(CG_Player white, CG_Player black, quint64 time)
    :mWhite(white),mBlack(black),mBClock(time),mWClock(time),
      mWResult(-5), mBResult(-5),  mWDraw(-3), mBDraw(-3), mValid(false), // -1 is valid result (loss) -2 is not
      mCurrentState("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")
{}


CG_Game::CG_Game(const CG_Game &right)
    :mBlack(right.mBlack),mBClock(right.mBClock), mWhite(right.mWhite),
     mWResult(right.mWResult), mWClock(right.mWClock), mBResult(right.mBResult),
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

quint64 CG_Game::blackClock()
{
    return mBClock;
}

int CG_Game::blackResult(){
    return mBResult;
}

QWebSocket* CG_Game::makeMove(QWebSocket *socket, int elapsed, quint64 ping, QString fen, quint64 &out_time)
{
    int elapsed_s = mTurnTime.elapsed();
    QWebSocket* out(nullptr);
    if(socket == mBlack.mWebSocket){
        out = mWhite.mWebSocket;
        if(elapsed_s > ((ping*2) + elapsed + 300)){
            mBClock -= (elapsed_s*.9); // adjusted elapsed
        }
        else
        {
            mBClock -= elapsed;// elapsed is within reason
        }
        out_time = mBClock;
        //obj["time"] = double(mWClock);
    }
    else{
        out = mBlack.mWebSocket;
        if(elapsed_s > ((ping*2) +elapsed + 300)){
            mWClock -= (elapsed_s *.9); // adjusted elapsed
        }
        else
        {
            mWClock -= elapsed;// elapsed is within reason
        }
        out_time = mWClock;
        //obj["time"] = double(mBClock);
    }
    mCurrentState = fen;
    mTurnTime.start();
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
    obj["wclock"] =  double(mWClock);
    obj["bclock"] = double(mBClock);

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
        mTurnTime.start();
        socket = mWhite.mWebSocket;
        return mBlack.mWebSocket;
    }
    return nullptr;
}


bool CG_Game::setResult(QWebSocket *socket, int result, double white_t, double black_t, bool & draw)
{
    //  Results TABLE
    //  Player Win (I Win by Checkmate)              : 1
    //  Other Player Win (Won by checkmate)          : -1
    //  Drawn by Agreement                           : 0
    //  Drawn by Stalemate                           : 10
    //  Drawn by 50 Move Rule                        : 11
    //  Drawn by 3 Move Repetition                   : 12
    //  Drawn by Lack of Material                    : 13
    //          TODO: ADD LAM "book draws", "naked king + time"
    //  My Time expired                              : 2
    //  Opponents time expired                       : -2
    //  Player Resigned                              : 3
    //  Other Player Resigned                        : -3


    bool game_over(false);
    if(mWhite.mWebSocket == socket){ //set corresponding result
        mWResult = result;
        if(result == -3)
        {
            mBResult = 3; // won by resignation
        }
    }
    else{
        mBResult = result;
        if(result == -3)
        {
            mWResult = 3; // won by resignation
        }
    }

    if(mWResult == mBResult){
        game_over = true;
        draw = true;
        mBClock = black_t;
        mWClock = white_t;
    }
    else if (mWResult == (mBResult * -1))
    {
        game_over = true;
        mBClock = black_t;
        mWClock = white_t;
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
    int ret_val(-3); // bad state
    if(mWhite.mWebSocket == socket){ // whites response
        switch(draw){
            case 0: // white offering draw
                mWDraw = draw;
                ret_val = draw;
                break;
            case 1: //white accepted
                if(mBDraw == 0){ // black
                    ret_val = 1; // send accept
                }
                mWDraw = -3;
                mBDraw = -3;
                break;

            case 2: //white declined
                if(mBDraw == 0){ // black offered
                    ret_val = 2; //send decline
                }
                mWDraw = -3;
                mBDraw = -3;
                break;
            default:
                mWDraw = -3;
                mBDraw = -3;
            break;
        }
    }
    else{
        switch(draw){
            case 0: // black offering draw
                mBDraw = draw;
                ret_val = draw;
                break;
            case 1: //black accepted
                if(mWDraw == 0){ // white offered
                    ret_val = 1; // send accept
                }
                mWDraw = -3;
                mBDraw = -3;
                break;

            case 2: // black declined
                if(mWDraw == 0){ // white offered
                    ret_val = 2; //send decline
                }
                mWDraw = -3;
                mBDraw = -3;
                break;
            default:
                mWDraw = -3;
                mBDraw = -3;
            break;
        }
    }
    return ret_val;
}

CG_Player& CG_Game::white()
{
    return mWhite;
}

quint64 CG_Game::whiteClock()
{
    return mWClock;
}

int CG_Game::whiteResult()
{
    return mWResult;
}
