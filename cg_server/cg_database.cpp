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
#include <QCryptographicHash>


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
void CG_Database::testAddUser_data()
{
    QTest::addColumn<QString>("str_username");
    QTest::addColumn<QString>("pass");
    QTest::addColumn<QString>("str_email");

    QTest::newRow("StarWars") <<  "StarWars" <<"sw1" << "sw@stu.com";
    QTest::newRow("Tpimp")     <<  "Tpimp" << "pass" << "tpimp@tester.com";
    QTest::newRow("test user") <<  "Test" << "test" << "test@user.com";
}

void CG_Database::testAddUser()
{

    QFETCH(QString, str_username);
    QFETCH(QString, pass);
    QFETCH(QString, str_email);
    if(userExists(str_username)){
        return; // cannot add user that already exists
    }

    QSqlQuery qry( m_dbUser);
    // create settings object
    //updateCurrentELO(str_username,0);
    CG_User user;
    user.isValid = true;
    user.username = str_username;
    QCryptographicHash hasher(QCryptographicHash::Sha256);
    hasher.addData(pass.toLocal8Bit());
    QByteArray hpass = hasher.result().toHex();
    qry.prepare( "INSERT INTO cg_user (name, pass, email, data) VALUES(?, ?, ?, ?);" );
    qry.addBindValue(str_username);
    qry.addBindValue(hpass);
    qry.addBindValue(str_email);
    qry.addBindValue(serializeUser(user));

    QVERIFY(qry.exec());


    qry.prepare("SELECT * FROM cg_user WHERE name LIKE ?");
    qry.addBindValue(str_username);
    QVERIFY(qry.exec());
    QSqlError err = qry.lastError();
    if(err.isValid()){
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
    QFETCH(QString, pass);
    QFETCH(bool, result);
    qDebug() << "Testing User Verify " << username << " /w " << pass << result;
    CG_User user;
    QCryptographicHash hasher(QCryptographicHash::Sha256);
    hasher.addData(pass.toLocal8Bit());
    QByteArray hpass = hasher.result().toHex();
    bool actual_result = pverifyUserCredentials(username,hpass,user);
    // should be comparing an empty string
    if(!result){
        QEXPECT_FAIL(username.toLocal8Bit().data() ,"User Didnt exist",Continue);
    }
    QCOMPARE(actual_result,true);

}

void CG_Database::testUserVerify_data()
{
    QTest::addColumn<QString>("username");
    QTest::addColumn<QString>("pass");
    QTest::addColumn<bool>("result");

    QTest::newRow("Stu") << "Stu" << "afasf323fa" << false;
    QTest::newRow("StarWars") <<  "StarWars" << "sw1" << true;
    QTest::newRow("Chris") << "Chris" << "fads" << false;
    QTest::newRow("Tpimp")     <<  "Tpimp" << "pass" << true;
    QTest::newRow("test user") <<  "Test" << "test" << true;
}

#endif
/**************************************************************
*	  Purpose:  Updates player elo
*
*     Entry:  sets to zero on create, and
*
*     Exit:  Returns true if player elo was set
****************************************************************/



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
    QJsonObject obj;
    obj[CG_UN] = user.username;
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

    return out;
}



CG_Database::~CG_Database()
{
    if(m_dbUser.isOpen()){
        m_dbUser.close();
    }
}
