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
*	CG_dbManager(QString str_connection)
*       Opens the SQLite database for connection based upon the db path passed.
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

    explicit CG_Database(QString user_db_path, QString user_name, QString password, QObject *parent = nullptr);
    ~CG_Database();

    static void setUserStruct(CG_User & user, QString name, QString json_settings);
    static QString serializeUser(const CG_User &user);
    bool databaseExists(QString path);
    void setToAThread(QThread* thread);

signals:
    void databaseLoadComplete();
    void userVerificationComplete(QWebSocket* socket, bool verified, CG_User data);
    void foundUser(QString name, bool found);
    void addUserReply(QWebSocket* socket, bool added, int reason);
    void userDataSet(QWebSocket * socket, bool set);

public slots:
    void verifyUserCredentials(QWebSocket* socket, QString name, QByteArray hpass);
    bool addUser(QWebSocket* socket,QString str_username, QByteArray pass, QString str_email);
    bool userExists(QString str_username);
    bool setUserData(QWebSocket * socket, QString name, QByteArray pass, QString data);
    void userRankings(QWebSocket * socket, QString name);

protected:
    QSqlDatabase  m_dbUser; // users and profiles
    QSqlDatabase  m_dbGames; // past games
    QString       m_UserDBPath;
    void createMatchTables();
    void clearUserDatabase();
    void createUserTables();

    // actualy methods
    bool puserExists(QString str_username);
    int paddUser(QString str_username, QByteArray pass, QString str_email);
    bool pemailExists(QString str_email);
    //bool psetUserCredentials(QString str_username, QByteArray new_pass, QByteArray old_pass);
    //bool pupdateCountryFlag(QString str_username, int country_flag);
    //bool pupdateUserInfo(CG_User user);
    //int pgetCountryFlag(QString str_username);
    //void pencryptPassword(QString & password);
    bool psetUserData(QString name, QString data);
    bool pverifyUserCredentials(QString name, QByteArray pass, CG_User &user);
    //bool pupdateCurrentELO(QString str_username, int elo);
    QString pgetCurrentELO(QString str_username);
    //void pgetUserInfo(CG_User &user, QString str_username);

#ifdef CG_TEST_ENABLED
private slots:
    void createUserDatabase();
    void testAddUser();
    void testAddUser_data();
    void testUserVerify();
    void testUserVerify_data();
#else
public:
    void createUserDatabase();

#endif

};

#endif // CG_DATABASE_H
