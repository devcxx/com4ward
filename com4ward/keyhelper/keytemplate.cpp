/****************************************************************************
**
** Copyright© 2014-2015 WUHAN PHONEYOU INFORMATICS CO,.LTD.
** Web-site: http://www.phoneyou.net/
** E-Mail:phoneyou@phoneyou.net
** This file is part of Roshan Framework.
**
****************************************************************************/
#include "keytemplate.h"
#include "keyhelper.h"

const static int CURRENT_KEY_VERSION = 1;
KeyTemplate::KeyTemplate()
    : m_version(CURRENT_KEY_VERSION)
{
}

KeyTemplate::KeyTemplate(const KeyTemplate& v)
{
    *this = v;
}

KeyTemplate& KeyTemplate::operator=(const KeyTemplate& v)
{
    m_expirationDate = v.m_expirationDate;
    m_registrationName = v.m_registrationName;
    m_machineID = v.m_machineID;
    return *this;
}

QString KeyTemplate::toString() const
{
    QString _res;
//    QDebug _dbg(&_res);
//    _dbg<<*this;
    return _res;
}

QString KeyTemplate::licenseInfo() const
{
    QString _res;
    if(!m_registrationName.isEmpty())
        _res.append(tr("Register to %1.\r\n").arg(m_registrationName));
    if(!m_machineID.isEmpty()){
        _res.append(tr("Local Device Certification.\r\n"));
    }else if(m_expirationDate.isValid()){
        _res.append(tr("Expiration Date is [%1].").arg(m_expirationDate.toString(Qt::ISODate)));
    }
    return _res;
}

void KeyTemplate::reset()
{
    m_expirationDate = QDate();
    m_registrationName = QString();
    m_machineID = QByteArray();
}

bool KeyTemplate::isValidate()const
{
    bool _res = true;
    if (m_version != CURRENT_KEY_VERSION) {
        _res = false;
        _updateErrorString(QString::fromLatin1("license version is wrong , current is [%1] but [%2] is wanted!").arg(m_version).arg(CURRENT_KEY_VERSION));
    }
    else if (!m_expirationDate.isNull() && m_expirationDate < QDate::currentDate()) {
        _res = false;
        _updateErrorString(QString::fromLatin1("expiration date is Over , last validate date is [%1] !").arg(m_expirationDate.toString()));
    }
    else if (!m_machineID.isEmpty()) {
        QByteArray _localMachineID;
        if (KeyHelper::getMachineId(&_localMachineID)) {
            if(_localMachineID != m_machineID){
                _res = false;
                _updateErrorString(QString::fromLatin1("machine id [%1]  is invalid!").arg(QString::fromLatin1(m_machineID.toBase64())));
            }
        }
    }
    return _res;
}

bool KeyTemplate::isNull() const
{
    return m_expirationDate.isNull() && m_machineID.isEmpty() && m_registrationName.isEmpty();
}

bool KeyTemplate::operator==(const KeyTemplate& v)
{
    return m_expirationDate == v.expirationDate() && m_machineID == v.machineID() && m_registrationName == v.registrationName();
}

void KeyTemplate::_updateErrorString(const QString& _msg)const
{
    m_errorString.clear();
    m_errorString = _msg;
//    qCCritical(licensingLog) << _msg;
    throw QString::fromLatin1("License Check Failed!");
}

QDataStream& operator>>(QDataStream& s, KeyTemplate& v)
{
    int _version = 0;
    QDate _expirationDate;
    QString _registrationName;
    QByteArray _machineID;
    s >> _version >> _expirationDate >> _registrationName >> _machineID;
    v.setVersion(_version);
    v.setExpirationDate(_expirationDate);
    v.setMachineID(QByteArray::fromBase64(_machineID));
    v.setRegistrationName(_registrationName);
    return s;
}

QDataStream& operator<<(QDataStream& s, const KeyTemplate& l)
{
    s << l.version() << l.expirationDate() << l.registrationName() << l.machineID().toBase64();
    return s;
}

QDebug operator<<(QDebug dbg , const KeyTemplate & v)
{
    dbg.nospace() << "KeyTemplate(";
    //
    dbg.space()  << "ExpirationDate:[" << v.expirationDate().toString() << ']';
    dbg.space()  << "RegistrationName:[" << v.registrationName() << ']';
    dbg.space()  << "MachineID:[" << v.machineID().toBase64() <<']';
    dbg.space() << ')';
    return dbg.space();
}

