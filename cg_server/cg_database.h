#ifndef CG_DATABASE_H
#define CG_DATABASE_H

#include <QObject>
#include "cg_global.h"
#include <QSqlDatabase>
#include <QString>
#include <QThread>
#include <QDateTime>
#include "cg_usergraph.h"

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

#include <QWebSocket>

class CG_Database : public QObject
{
    Q_OBJECT
public:

    explicit CG_Database(QString host_name = "", QString user = "", QString password = "", int port= -1, QObject *parent = nullptr);
    ~CG_Database();

    bool databaseExists();
    void setToAThread(QThread* thread);

signals:
    void databaseLoadComplete();
    void userVerificationComplete(QWebSocket* socket, bool verified, QString meta,CG_User user);
    void foundUser(QString name, bool found);
    void addUserReply(QWebSocket* socket, bool added, int reason);
    void userRankingUpdated(QWebSocket* socket, int ranking);
    void userDataRefreshed(QWebSocket * socket, QString meta, CG_User user);

public slots:
    bool addUser(QWebSocket* socket, QString str_username, QByteArray pass, QString str_email, QString cg_data);
    bool userExists(QString str_username);
    bool setUserData(QWebSocket * socket, QString name, QByteArray pass, QString data);
    bool updateUserRanking(QWebSocket* socket,QString name, int rank);
    void userRankings(QWebSocket * socket, QString name);
    void verifyUserCredentials(QWebSocket* socket, QString name, QByteArray hpass);
    void updateLastGame(int id, int elo_change, quint64 secs_date, QString game_data);

protected:
    QSqlDatabase  m_dbUser; // users and profiles
    CG_UserGraph  m_graphUser;
    QString       m_UserDBPath;

    bool connectToDatabase();
    void clearUserDatabase();
    bool createMatchTables();
    bool createUserDatabase();
    bool createUserTables();
    void initializeDatabase(QString path, QString user, QString password, int port);

    // actual methods
    bool puserExists(QString str_username);
    int  paddUser(QString str_username, QByteArray pass, QString str_email, QString cg_data);
    bool pemailExists(QString str_email);
    bool psetUserData(QString name, QByteArray hpass, QString data, CG_User &user);
    int puserRankings(QString name);
    bool pupdateUserRanking(QString name, QString &meta, CG_User &user, int rank);
    bool pverifyUserCredentials(QString name, QByteArray pass, QString &meta, CG_User &user);
    QString pfetchRecentGame(int player_id);
    void paddUserMatch(int id);

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
