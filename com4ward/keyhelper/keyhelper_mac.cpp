/****************************************************************************
**
** Copyright© 2014-2015 WUHAN PHONEYOU INFORMATICS CO,.LTD.
** Web-site: http://www.phoneyou.net/
** E-Mail:phoneyou@phoneyou.net
** This file is part of Roshan Framework.
**
****************************************************************************/
#include "keyhelper.h"
#include <QProcess>

bool KeyHelper::getRawMachineId(QString* sid_string, int* volume_id)
{
    QProcess proc;

    QStringList args;
    args << QLatin1String("-c") << QLatin1String("ioreg -rd1 -c IOPlatformExpertDevice |  awk '/IOPlatformUUID/ { print $3; }'");
    proc.start(QLatin1String("/bin/bash"), args );

    proc.waitForFinished();

    if(sid_string)*sid_string = QString::fromLatin1(proc.readAll());

    // Get the system drive volume serial number.
    *volume_id = 0;
    return true;
}
