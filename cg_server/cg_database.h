#ifndef CG_DATABASE_H
#define CG_DATABASE_H

#include <QObject>
#include "cg_global.h"
#include <QSqlDatabase>
#include <QString>
#include <QThread>
/******************************************************************************
* Class: CG_dbManager
*
* Constructors:
*   CG_Datbase  requires a hostname, user_name, password - if the db
*     has no username or password, pass empty strings
*
* Methods:
*   bool UserExists(QString str_username)
*       Returns whether or not a user with the passed username exists in
*       the database.
*   bool CorrectUserInfo(QString str_username, QString str_password)
*       Returns whether or not the username and password info are correct
*       and in the database.
*   bool AddUser(QString str_username, QString str_password, QString str_email)
*       Returns true if the user is successfully added into the database.
*   QString getCurrentELO(QString str_username)
*       Returns the current ELO of a user.
*
*
* April 14th, 2017 - Carry over from previous database api - attempting to
* keep compatability with code - C.Dean
*
*******************************************************************************/

static const char CG_L[] = "LI";  // Logged in
static const char CG_BAN[] = "BA";// Banned
static const char CG_UN[] = "UN"; // Username
static const char CG_E[] = "EL";  // Elo
static const char CG_CF[] = "CF"; // Country flag
static const char CG_PS[] = "PS"; // Piece Set
static const char CG_LANG[] ="LA";// Language
static const char CG_SND[] = "SN"; // sound
static const char CG_CO[] = "CO"; // Co-ordinates
static const char CG_AR[] = "AR"; // Arrows
static const char CG_AP[] ="AP"; // Auto Promote
static const char CG_BT[] ="BT"; // Board Theme
static const char CG_BF[] ="BF"; // Bit Field

#include <QWebSocket>

class CG_Database : public QObject
{
    Q_OBJECT
public:

    explicit CG_Database(QString host_name = "", QString user = "", QString password = "", int port= -1, QObject *parent = nullptr);
    ~CG_Database();

    static void setUserStruct(CG_User & user, QString name, QString json_settings);
    static QString serializeUser(const CG_User &user);
    bool databaseExists();
    void setToAThread(QThread* thread);

signals:
    void databaseLoadComplete();
    void userVerificationComplete(QWebSocket* socket, bool verified, CG_User data);
    void foundUser(QString name, bool found);
    void addUserReply(QWebSocket* socket, bool added, int reason);
    void userDataSet(QWebSocket * socket, bool set);
    void userRankingUpdated(QWebSocket* socket, int ranking);

public slots:
    bool addUser(QWebSocket* socket, QString str_username, QByteArray pass, QString str_email, QString cg_data);
    bool userExists(QString str_username);
    bool setUserData(QWebSocket * socket, QString name, QByteArray pass, QString data);
    bool updateUserRanking(QWebSocket* socket,QString name, int rank);
    void userRankings(QWebSocket * socket, QString name);
    void verifyUserCredentials(QWebSocket* socket, QString name, QByteArray hpass);

protected:
    QSqlDatabase  m_dbUser; // users and profiles
    QSqlDatabase  m_dbGames; // past games
    QString       m_UserDBPath;
    bool connectToDatabase();
    void clearUserDatabase();
    void createMatchTables();
    bool createUserDatabase();
    bool createUserTables();
    void initializeDatabase(QString path, QString user, QString password, int port);

    // actual methods
    bool puserExists(QString str_username);
    int  paddUser(QString str_username, QByteArray pass, QString str_email, QString cg_data);
    bool pemailExists(QString str_email);
    bool psetUserData(QString name, QByteArray hpass, QString data);
    int puserRankings(QString name);
    bool pupdateUserRanking(QString name, int rank);
    bool pverifyUserCredentials(QString name, QByteArray pass, CG_User &user);

#ifdef CG_TEST_ENABLED
private slots:
    //
    void testAddUser();
    void testAddUser_data();
    void testUserVerify();
    void testUserVerify_data();
    void testUserSetRank();
    void testUserSetRank_data();
    void testUserSettingsUpdate();
    void testUserSettingsUpdate_data();

#endif

};

#endif // CG_DATABASE_H
