#ifndef CG_USERGRAPH_H
#define CG_USERGRAPH_H

#include <QObject>

class cg_usergraph : public QObject
{
    Q_OBJECT
public:
    explicit cg_usergraph(QObject *parent = nullptr);

signals:

public slots:
};

#endif // CG_USERGRAPH_H