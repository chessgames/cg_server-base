#ifndef CG_LOBBYMANAGER_H
#define CG_LOBBYMANAGER_H

#include <QObject>

class CG_LobbyManager : public QObject
{
    Q_OBJECT
public:
    explicit CG_LobbyManager(QObject *parent = nullptr);
    ~CG_LobbyManager();
signals:

public slots:

protected:

};

#endif // CG_LOBBYMANAGER_H
