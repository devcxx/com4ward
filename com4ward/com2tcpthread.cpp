#include "com2tcpthread.h"

extern int main_process(const ComParams& comParams, char* pPath, char* pAddr, char* pPort);

void Com2TcpThread::run()
{
    main_process(m_comParams, m_comPath.toLatin1().data(), m_hostAddr.toLatin1().data(), m_hostPort.toLatin1().data());
}
