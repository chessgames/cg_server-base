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

/****************************************************************************************************
 * The section below defines the SQLITE FUNCTIONS
 ****************************************************************************************************
 ****************************************************************************************************/
#ifdef USE_SQLITE
bool CG_Database::createUserDatabase()
{
    QFileInfo f(m_UserDBPath);
    QString path_to_file = f.absolutePath();
    QFileInfo fdir(path_to_file);
    bool success(false);
    if( fdir.isDir() && fdir.isWritable()){
        m_dbUser.setDatabaseName(m_UserDBPath);
        m_dbUser.setConnectOptions();
        m_dbUser.open();
        success = createUserTables();
        success = createMatchTables();
        m_dbUser.close();
    }
    m_dbUser.open();
    return success;
}

bool CG_Database::createUserTables()
{
    QSqlQuery create_user_tbl(m_dbUser);
    create_user_tbl.prepare("CREATE TABLE cg_user (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, pass BINARY(32), email TEXT, elo INT, country TEXT, data TEXT);");
    return create_user_tbl.exec();
}


void CG_Database::initializeDatabase(QString path, QString user, QString password,int port)
{
    Q_UNUSED(path)
    Q_UNUSED(user)
    Q_UNUSED(password)
    Q_UNUSED(port)
    m_dbUser = QSqlDatabase::addDatabase("QSQLITE");
    m_dbUser.setHostName("CG");
    m_UserDBPath = path;
}

bool CG_Database::connectToDatabase()
{
    m_dbUser.setDatabaseName(m_UserDBPath);
    m_dbUser.setConnectOptions();
    return m_dbUser.open();
}

bool CG_Database::databaseExists()
{
    QFileInfo file(m_UserDBPath);
    return file.exists();
}

int CG_Database::paddUser(QString str_username, QByteArray pass, QString str_email, QString cg_data)
{

    if(puserExists(str_username)){
        return 1; // cannot add user that already exists
    }
    if(pemailExists(str_email)){
        return 2;
    }

    int user_id(-1); // should be set to an invalid id

    QSqlQuery qry( m_dbUser);
    // create settings object
    CG_User user;
    QString data =  CG_User::createUserData(user);
    qry.prepare( "INSERT INTO cg_user (name, pass, email, elo, country, data) VALUES(?, ?, ?, ?, ?, ?);" );
    qry.addBindValue(str_username);
    qry.addBindValue(pass);
    qry.addBindValue(str_email);
    int elo = 1000;
    qry.addBindValue(elo);
    QString country = "United States";
    qry.addBindValue(country);
    qry.addBindValue(data);
    if(qry.exec()){
        qry.prepare("SELECT id FROM cg_user WHERE name LIKE ?");
        qry.addBindValue(str_username);
        qry.exec();
        QSqlError err = qry.lastError();
        if(err.isValid()){
            return 4;
        }
        if(qry.next())
        {
            user_id = qry.value(0).toInt();
            qry.finish();
            paddUserMatch(user_id);
            qDebug() << "User Id found in DB @ "<< user_id <<" for " << str_username;
            return 0;
        }
    }
    return 3;
}

void CG_Database::paddUserMatch(int id)
{
    QSqlQuery qry( m_dbUser);
    qry.prepare( "INSERT INTO cg_match (id, total, won, date, last) VALUES(?, ?, ?, ?, ?);" );
    qry.addBindValue(id);
    qry.addBindValue(0);
    qry.addBindValue(0);
    qry.addBindValue(0);
    qry.addBindValue("");
    qry.exec();
}



/****************************************************************************************
 * Below is the Mysql implementations
 ****************************************************************************************
 ****************************************************************************************/
#else
/****************************************************************************************
 *Above is the SQLITE implementation
 ****************************************************************************************
 ****************************************************************************************/

bool CG_Database::createUserDatabase()
{
    QSqlQuery create_db(m_dbUser);
    m_dbUser.open();
    create_db.prepare("CREATE DATABASE IF NOT EXISTS CG_DB;");
    if(create_db.exec())
    {
        m_dbUser.close();
        m_dbUser.setDatabaseName("CG_DB");
        m_dbUser.open();
        return createUserTables();
    }
    return false;
}

bool CG_Database::createUserTables()
{
    QSqlQuery create_user_tbl(m_dbUser);
    create_user_tbl.prepare("CREATE TABLE cg_user (id BLOB, name TEXT, pass TINYBLOB , email TEXT, data TEXT, INDEX(id(1024)));");
    return create_user_tbl.exec();
}


void CG_Database::initializeDatabase(QString path, QString user, QString password,int port)
{
    m_dbUser = QSqlDatabase::addDatabase("QMYSQL");
    m_dbUser.setPort(port);
    m_dbUser.setHostName(path);
    m_dbUser.setUserName(user);
    m_dbUser.setPassword(password);
}


bool CG_Database::connectToDatabase()
{
    m_dbUser.setDatabaseName("CG_DB");
    return m_dbUser.open();
}

bool CG_Database::databaseExists()
{
    m_dbUser.open();
    QSqlQuery query;
    query.prepare("SELECT DATABASE CG_DB;");
    if(query.exec() && query.next());
    {
        return true;
    }
    m_dbUser.close();
    return false;
}

int CG_Database::paddUser(QString str_username, QByteArray pass, QString str_email, QString cg_data)
{
    if(puserExists(str_username)){
        return 1; // cannot add user that already exists
    }
    if(pemailExists(str_email)){
        return 2;
    }

    int user_id(-1); // should be set to an invalid id

    QSqlQuery qry( m_dbUser);
    // create settings object
    CG_User user;
    QString data =  serializeUser(user);
    QByteArray hasher_data;
    hasher_data.append(str_username.toLocal8Bit());
    hasher_data.append(":");
    hasher_data.append(str_email.toLocal8Bit());
    QByteArray id(QCryptographicHash::hash(hasher_data,QCryptographicHash::Sha3_256).toHex());
    qry.prepare( "INSERT INTO cg_user (id, name, pass, email, data) VALUES(?, ?, ?, ?, ?);" );
    qry.addBindValue(id);
    qry.addBindValue(str_username);
    qry.addBindValue(pass);
    qry.addBindValue(str_email);
    qry.addBindValue(data);
    if(!qry.exec()){
        return 3;
    }

    qry.prepare("SELECT id FROM cg_user WHERE name LIKE ?");
    qry.addBindValue(str_username);
    qry.exec();
    QSqlError err = qry.lastError();
    if(err.isValid()){
        return 4;
    }
    if(qry.next();
        user_id = qry.value(0).toInt();
        qDebug() << "User Id found in DB @ "<< user_id <<" for " << str_username;
    }
    return 0;
}

#endif
/****************************************************************************************
 * Below are functions not specific to which sqldriver but still must still consider
 *  TESTING vs RELEASE
 ****************************************************************************************
 ****************************************************************************************/
#ifndef CG_TEST_ENABLED  // Release




/**************************************************************
*	  Purpose:  Creates a new CG database object
****************************************************************/
CG_Database::CG_Database(QString db_path, QString user, QString password, int port, QObject *parent)
    : QObject(parent), m_UserDBPath(db_path)
{
    initializeDatabase(db_path,user,password,port);
    bool db_exists = databaseExists();
    if(!db_exists){
        Q_ASSERT(createUserDatabase());
    }
    else{
        Q_ASSERT(connectToDatabase());
    }
    if(m_dbUser.open()){
        qDebug() << "User Database open.";
    }
}

#endif


/*****************************************************************
 * Below is the functions declared for both situations
 * Often this means the actual implementation is abstracted
 * into a private method (overloaded). or the query is universal
 * sql.
 *****************************************************************/

bool CG_Database::userExists(QString str_username)
{
    return puserExists(str_username);
}


bool CG_Database::pemailExists(QString str_email)
{
    bool found(false);
    QSqlQuery qry( m_dbUser );
    qry.prepare( "SELECT * FROM cg_user WHERE email= ?;" );
    qry.addBindValue(str_email);
    if(qry.exec() &&
       qry.next()){
        found = true;
    }
    return found;
}


QString CG_Database::pfetchRecentGame(int player_id)
{
    QString data;
    QSqlQuery qry( m_dbUser );
    qry.prepare( "SELECT * FROM cg_match WHERE id=?;" );
    qry.addBindValue(player_id);
    if(qry.exec() && qry.next()){
        QSqlRecord record = qry.record();
        QJsonObject obj; // build out
        obj["total"] = record.value(1).toJsonValue();
        obj["won"] = record.value(2).toJsonValue();
        obj["date"] = record.value(3).toJsonValue();
        obj["snap"] = record.value(4).toJsonValue();
        QJsonDocument doc;
        doc.setObject(obj);
        data = doc.toJson();

    }
    return data;
}

bool CG_Database::puserExists(QString str_username)
{
    bool found(false);
    QSqlQuery qry( m_dbUser );
    qry.prepare( "SELECT * FROM cg_user WHERE name=?;" );
    qry.addBindValue(str_username);
    if(qry.exec() && qry.next()){
        found = true;

    }
    return found;
}

int CG_Database::puserRankings(QString name)
{
    int elo(0);
    QSqlQuery qry( m_dbUser );
    qry.prepare( "SELECT elo FROM cg_user WHERE name=?;" );
    qry.addBindValue(name);
    if(qry.exec() && qry.next())
    {
        elo = qry.record().value(0).toInt();
    }
    return elo;
}

bool CG_Database::pupdateUserRanking(QString name, QString &meta, CG_User & user, int rank)
{
    QSqlQuery qry( m_dbUser );
    qry.prepare( "UPDATE cg_user SET elo =? WHERE name=?;" );
    qry.addBindValue(rank);
    qry.addBindValue(name);
    if(qry.exec())
    {
        qry.clear();
        qry.prepare("SELECT * FROM cg_user WHERE name=?");
        qry.addBindValue(name);
        if(qry.exec() && qry.next()){
            QSqlRecord record = qry.record();
            int id = record.value(0).toInt();
            QString recent = pfetchRecentGame(id);
            QJsonArray array;
            QString data(record.value(6).toString());
            CG_User::fromData(user,data);
            user.id = id;
            user.username = record.value(1).toString();
            user.elo = record.value(4).toInt();
            user.countryFlag = record.value(5).toString();
            array.append(CG_User::serializeUser(user));
            array.append(recent);
            QJsonDocument doc;
            doc.setArray(array);
            meta = doc.toJson();
        }

        return true;
    }
    return false;
}

bool CG_Database::pverifyUserCredentials(QString name, QByteArray pass, QString & meta, CG_User &user)
{
    m_dbUser.open();
    bool verified(false);
    QSqlQuery qry(m_dbUser);
    qry.prepare("SELECT * FROM cg_user WHERE name=? AND pass=?;");
    qry.addBindValue(name);
    qry.addBindValue(pass);
    if(qry.exec() && qry.next()){
        QSqlRecord record = qry.record();
        int id = record.value(0).toInt();
        QString recent = pfetchRecentGame(id);
        QJsonArray array;
        QString data(record.value(6).toString());
        CG_User::fromData(user,data);
        user.id = id;
        user.username = record.value(1).toString();
        user.elo = record.value(4).toInt();
        user.countryFlag = record.value(5).toString();
        array.append(CG_User::serializeUser(user));
        array.append(recent);
        QJsonDocument doc;
        doc.setArray(array);
        meta = doc.toJson();
        verified = true;
    }
    return verified;
}


void CG_Database::updateLastGame(int id, int elo_change, quint64 secs_date, QString game_data)
{
    QString current = pfetchRecentGame(id);
    QJsonDocument doc = QJsonDocument::fromJson(current.toLocal8Bit());
    QJsonObject record = doc.object();
    int prev_won = record.value("won").toInt();
    int total = record.value("total").toInt();
    total += 1;
    if(elo_change > 0){
        prev_won += 1;
    }
    QSqlQuery qry( m_dbUser );
    qry.prepare( "UPDATE cg_match SET total=?,won=?,date=?,last=? WHERE id=?);");
    qry.addBindValue(total);
    qry.addBindValue(prev_won);
    qry.addBindValue(secs_date);
    qry.addBindValue(game_data);
    qry.addBindValue(id);
    qry.exec();
}




bool CG_Database::psetUserData(QString name, QByteArray hpass, QString data, CG_User & user)
{
    bool set_data(false);
    QSqlQuery qry(m_dbUser);
    CG_User::fromData(user,data);
    QString db_data = CG_User::createUserData(user);
    qry.prepare("UPDATE cg_user set country=?,data=? WHERE name = ? AND pass = ?;");
    qry.addBindValue(user.countryFlag);
    qry.addBindValue(db_data);
    qry.addBindValue(name);
    qry.addBindValue(hpass);
    if(qry.exec()){
        set_data = true;
    }
    return set_data;
}



bool CG_Database::createMatchTables()
{

    QSqlQuery create_match_tbl(m_dbUser);
    create_match_tbl.prepare("CREATE TABLE cg_match(id INT PRIMARY KEY NOT NULL, total INT, won INT, date INT, last TEXT);");
    return create_match_tbl.exec();
}



/**************************************************************
*	  Purpose:  To add a user into the database with the passed
*               parameters: username, password and email.
*
*     Entry:  User has clicked the register button or
*          CGWebApi wants to add a CG Member
*
*     Exit:  Returns whether or not the user was successfully
*            added into the database.
****************************************************************/

bool CG_Database::addUser(QWebSocket * socket, QString str_username, QByteArray pass, QString str_email, QString cg_data)
{
    int added(paddUser(str_username,pass,str_email,cg_data));
    emit addUserReply(socket,bool(added == 0), added);
    return (added == 0);
}


bool CG_Database::setUserData(QWebSocket *socket, QString name, QByteArray pass, QString data)
{
    CG_User user;
    QString meta;
    bool verified = pverifyUserCredentials(name,pass,meta,user);
    if(verified){
        verified = psetUserData(name,pass,data,user);
        emit userDataRefreshed(socket,meta,user);
    }
    return verified;
}



bool CG_Database::updateUserRanking(QWebSocket *socket, QString name, int rank)
{
    if(userExists(name)){
        QString meta;
        CG_User user;
        bool set = pupdateUserRanking(name,meta,user,rank);
        if(set){
            emit userRankingUpdated(socket,rank);
            emit userDataRefreshed(socket,meta,user);
        }
    }
}

void CG_Database::userRankings(QWebSocket *socket, QString name)
{
    if(userExists(name)){
        int rank = puserRankings(name);
        emit userRankingUpdated(socket,rank);
    }
}

void CG_Database::verifyUserCredentials(QWebSocket *socket, QString name, QByteArray hpass)
{
    CG_User user;
    QString meta;
    bool verified = pverifyUserCredentials(name, hpass, meta,user);
    emit userVerificationComplete(socket,verified,meta, user);
}


void CG_Database::setToAThread(QThread *thread)
{
    moveToThread(thread);
}



CG_Database::~CG_Database()
{
    if(m_dbUser.isOpen()){
        m_dbUser.close();
    }
}







/****************************************************************************************
 * Below is the Qt Auto TEST Functions
 ****************************************************************************************
 ****************************************************************************************/
#ifdef CG_TEST_ENABLED


/**************************************************************
*	  Purpose:  Creates a new CG database object
****************************************************************/
CG_Database::CG_Database(QString db_path, QString user, QString password, int port, QObject *parent)
    : QObject(parent), m_UserDBPath(db_path)
{
    initializeDatabase(db_path,user,password,port);
    if(!databaseExists()){
        QVERIFY(createUserDatabase());
    }
    else{
        QVERIFY(connectToDatabase());
    }
   qDebug() << "User Database open.";
}



void CG_Database::testAddUser_data()
{
    QTest::addColumn<QString>("username");
    QTest::addColumn<QString>("pass");
    QTest::addColumn<QString>("email");
    QTest::addColumn<QString>("cg_data");
    QTest::addColumn<int>("ret_val");   // Zero means success

    QTest::newRow("StarWars") <<  "StarWars" <<"sw1" << "sw@stu.com" << ""<< 0;
    QTest::newRow("Tpimp")     <<  "Tpimp" << "pass" << "tpimp@tester.com" << ""<< 0;
    QTest::newRow("duplicate-username")  <<  "Tpimp" << "pass" << "bob@tester.com" << ""<< 1;  // name already exists
    QTest::newRow("duplicate-email")  <<  "Jerry" << "pass" << "tpimp@tester.com" << ""<< 2; // email already exists
    QTest::newRow("test user") <<  "Test" << "test" << "test@user.com" << "" << 0;
}

void CG_Database::testAddUser()
{

    QFETCH(QString, username);
    QFETCH(QString, pass);
    QFETCH(QString, email);
    QFETCH(QString, cg_data);
    QFETCH(int, ret_val);
    QCryptographicHash hasher(QCryptographicHash::Sha256);
    hasher.addData(pass.toLocal8Bit());
    QByteArray hpass = hasher.result().toHex();
    int user_exists = paddUser(username,hpass,email,cg_data);
    if(user_exists > 0){ // failed some how
        if(ret_val)
        {
            QEXPECT_FAIL(username.toLocal8Bit().data() ,"User Already Exists",Continue);
        }
    }
    QCOMPARE(user_exists,ret_val);
}

void CG_Database::testUserSettingsUpdate_data()
{

    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("pass");
    QTest::addColumn<int>("pieceSet");
    QTest::addColumn<int>("language");
    QTest::addColumn<bool>("sound");
    QTest::addColumn<bool>("coordinates");
    QTest::addColumn<bool>("arrows");
    QTest::addColumn<bool>("autoPromote");
    QTest::addColumn<bool>("result");

    QTest::newRow("StarWars") <<  "StarWars" << "sw1" << 1 << 0 << false << true << false << true << true;
    QTest::newRow("Stu") << "Stu" << "afasf323fa" << 0 << 0  << true << true << true << true << false;
    QTest::newRow("test user") << "Test" << "test" << 0 << 1 << true << true << false << true << true;
    QTest::newRow("Tpimp")     <<  "Tpimp" << "pass" << 0 << 2 << false << true << false << true << true;
}



void CG_Database::testUserSettingsUpdate()
{
    QFETCH(QString, name);
    QFETCH(QString, pass);
    QFETCH(int, pieceSet);
    QFETCH(int, language);
    QFETCH(bool, sound);
    QFETCH(bool, coordinates);
    QFETCH(bool, arrows);
    QFETCH(bool, autoPromote);
    QFETCH(bool, result);



    QCryptographicHash hasher(QCryptographicHash::Sha256);
    hasher.addData(pass.toLocal8Bit());
    QByteArray hpass = hasher.result().toHex();
    CG_User user;
    bool verified = pverifyUserCredentials(name,hpass,user);
    QStringList changes;
    if(verified){
        if(user.arrows != arrows){
            user.arrows = arrows;
            changes.append("arrows");
        }
        if(user.sound != sound){
            user.sound = sound;
            changes.append("sound");
        }
        if(user.language != language){
            user.language = language;
            changes.append("language");
        }
        if(user.pieceSet != pieceSet){
            user.pieceSet = pieceSet;
            changes.append("pieceSet");
        }
        if(user.coordinates != coordinates){
            user.coordinates = coordinates;
            changes.append("coordinates");
        }
        if(user.autoPromote != autoPromote){
            user.autoPromote = autoPromote;
            changes.append("autoPromote");
        }
        verified = psetUserData(name,hpass,serializeUser(user));
        if(verified){
            CG_User test;
            verified = pverifyUserCredentials(name,hpass,test);
            if(verified){
                for(QString value : changes){
                    bool all_set(true);
                    switch(value.at(2).toLatin1()){
                        case 'r':{
                            if(test.arrows != arrows){
                                all_set = false;
                            }
                            break;
                        }
                        case 'u':{
                            if(test.sound != sound){
                                all_set = false;
                            }
                            break;
                        }
                        case 'n':{
                            if(test.language != language){
                                all_set = false;
                            }
                            break;
                        }
                        case 'e':{
                            if(test.pieceSet != pieceSet){
                                all_set = false;
                            }
                            break;
                        }
                        case 'o':{
                            if(test.coordinates != coordinates){
                                all_set = false;
                            }
                            break;
                        }
                        case 't':{
                            if(test.autoPromote != autoPromote){
                                all_set = false;
                            }
                            break;
                        }
                        default: break;
                    }
                    QVERIFY(all_set);
                    return;
                }
            }
            else{
                if(!result){
                    QEXPECT_FAIL(name.toLocal8Bit().data(),"Failed to fetch user data for test",Continue);
                }
            }
        }
        else{
            if(!result){
                QEXPECT_FAIL(name.toLocal8Bit().data(),"Failed to set user data",Continue);
            }
        }
    }
    else{
        if(!result){
            QEXPECT_FAIL(name.toLocal8Bit().data(),"User did not verify with system (or doesn't exist)",Continue);
        }
    }
    QVERIFY(result);

}



void CG_Database::testUserSetRank_data()
{
    QTest::addColumn<QString>("username");
    QTest::addColumn<int>("rank");
    QTest::addColumn<bool>("result");

    QTest::newRow("Stu") << "Stu" << 1234 << false;
    QTest::newRow("StarWars") <<  "StarWars" << 1645 << true;
    QTest::newRow("Test User") << "test" << 1450 << true;
    QTest::newRow("Tpimp") <<  "Tpimp" << 1110 << true;
}

void CG_Database::testUserSetRank()
{
    QFETCH(QString, username);
    QFETCH(int, rank);
    QFETCH(bool, result);

    if(pupdateUserRanking(username,rank)){
        int new_rank = puserRankings(username);
        QVERIFY(new_rank == rank);
    }
    else{
        if(!result){
            QEXPECT_FAIL(username.toLocal8Bit().data(),"Failed to update User ranking, User does not exist.",Continue);
        }
        QVERIFY(result);
    }

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
    QVERIFY(actual_result);


}




#endif
