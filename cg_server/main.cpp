#include <QCoreApplication>
#include "cg_server.h"
#include "cg_global.h"
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
    QCoreApplication a(argc, argv);
    a.setAttribute(Qt::AA_Use96Dpi, true);

    handleUnixSignals({SIGABRT,SIGINT,SIGQUIT,SIGTERM });
#ifdef USE_SQLITE
    CG_Server server("/srv/CG/user.sqlite",&a);
#else
    CG_Server server("localhost","root","Sup@FlyChess2017",9654,&a);
#endif

#ifdef CG_TEST_ENABLED
    QTEST_SET_MAIN_SOURCE_PATH
    QTest::qExec(&server, argc, argv);

#else
    server.startToListen(QHostAddress("127.0.0.1"),5452);
#endif
    return a.exec();
}

#else
int main(int argc, char *argv[])
{
    qRegisterMetaType<CG_User>("CG_User");
    QCoreApplication a(argc, argv);
#ifdef USE_SQLITE
    CG_Server server(a.applicationDirPath() + "/user.sqlite",&a);
#endif
    server.startToListen(QHostAddress("127.0.0.1"),5452);
    return a.exec();
}
#endif
