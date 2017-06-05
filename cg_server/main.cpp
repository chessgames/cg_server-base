#include <QCoreApplication>
#include "cg_server.h"
#include "cg_global.h"
#include "cg_player.h"
#include <QNetworkConfiguration>
#include <QNetworkConfigurationManager>
Q_DECLARE_METATYPE(CG_User)

#ifdef CG_TEST_ENABLED
#include <QtTest/QTest>
#endif



#ifdef Q_OS_LINUX
#include"signal.h"
#include"unistd.h"
static void kill_server(int sig)
{
        printf("\nquit the application (user request signal = %d).\n", sig);
        QCoreApplication::exit();
}



void handleUnixSignals(const std::vector<int>& quitSignals,
                      const std::vector<int>& ignoreSignals = std::vector<int>()) {


    // all these signals will be ignored.
    for ( int sig : ignoreSignals )
        signal(sig, SIG_IGN);

    // each of these signals calls the handler (quits the QCoreApplication).
    for ( int sig : quitSignals )
        signal(sig, kill_server);
}


int main(int argc, char *argv[])
{
    qRegisterMetaType<CG_User>("CG_User");
    qRegisterMetaType<CG_Player>("CG_Player");
    QCoreApplication a(argc, argv);
    a.setAttribute(Qt::AA_Use96Dpi, true);
    QNetworkConfigurationManager mgr;
    QNetworkConfiguration ap = mgr.defaultConfiguration();

    handleUnixSignals({SIGABRT,SIGINT,SIGQUIT,SIGTERM });
#ifdef USE_SQLITE
    CG_Server server(a.applicationDirPath() + "user.sqlite",&ap,&a);
#else
    CG_Server server("localhost","root","Sup@FlyChess2017",9654,&ap,&a);
#endif

#ifdef CG_TEST_ENABLED
    QTEST_SET_MAIN_SOURCE_PATH
    QTest::qExec(&server, argc, argv);

#else
    server.startToListen(QHostAddress(QHostAddress::Any),5445);
#endif
    return a.exec();
}

#else
int main(int argc, char *argv[])
{
    qRegisterMetaType<CG_User>("CG_User");
    qRegisterMetaType<CG_Player>("CG_Player");
    QCoreApplication a(argc, argv);
#ifdef USE_SQLITE
    CG_Server server(a.applicationDirPath() + "/user.sqlite",&a);
#endif
    server.startToListen(QHostAddress("127.0.0.1"),5445);
    return a.exec();
}
#endif
