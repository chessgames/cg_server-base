#include <QCoreApplication>
#include "cg_server.h"


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    CG_Server server(&a);
    return a.exec();
}
