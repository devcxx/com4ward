/****************************************************************************
**
** Copyright© 2014-2015 WUHAN PHONEYOU INFORMATICS CO,.LTD.
** Web-site: http://www.phoneyou.net/
** E-Mail:phoneyou@phoneyou.net
** This file is part of Roshan Framework.
**
****************************************************************************/
#include "keyhelper.h"
#include <QFile>
#include <QProcess>
#include <QUuid>

bool isUuid(const QString &id)
{
    QUuid uid(id); //make sure this can be made into a valid QUUid
    return !uid.isNull();
}

bool KeyHelper::getRawMachineId(QString* sid_string, int* volume_id)
{
    QString uniqueDeviceIDBuffer;
    if (uniqueDeviceIDBuffer.isEmpty()) {
        // for dmi enabled kernels
        QFile file(QStringLiteral("/sys/devices/virtual/dmi/id/product_uuid"));
        if (file.open(QIODevice::ReadOnly)) {
            QString id = QString::fromLocal8Bit(file.readAll().simplified().data());
            if (id.length() == 36) {
                if (isUuid(id)) {
                    uniqueDeviceIDBuffer = id;
                }
            }
        }
    }

    if (uniqueDeviceIDBuffer.isEmpty()) {
        QFile file(QStringLiteral("/etc/unique-id"));
        if (file.open(QIODevice::ReadOnly)) {
            QString id = QString::fromLocal8Bit(file.readAll().simplified().data());
            if (id.length() == 32) {
                id = id.insert(8,QLatin1Char('-')).insert(13,QLatin1Char('-')).insert(18,QLatin1Char('-')).insert(23,QLatin1Char('-'));
                if (isUuid(id)) {
                    uniqueDeviceIDBuffer = id;
                }
                file.close();
            }
        }
    }

//    if (uniqueDeviceIDBuffer.isEmpty()) { //try wifi mac address
//        QNetworkInfo netinfo;
//        QString macaddy;
//        macaddy = netinfo.macAddress(QNetworkInfo::WlanMode,0);
//        if (macaddy.isEmpty())
//            macaddy = netinfo.macAddress(QNetworkInfo::EthernetMode,0);
//        if (!macaddy.isEmpty()) {
//            QCryptographicHash hash2(QCryptographicHash::Sha1);
//            hash2.addData(macaddy.toLocal8Bit());

//            QUuid id = QUuid::fromRfc4122(hash2.result().left(16));
//            if (!id.isNull())
//                uniqueDeviceIDBuffer = id.toString();
//        }
//    }
    if (uniqueDeviceIDBuffer.isEmpty()) {
        QFile file(QStringLiteral("/etc/machine-id"));
        if (file.open(QIODevice::ReadOnly)) {
            QString id = QString::fromLocal8Bit(file.readAll().simplified().data());
            if (id.length() == 32) {
                id = id.insert(8,QLatin1Char('-')).insert(13,QLatin1Char('-')).insert(18,QLatin1Char('-')).insert(23,QLatin1Char('-'));
                if (isUuid(id)) {
                    uniqueDeviceIDBuffer = id;
                }
            }
            file.close();
        }
    }

    //last ditch effort
    if (uniqueDeviceIDBuffer.isEmpty()) {
        QFile file(QStringLiteral("/var/lib/dbus/machine-id"));

        if (file.open(QIODevice::ReadOnly)) {
            QString id = QString::fromLocal8Bit(file.readAll().simplified().data());
            if (id.length() == 32) {
                id = id.insert(8,QLatin1Char('-')).insert(13,QLatin1Char('-')).insert(18,QLatin1Char('-')).insert(23,QLatin1Char('-'));
                if (isUuid(id)) {
                    uniqueDeviceIDBuffer = id;
                }
            }
            file.close();
        }
    }

    if(sid_string)*sid_string = uniqueDeviceIDBuffer;
    *volume_id = 0;
    return uniqueDeviceIDBuffer.isEmpty();
}

