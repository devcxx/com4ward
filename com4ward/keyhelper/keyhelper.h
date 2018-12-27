/****************************************************************************
**
** Copyright© 2014-2015 WUHAN PHONEYOU INFORMATICS CO,.LTD.
** Web-site: http://www.phoneyou.net/
** E-Mail:phoneyou@phoneyou.net
** This file is part of Roshan Framework.
**
****************************************************************************/
#ifndef KEYHELPER_H
#define KEYHELPER_H

#include "keytemplate.h"

class KeyHelper
{
public:
    static bool getMachineId(QByteArray *_id);
    static QByteArray encode(const KeyTemplate& _keyTemplate);
    static void decode(KeyTemplate& _keyTemplate,const QByteArray& _byteArray);
private:

    static bool getRawMachineId(QString* sid_string, int* volume_id);
};

#endif // KEYHELPER_H
