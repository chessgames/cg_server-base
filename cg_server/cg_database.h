#ifndef CG_DATABASE_H
#define CG_DATABASE_H

#include <QObject>
#include "cg_global.h"
#include <QSqlDatabase>
#include <QString>

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



class CG_Database : public QObject
{
    Q_OBJECT
public:

    explicit CG_Database(QString user_db_path,QObject *parent = nullptr);
    ~CG_Database();
    bool databaseExists(QString path);
    void userExists(QString str_username, bool &found);
    bool setUserCredentials(QString str_username, QByteArray new_pass, QByteArray old_pass);
    bool updateCountryFlag(QString str_username, int country_flag);
    bool updateUserInfo(CG_User user);
    int getCountryFlag(QString str_username);
    void encryptPassword(QString & password);

    bool updateCurrentELO(QString str_username, int elo);
    QString getCurrentELO(QString str_username);
    CG_User getUserInfo(QString str_username);

signals:
    void databaseLoadComplete();
    void databaseFinishedTransaction(QString id);

protected:
    QSqlDatabase  m_dbUser; // users and profiles
    QSqlDatabase  m_dbGames; // past games
    QString       m_UserDBPath;
    void createMatchTables();
    void clearUserDatabase();
#ifdef CG_TEST_ENABLED
private slots:
    void createUserDatabase();
    void createUserTables();
    void addUser();
    void addUser_data();
#else
public:
    void createUserDatabase();
    void createUserTables();
    void addUser(QString str_username, QByteArray pass, QString str_email, bool &error);

#endif

};

#endif // CG_DATABASE_H
