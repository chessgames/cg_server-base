#include "cg_database.h"
#include <QSqlQuery>
#include <QDir>
#include <QDebug>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlRecord>
#ifdef CG_TEST_ENABLED
#include <QtTest/QTest>
#endif


CG_Database::CG_Database(QString user_db_path, QObject *parent)
    : QObject(parent), m_UserDBPath(user_db_path)
{
    m_dbUser = QSqlDatabase::addDatabase("QSQLITE");
    m_dbUser.setHostName("CG");
    if(!databaseExists(user_db_path)){
        #ifndef CG_TEST_ENABLED
        createUserDatabase();
        #endif
    }
    else{
        m_dbUser.setDatabaseName(user_db_path);
        // test db exists
#ifdef CG_TEST_ENABLED
        QVERIFY(m_dbUser.isValid());
#else
        Q_ASSERT(m_dbUser.isValid());
        m_dbUser.setConnectOptions();
        // test db connection will open
    #ifdef CG_TEST_ENABLED
        QVERIFY(m_dbUser.open());
    #else
        Q_ASSERT(m_dbUser.open());
    #endif
#endif
    }

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
void CG_Database::userExists(QString str_username, bool &found)
{
    QSqlQuery qry( m_dbUser );
    qry.prepare( "SELECT * FROM CG_user WHERE Username= ?;" );
    qry.addBindValue(str_username);
    qry.exec();
    QSqlError err = qry.lastError();
#ifdef CG_TEST_ENABLED
    QCOMPARE(err.isValid(),false);
#else
    Q_ASSERT(!qry.lastError().isValid());
#endif
    found = !err.isValid();
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
#ifndef CG_TEST_ENABLED
void CG_Database::addUser(QString str_username, QByteArray pass, QString str_email, bool &error)
{
    userExists(str_username,error);
    if(error){
        return; // cannot add user that already exists
    }

    bool added_user(false);
    int user_id(-1); // should be set to an invalid id

    QSqlQuery qry( m_dbUser);
    // create settings object
    updateCurrentELO(str_username,0);
    QString data = "";// setting_object.toJson();
    qry.prepare( "INSERT INTO CG_User (name, pass, email, data) VALUES(?, ?, ?,?);" );
    qry.addBindValue(str_username);
    qry.addBindValue(pass);
    qry.addBindValue(str_email);
    qry.addBindValue(data);

#ifdef CG_TEST_ENABLED
    QVERIFY(qry.exec());
#endif
    added_user = true;
    qry.prepare("SELECT id FROM CG_User WHERE name = ?");
    qry.addBindValue(str_username);
#ifdef CG_TEST_ENABLED
    QVERIFY(qry.exec());
#endif
    QSqlError err = qry.lastError();
    if(err.isValid()){
        error = false;
        return;
    }
    else
    {
        user_id = qry.value(0).toInt();
        qDebug() << "Bad User Id found in DB @ "<< user_id <<" for " << str_username;
    }
}

#else
void CG_Database::addUser_data()
{
    QTest::addColumn<QString>("str_username");
    QTest::addColumn<QString>("pass");
    QTest::addColumn<QString>("str_email");

    QTest::newRow("StarWars") << "StarWas" << "starwars1" << "sw@stu.com";
    QTest::newRow("Tpimp")     << "Tpimp" << "pass" << "tpimp@tester.com";
    QTest::newRow("test user") << "Test" << "test" << "test@user.com";
}

void CG_Database::addUser()
{

    QFETCH(QString, str_username);
    QFETCH(QString, pass);
    QFETCH(QString, str_email);
    bool error(false);
    //userExists(str_username,error);
    if(error){
        return; // cannot add user that already exists
    }

    bool added_user(false);
    int user_id(-1); // should be set to an invalid id

    QSqlQuery qry( m_dbUser);
    // create settings object
    updateCurrentELO(str_username,0);
    QString data = "";// setting_object.toJson();
    qry.prepare( "INSERT INTO CG_User (name, pass, email, data) VALUES(?, ?, ?,?);" );
    qry.addBindValue(str_username);
    qry.addBindValue(pass);
    qry.addBindValue(str_email);
    qry.addBindValue(data);

#ifdef CG_TEST_ENABLED
    QVERIFY(qry.exec());
#endif
    added_user = true;
    qry.prepare("SELECT * FROM CG_User WHERE name = ?");
    qry.addBindValue(str_username);
#ifdef CG_TEST_ENABLED
    QVERIFY(qry.exec());
#endif
    QSqlError err = qry.lastError();
    if(err.isValid()){
        error = false;
        return;
    }
    else
    {
        QSqlRecord record = qry.record();
        qDebug() << "Bad User Id found in DB @ "<< record.value(0).toInt() <<" for " << record.value(1).toString();
        qDebug() << record;
    }
}
#endif
void CG_Database::createUserDatabase()
{
    QFileInfo f(m_UserDBPath);
    QString path_to_file = f.absolutePath();
    QFileInfo fdir(path_to_file);
    if( fdir.isDir() && fdir.isWritable()){
        m_dbUser.setDatabaseName(m_UserDBPath);
        #ifdef CG_TEST_ENABLED
            QVERIFY(m_dbUser.isValid());
        #else
            Q_ASSERT(m_dbUser.isValid());
        #endif
        m_dbUser.setConnectOptions();
        // test db connection will open
        #ifdef CG_TEST_ENABLED
            QVERIFY(m_dbUser.open());
        #else
            Q_ASSERT(m_dbUser.open());
        #endif
    #ifndef CG_TEST_ENABLED
            createUserTables();
    #endif
    }
}

void CG_Database::createUserTables()
{

    QSqlQuery create_user_tbl(m_dbUser);
    create_user_tbl.prepare("CREATE TABLE CG_User (id INT PRIMARY KEY NOT NULL, name TEXT, pass BLOB , email TEXT, data TEXT);");
    create_user_tbl.exec();
    QSqlError err = m_dbUser.lastError();
#ifdef CG_TEST_ENABLED
    QVERIFY(!err.isValid());
#else
    Q_ASSERT(!err.isValid());
#endif
}


void CG_Database::createMatchTables()
{

    QSqlQuery create_match_tbl("CREATE TABLE CG_User(id INT PRIMARY KEY NOT NULL, white INT, black INT , date TEXT, data TEXT);");
    create_match_tbl.exec();
    QSqlError err = m_dbUser.lastError();
#ifdef CG_TEST_ENABLED
    QVERIFY(!err.isValid());
#else
    Q_ASSERT(!err.isValid());
#endif
}

bool CG_Database::databaseExists(QString path)
{
    QDir file(path);
    return file.exists();
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
    bool error(false);
    userExists(str_username,error);
    if(error){
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
    bool error(false);
    userExists(user.username,error);
    if(error){
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
