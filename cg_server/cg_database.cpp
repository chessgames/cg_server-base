#include "cg_database.h"
#include <QSqlQuery>
#include <QDir>
#include <QDebug>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlResult>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QVariant>

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

bool CG_Database::userExists(QString str_username)
{
    return puserExists(str_username);
}

bool CG_Database::puserExists(QString str_username)
{
    bool found(false);
    QSqlQuery qry( m_dbUser );
    qry.prepare( "SELECT * FROM cg_user WHERE name= ?;" );
    qry.addBindValue(str_username);
    qry.exec();
    QSqlError err = qry.lastError();
    if(err.isValid()){
        found = false;
    }
    for(;qry.next();){

        QSqlRecord record = qry.record();
        int id = record.value(0).toInt();
        QString name = record.value(1).toString();
        if(id > 0 && (name.compare(str_username) == 0))
        {
            found = true;
        }
    }
    emit foundUser(str_username,found);
    return found;
}



bool CG_Database::databaseExists(QString path)
{
    QDir file(path);
    return file.exists();
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

bool CG_Database::addUser(QString str_username, QByteArray pass, QString str_email)
{
    return paddUser(str_username,pass,str_email);
}

bool CG_Database::paddUser(QString str_username, QByteArray pass, QString str_email)
{
    if(!puserExists(str_username)){
        return false; // cannot add user that already exists
    }

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


    qry.prepare("SELECT id FROM CG_User WHERE name = ?");
    qry.addBindValue(str_username);
    QSqlError err = qry.lastError();
    if(err.isValid()){
        return false;
    }
    else
    {
        user_id = qry.value(0).toInt();
        qDebug() << "Bad User Id found in DB @ "<< user_id <<" for " << str_username;
    }
    return true;
}

#ifndef CG_TEST_ENABLED


#else
void CG_Database::addUser_data()
{
    QTest::addColumn<QString>("str_username");
    QTest::addColumn<QByteArray>("pass");
    QTest::addColumn<QString>("str_email");

    QByteArray encoded_str;
    encoded_str = QString("sw1").toLatin1();
    QTest::newRow("StarWars") <<  "StarWars" << encoded_str << "sw@stu.com";
    encoded_str = QString("pass").toLatin1();
    QTest::newRow("Tpimp")     <<  "Tpimp" << encoded_str << "tpimp@tester.com";
    encoded_str = QString("test").toLatin1();
    QTest::newRow("test user") <<  "Test" << encoded_str << "test@user.com";
}

void CG_Database::addUser()
{

    QFETCH(QString, str_username);
    QFETCH(QByteArray, pass);
    QFETCH(QString, str_email);
    bool error(false);
    userExists(str_username,error);
    if(error){
        return; // cannot add user that already exists
    }

    bool added_user(false);

    QSqlQuery qry( m_dbUser);
    // create settings object
    //updateCurrentELO(str_username,0);
    QString data = "";// setting_object.toJson();
    qry.prepare( "INSERT INTO cg_user (name, pass, email, data) VALUES(?, ?, ?, ?);" );
    qry.addBindValue(str_username);
    qry.addBindValue(pass);
    qry.addBindValue(str_email);
    qry.addBindValue(data);

    QVERIFY(qry.exec());

    added_user = true;
    qry.prepare("SELECT * FROM cg_user WHERE name LIKE ?");
    qry.addBindValue(str_username);
    QVERIFY(qry.exec());
    QSqlError err = qry.lastError();
    if(err.isValid()){
        error = false;
        return;
    }
    else
    {
        int count(0);
        for(;qry.next();count++){
            QSqlRecord record = qry.record();
            qDebug() << record;
        }
    }
}


void CG_Database::testUserVerify()
{
    QFETCH(QString, username);
    QFETCH(QByteArray, pass);
    QFETCH(bool, result);
    qDebug() << "Testing User Verify " << username << " /w " << pass << result;
    bool actual_result = pverifyUserCredentials(username,pass);
    // should be comparing an empty string
    QVERIFY( actual_result == result);
    if((value != 0) && result){
        qDebug() << "Verify success: User Credentials";
        qDebug() << user;
    }
    else{
        qDebug() << "Verify Failed User doesn't Exist :"<< username << " " << pass;
    }

}

void CG_Database::testUserVerify_data()
{
    QTest::addColumn<QString>("username");
    QTest::addColumn<QByteArray>("pass");
    QTest::addColumn<bool>("result");
    QByteArray encoded_str;
    encoded_str = QString("afasf323fa").toLatin1();
    QTest::newRow("Stu") << "Stu" << encoded_str << false;
    encoded_str = QString("sw1").toLatin1();
    QTest::newRow("StarWars") <<  "StarWars" << encoded_str << true;
    encoded_str = QString("fads" ).toLatin1();
    QTest::newRow("Chris") << "Chris" << encoded_str << false;
    encoded_str = QString("pass").toLatin1();
    QTest::newRow("Tpimp")     <<  "Tpimp" << encoded_str << true;
    encoded_str = QString("test").toLatin1();
    QTest::newRow("test user") <<  "Test" << encoded_str << true;
}

#endif
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
    CG_User user;
    //int user_id = getUserInfo(user,str_username);
    if(user.isValid){
        user.elo = elo;
        //updateUserData(user, user_id);
    }
    return updateElo;
}


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
            createUserTables();
    }
}

void CG_Database::createUserTables()
{

    QSqlQuery create_user_tbl(m_dbUser);
    create_user_tbl.prepare("CREATE TABLE cg_user (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, pass BLOB , email TEXT, data TEXT);");
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

    QSqlQuery create_match_tbl("CREATE TABLE cg_match(id INT PRIMARY KEY NOT NULL, white INT, black INT , date TEXT, data TEXT);");
    create_match_tbl.exec();
    QSqlError err = m_dbUser.lastError();
#ifdef CG_TEST_ENABLED
    QVERIFY(!err.isValid());
#else
    Q_ASSERT(!err.isValid());
#endif
}




/**************************************************************
*	  Purpose:  To retrieve the current ELO rating of a user.
*
*     Entry:  User is logged in.
*
*     Exit:  Returns the current ELO of a user.
****************************************************************/

QString CG_Database::pgetCurrentELO(QString str_username)
{
    QString result;
    QSqlQuery qry(m_dbUser );
    qry.prepare( "SELECT CurrentELO FROM CG_user WHERE name= ?;" );
    qry.addBindValue(str_username);

    if(qry.exec()){
        for (int count = 0; qry.next(); count++){
            result = qry.value(0).toString();
        }
    }
    return result;
}

QString CG_Database::getCurrentELO(QString str_username)
{
    QString elo = pgetCurrentELO(str_username);
    emit gotElo(str_username,elo.toInt());
    return elo;
}



bool CG_Database::updateCountryFlag(QString str_username, int country_flag)
{
    bool updateCountry = false;
    QSqlQuery qry( m_dbUser );

    qry.prepare( "UPDATE CG_user SET userCountry = ? WHERE name = ?;" );
    qry.addBindValue(country_flag);
    qry.addBindValue(str_username);

    if(qry.exec()){
        updateCountry = true;
    }
    return updateCountry;
}

void CG_Database::verifyUserCredentials(QWebSocket *socket, QString name, QByteArray hpass)
{
    CG_User user;
    bool verified = pverifyUserCredentials(name, hpass, user);
    emit userVerificationComplete(socket,verified,user);

}

bool CG_Database::pverifyUserCredentials(QString name, QByteArray pass, CG_User & user){
    bool verified(false);
    QSqlQuery qry(m_dbUser);
    qry.prepare("SELECT * FROM cg_user WHERE name LIKE ? AND pass = ? ");
    qry.addBindValue(name);
    qry.addBindValue(pass);
    if(qry.exec()){
        if(qry.next()){
            QSqlRecord record = qry.record();
            QString user_data = record.value(4).toString();
            setUserStruct(user,name,user_data);
            verified = true;
        }
    }
    return verified;
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



void CG_Database::getUserInfo(CG_User& user,QString str_username)
{
    if(userExists(str_username)){
        return;
    }
    QSqlQuery qry( m_dbUser);
    qry.prepare( "SELECT data WHERE name LIKE ?;" );
    qry.addBindValue(str_username);


    if(qry.exec())
    {
        QVariant var = qry.value("data");
        user.loggedIn = qry.value(0).toBool();
        qDebug() << "UserInfo Logged in bit: " << user.loggedIn;

        user.banned = qry.value(1).toBool();
        qDebug() << "UserInfo Banned bit: " << user.banned;

        user.username = qry.value(2).toString();
        user.elo = qry.value(3).toInt();
        qDebug() << "Username from database: " << user.username << " ELO: " << user.elo;

        user.countryFlag = qry.value(4).toInt();
        user.pieceSet = qry.value(5).toInt();
        user.language = qry.value(6).toInt();
        user.sound = qry.value(7).toBool();
        user.coordinates = qry.value(8).toBool();
        user.arrows = qry.value(9).toBool();
        user.autoPromote = qry.value(10).toBool();
        user.boardTheme = qry.value(11).toString();

    }
}

bool CG_Database::updateUserInfo(CG_User user)
{
    if(userExists(user.username)){
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

void CG_Database::setToAThread(QThread *thread)
{
    moveToThread(thread);
}

void CG_Database::setUserStruct(CG_User &user, QString name, QString json_settings)
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
    user.countryFlag = obj.value(CG_CF).toInt();
    user.elo = obj.value(CG_E).toInt();
    user.language = obj.value(CG_LANG).toInt();
    user.isValid = true;
}

bool CG_Database::setUserData(QString name, QByteArray pass, QString data)
{
    CG_User user;
    bool data_set = pverifyUserCredentials(name,pass,user);
    if(data_set){
        data_set = psetUserData(name,data);
    }
    emit userDataSet(name,data_set);
    return data_set;
}


bool CG_Database::psetUserData(QString name, QString data)
{
    bool set_data(false);
    QSqlQuery qry(m_dbUser);
    qry.prepare("UPDATE cg_user set data = ? WHERE name LIKE ?;");
    qry.addBindValue(data);
    qry.addBindValue(name);
    if(qry.exec()){
        if(qry.next()){
            set_data = true;
        }
    }
    return set_data;
}

QString CG_Database::serializeUser(const CG_User &user)
{
    QString out;
    if(user.isValid){
        QJsonObject obj;
        obj[CG_AR] =user.arrows;
        obj[CG_AP] = user.autoPromote;
        obj[CG_BAN] = user.banned;
        obj[CG_BT] = user.boardTheme;
        obj[CG_BF] = int(user.cgbitfield);
        obj[CG_CO] = user.coordinates;
        obj[CG_CF] = user.countryFlag;
        obj[CG_E] = user.elo;
        obj[CG_LANG] = user.language;
        QJsonDocument doc;
        doc.setObject(obj);
        out = doc.toJson();
    }
    return out;
}



CG_Database::~CG_Database()
{
    if(m_dbUser.isOpen()){
        m_dbUser.close();
    }
}
