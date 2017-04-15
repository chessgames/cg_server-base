#include "cg_database.h"
#include <QSqlQuery>

#include <QDebug>

#ifdef CG_TEST_ENABLED
#include <QtTest/QTest>
#endif


CG_Database::CG_Database(QObject *parent)
    : QObject(parent)
{

    m_dbUser = QSqlDatabase::addDatabase("QSQLITE");
    m_dbUser.setHostName("CG");
    m_dbUser.setDatabaseName("/srv/CG/user.sqlite");
    m_dbUser.setConnectOptions();
#ifdef CG_TEST_ENABLED
    Q_VERIFY(m_dbUser.open());
#else
    Q_ASSERT(m_dbUser.open());
#endif
    qDebug() << "User Database open.";
}

/**************************************************************
*	  Purpose:  To check whether or not the user exists in the
*               database.
*
*     Entry:  User has clicked the register button
*
*     Exit:  Returns whether or not the user exists.
****************************************************************/
bool CG_Database::userExists(QString str_username)
{
    QSqlQuery qry( m_dbUser );
    qry.prepare( "SELECT * FROM CG_user WHERE Username= ?;" );
    qry.addBindValue(str_username);
    int username_count(0);
    if(qry.exec())
        for (; qry.next(); username_count++);
    return username_count > 0;
}


/**************************************************************
*	  Purpose:  To add a user into the database with the passed
*               parameters: username, password and email.
*
*     Entry:  User has clicked the register button
*
*     Exit:  Returns whether or not the user was successfully
*            added into the database.
****************************************************************/

bool CG_Database::addUser(QString str_username, QString str_password, QString str_email)
{

    if(userExists(str_username)){
        return false; // cannot add user that already exists
    }

    bool added_user(false);
    int user_id(-1); // should be set to an invalid id

    QSqlQuery qry( m_dbUser);
    qry.prepare( "INSERT INTO CG_user (Username, Passwd, Email) VALUES(?, ?, ?);" );
    qry.addBindValue(str_username);
    qry.addBindValue(str_password);
    qry.addBindValue(str_email);

    if(qry.exec()){
        added_user = true;
        qry.prepare("SELECT UserID FROM CG_user WHERE Username = ?");
        qry.addBindValue(str_username);

        if(qry.exec()){
            for (int count = 0; qry.next(); count++){
                user_id = qry.value(0).toInt(); // could be zero
            }
        }
        if(user_id != -1){
            qry.prepare( "INSERT INTO CG_settings (sound, auto_promotion, arrows, boardCoordinates, pieceSet, UserID) VALUES (1,1,1,1,1,?);" );
            qry.addBindValue(user_id);
            added_user = qry.exec();
        }
        updateCurrentELO(str_username,0);
    }
    return added_user;
}

/**************************************************************
*	  Purpose:  Updates player elo
*
*     Entry:  sets to zero on create, and
*
*     Exit:  Returns true if player elo was set
****************************************************************/

bool CG_Database::updateCurrentELO(QString str_username, int elo)
{
    bool updateElo = false;
    QSqlQuery qry( m_dbUser );

    qry.prepare( "UPDATE CG_user SET CurrentELO = ? WHERE Username = ?;" );
    qry.addBindValue(elo);
    qry.addBindValue(str_username);

    if(qry.exec()){
        updateElo = true;
    }

    return updateElo;
}


/**************************************************************
*	  Purpose:  To retrieve the current ELO rating of a user.
*
*     Entry:  User is logged in.
*
*     Exit:  Returns the current ELO of a user.
****************************************************************/

QString CG_Database::getCurrentELO(QString str_username)
{
    QString result;
    QSqlQuery qry(m_dbUser );
    qry.prepare( "SELECT CurrentELO FROM CG_user WHERE Username= ?;" );
    qry.addBindValue(str_username);

    if(qry.exec()){
        for (int count = 0; qry.next(); count++){
            result = qry.value(0).toString();
        }
    }
    return result;
}


bool CG_Database::updateCountryFlag(QString str_username, int country_flag)
{
    bool updateCountry = false;
    QSqlQuery qry( m_dbUser );

    qry.prepare( "UPDATE CG_user SET userCountry = ? WHERE Username = ?;" );
    qry.addBindValue(country_flag);
    qry.addBindValue(str_username);

    if(qry.exec()){
        updateCountry = true;
    }
    return updateCountry;
}


int CG_Database::getCountryFlag(QString str_username)
{
    int result;
    QSqlQuery qry( m_dbUser );
    qry.prepare( "SELECT userCountry FROM CG_user WHERE Username= ?;" );
    qry.addBindValue(str_username);

    if(qry.exec()){
        for (int count = 0; qry.next(); count++){
            result = qry.value(0).toInt();
        }
    }
    return result;
}


/**************************************************************
*	  Purpose:  To encrypt a password using the SHA256 hashing
*               function.
*
*     Entry:  User is attempting to login or register.
*
*     Exit:  Alters the password string based upon the SHA256
*            encryption.
****************************************************************/
//TODO: This should have been done on client side
void CG_Database::encryptPassword(QString &password)
{
    // do nothing, but dont break anything

}

//void CG_Database::encryptPassword(QString & password)
//{
//    // Takes the text in the le_password and converts it to Utf8, so it can be placed in a type of 'const char *' next
//    QByteArray passwordInBytes = password.toUtf8();

//    // casting the data in passwordInBytes (currently in the form of 'Utf8' to a type of 'const char *'
//    const char * convertedPasswordToVerify = passwordInBytes.constData();

//            /* Instantiating an object that will create the hashing key for the password. Takes an argument specifying
//             * the encryption type you would like be executed on the string */
//    QCryptographicHash sha256PasswordEncryptionGenerator( QCryptographicHash::Sha256 );

//    // Adding the data to password encryption generator
//    sha256PasswordEncryptionGenerator.addData( convertedPasswordToVerify );

//    // Converting the password to the hash key using the result method and placing it in password.
//    password = (QString)sha256PasswordEncryptionGenerator.result();
//}



CG_User CG_Database::getUserInfo(QString str_username)
{

    CG_User user_info;
    if(!userExists(str_username)){
        return user_info;
    }
    QSqlQuery qry( m_dbUser);
    qry.prepare( "SELECT LoggedIn, Banned, Username, CurrentELO, \
                 userCountry, pieceSet, userLanguage, \
                 Sound, boardCoordinates, arrows, auto_promotion, BoardTheme \
                 FROM CG_user JOIN CG_settings ON CG_user.UserID \
                 = CG_settings.UserID WHERE Username= ?;" );
            qry.addBindValue(str_username);

    qDebug() << "Username being accessed by database: " << str_username;

    if(qry.exec())
    {
        user_info.loggedIn = qry.value(0).toBool();
        qDebug() << "UserInfo Logged in bit: " << user_info.loggedIn;

        user_info.banned = qry.value(1).toBool();
        qDebug() << "UserInfo Banned bit: " << user_info.banned;

        user_info.username = qry.value(2).toString();
        user_info.elo = qry.value(3).toInt();
        qDebug() << "Username from database: " << user_info.username << " ELO: " << user_info.elo;

        user_info.countryFlag = qry.value(4).toInt();
        user_info.pieceSet = qry.value(5).toInt();
        user_info.language = qry.value(6).toInt();
        user_info.sound = qry.value(7).toBool();
        user_info.coordinates = qry.value(8).toBool();
        user_info.arrows = qry.value(9).toBool();
        user_info.autoPromote = qry.value(10).toBool();
        user_info.boardTheme = qry.value(11).toString();

    }
    return user_info;
}

bool CG_Database::updateUserInfo(CG_User user)
{
    if(!userExists(user.username)){
        return false;
    }


    bool update = false;
    QSqlQuery qry(m_dbUser);

    qry.prepare( "UPDATE CG_user SET LoggedIn = ?, Banned = ?, CurrentELO = ?, userCountry = ?, userLanguage = ?, BoardTheme = ? WHERE Username = ?;" );
    qry.addBindValue(user.loggedIn);
    qry.addBindValue(user.banned);
    qry.addBindValue(user.elo);
    qry.addBindValue(user.countryFlag);
    qry.addBindValue(user.language);
    qry.addBindValue(user.boardTheme);
    qry.addBindValue(user.username);

    if(!qry.exec()){
        qDebug() << "Failed to update the user info: " << user.username;
        return false;
    }

    qry.prepare( "SELECT UserID \
                 FROM CG_user \
                 WHERE Username = ?;");
    qry.addBindValue(user.username);

    int userID = 0;
    if (qry.exec()){
        for (int count = 0; qry.next(); count++){
            userID = qry.value(0).toInt();
        }

        update = true;
    }
    qry.prepare( "UPDATE CG_settings \
                 SET sound = ?, auto_promotion = ?, \
                 arrows = ?, boardCoordinates = ?, \
                 pieceSet = ? \
                WHERE UserID = ?;");

    qry.addBindValue(user.sound);
    qry.addBindValue(user.autoPromote);
    qry.addBindValue(user.arrows);
    qry.addBindValue(user.coordinates);
    qry.addBindValue(user.pieceSet);
    qry.addBindValue(userID);
    update = qry.exec();

    return update;
}


CG_Database::~CG_Database()
{
    if(m_dbUser.isOpen()){
        m_dbUser.close();
    }
}
