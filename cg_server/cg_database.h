#ifndef CG_DATABASE_H
#define CG_DATABASE_H

#include <QObject>
#include "cg_global.h"
#include <QSqlDatabase>
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

    explicit CG_Database(QObject *parent = nullptr);
    ~CG_Database();

    bool userExists(QString str_username);
    bool correctUserInfo(QString str_username, QString str_password);
    bool addUser(QString str_username, QString str_password, QString str_email);
    bool updateCountryFlag(QString str_username, int country_flag);
    bool updateUserInfo(CG_User user);
    int getCountryFlag(QString str_username);
    void encryptPassword(QString & password);

    bool updateCurrentELO(QString str_username, int elo);
    QString getCurrentELO(QString str_username);
    CG_User getUserInfo(QString str_username);


protected:
    QSqlDatabase  m_dbUser; // users and profiles
    QSqlDatabase  m_dbGames; // past games
};

#endif // CG_DATABASE_H
