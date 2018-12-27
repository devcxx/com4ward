#ifndef COM2TCPTHREAD_H
#define COM2TCPTHREAD_H

#include <QThread>
#include <QMutex>

#include "com2tcp/precomp.h"

class Com2TcpThread : public QThread {
    Q_OBJECT
public:
    ComParams& comParams() { return m_comParams; }
    QString& comPath() { return m_comPath; }
    QString& hostAddress() { return m_hostAddr; }
    QString& hostPort() { return m_hostPort; }

protected:
    virtual void run();
    ComParams m_comParams;
    QString m_comPath;
    QString m_hostAddr;
    QString m_hostPort;
};

#endif
