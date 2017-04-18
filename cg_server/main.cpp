#include <QCoreApplication>
#include "cg_server.h"
#include"signal.h"
#include"unistd.h"

#ifdef CG_TEST_ENABLED
#include <QtTest/QTest>
#endif

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

#ifdef CG_TEST_ENABLED
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    a.setAttribute(Qt::AA_Use96Dpi, true);
    handleUnixSignals({SIGABRT,SIGINT,SIGQUIT,SIGTERM });
    CG_Server server("/srv/CG/user.sqlite",&a);
    QTEST_SET_MAIN_SOURCE_PATH
    QTest::qExec(&server, argc, argv);
}
#else
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    handleUnixSignals({SIGABRT,SIGINT,SIGQUIT,SIGTERM });
    CG_Server server("/srv/CG/user.sqlite",&a);
    bool error(false);
    server.startToListen(QHostAddress("192.168.3.105"),5452,error);
    return a.exec();
}
#endif
