#include <QCoreApplication>
#include "cg_server.h"
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
    QCoreApplication a(argc, argv);
    handleUnixSignals({SIGABRT,SIGINT,SIGQUIT,SIGTERM });
    CG_Server server(&a);
    server.startToListen(QHostAddress::LocalHost,5452);
    return a.exec();
}
