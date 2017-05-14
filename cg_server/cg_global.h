#ifndef CG_GLOBAL_H
#define CG_GLOBAL_H

#include <QString>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>


static const char CG_L[] = "LI";  // Logged in
static const char CG_BAN[] = "BA";// Banned
static const char CG_UN[] = "UN"; // Username
static const char CG_E[] = "EL";  // Elo
static const char CG_CF[] = "CF"; // Country flag
static const char CG_PS[] = "PS"; // Piece Set
static const char CG_LANG[] ="LA";// Language
static const char CG_TOTL[] ="TG";// Total games played
static const char CG_WON[] ="WG";// Total Won games
static const char CG_SND[] = "SN"; // sound
static const char CG_CO[] = "CO"; // Co-ordinates
static const char CG_AR[] = "AR"; // Arrows
static const char CG_AP[] ="AP"; // Auto Promote
static const char CG_BT[] ="BT"; // Board Theme
static const char CG_BF[] ="BF"; // Bit Field


class CG_User
{
public:
    bool     loggedIn = false;
    bool     banned = false;
    QString  username = "";
    int      elo = 0;
    QString  countryFlag = "United States";
    int      pieceSet = 0;
    int      language = 0;
    int      id;
    bool     sound = false;
    bool     coordinates = false;
    bool     arrows = false;
    bool     autoPromote = false;
    quint32  total = 0;
    quint32  won = 0;
    QString  boardTheme = "";
    QString  avatar;
    quint32  cgbitfield = 0;
    bool     isValid = false;
    bool operator ==(const CG_User & right)
    {
        return (pieceSet == right.pieceSet && language == right.language && sound == right.sound
                && coordinates == right.coordinates && arrows == right.arrows && autoPromote == right.autoPromote);
    }

    static void setUserStruct(CG_User & user, QString name, QString json_settings)
    {
        user.username = name;
        QJsonDocument doc = QJsonDocument::fromJson(json_settings.toLocal8Bit());
        QJsonObject obj = doc.object();
        // get data out of obj
        user.arrows = obj.value(CG_AR).toBool();
        user.autoPromote = obj.value(CG_AP).toBool();
        user.banned = obj.value(CG_BAN).toBool();
        user.boardTheme = obj.value(CG_BT).toString();
        user.cgbitfield = quint32(obj.value(CG_BF).toInt());
        user.coordinates = obj.value(CG_CO).toBool();
        user.countryFlag = obj.value(CG_CF).toString();
        user.elo = obj.value(CG_E).toInt();
        user.total = quint32(obj.value(CG_TOTL).toInt());
        user.won = quint32(obj.value(CG_WON).toInt());
        user.language = obj.value(CG_LANG).toInt();
        user.pieceSet = obj.value(CG_PS).toInt();
        user.sound = obj.value(CG_SND).toBool();
        user.isValid = true;
    }

    static QString serializeUser(const CG_User &user)
    {
        QString out;
        QJsonObject obj;
        // TODO: Add values from match table
        obj[CG_UN] = user.username;
        obj[CG_AR] =user.arrows;
        obj[CG_AP] = user.autoPromote;
        obj[CG_BAN] = user.banned;
        obj[CG_BT] = user.boardTheme;
        obj[CG_BF] = int(user.cgbitfield);
        obj[CG_CO] = user.coordinates;
        obj[CG_CF] = user.countryFlag;
        obj[CG_E] = user.elo;
        obj[CG_TOTL] = int(user.total);
        obj[CG_WON] = int(user.won);
        obj[CG_LANG] = user.language;
        obj[CG_SND] = user.sound;
        obj[CG_PS] = user.pieceSet;
        QJsonDocument doc;
        doc.setObject(obj);
        out = doc.toJson();

        return out;
    }
};


static QDebug operator<<(QDebug dbg, const CG_User &user)
{
    if(user.isValid){
        dbg.nospace() << "CG_User{";
        dbg.nospace() << "\nName: " << user.username;
        dbg.nospace() << "\nArrows: " << user.arrows;
        dbg.nospace() << "\nAvatar: " << user.avatar;
        dbg.nospace() << "\nAutoPromote: " << user.autoPromote;
        dbg.nospace() << "\nBanned: " << user.banned;
        dbg.nospace() << "\nBoardTheme: " << user.boardTheme;
        dbg.nospace() << "\nBitField: " << user.cgbitfield;
        dbg.nospace() << "\nCoordinates: " << user.coordinates;
        dbg.nospace() << "\nFlag: " << user.countryFlag;
        dbg.nospace() << "\nElo: " << user.elo;
        dbg.nospace() << "\nLanguage: " << user.language;
        dbg.nospace() << "\nLoggedIn: " << user.loggedIn;
        dbg.nospace() << "\nPieceSet: " << user.pieceSet;
        dbg.nospace() << "\nSound: " << user.sound;
        dbg.nospace() << "\n}";
    }
    return dbg.maybeSpace();
}



// server globals

// Login
static const int VERIFY_USER =  453;
static const int REGISTER_USER =  543;

// Profile
static const int SET_USER_DATA     =  6895;
static const int FETCH_GAMES       =  6875;
static const int FETCH_FAVES       =  6885;
static const int REFRESH_USER_DATA =  6896;

// Lobby
static const int JOIN_MATCHMAKING =  4541;
static const int SEND_MESSAGE     =  4512;
static const int JOIN_LOBBY       =  4321;
static const int LEAVE_LOBBY      =  4231;
static const int FETCH_LOBBIES    =  4555;
static const int MATCHED_PLAYER   =  4565;
static const int CANCEL_MATCHING  =  4454;


// Game
static const int CHOOSE_COLOR   =  5114;
static const int SEND_MOVE      =  5345;
static const int FORFEIT_MATCH  =  5899;
static const int AVATAR_CHANGED =  5235;
static const int FLAG_CHANGED   =  5236;
static const int SEND_SYNC      =  5461;
static const int SEND_RESULT    =  5246;



#endif // CG_GLOBAL_H
