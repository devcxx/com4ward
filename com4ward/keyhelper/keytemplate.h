/****************************************************************************
**
** Copyright© 2014-2015 WUHAN PHONEYOU INFORMATICS CO,.LTD.
** Web-site: http://www.phoneyou.net/
** E-Mail:phoneyou@phoneyou.net
** This file is part of Roshan Framework.
**
****************************************************************************/
#ifndef KEYTEMPLATE_H
#define KEYTEMPLATE_H

#include <QDebug>
#include <QDate>
#include <QObject>
#include <QDataStream>
#include <QSharedPointer>

class KeyTemplate : public QObject {
    Q_OBJECT
    Q_PROPERTY(int version READ version WRITE setVersion)
    Q_PROPERTY(QDate expirationDate READ expirationDate WRITE setExpirationDate)
    Q_PROPERTY(QString registrationName READ registrationName WRITE setRegistrationName)
    Q_PROPERTY(QByteArray machineID READ machineID WRITE setMachineID)
public:
    KeyTemplate();
    KeyTemplate(const KeyTemplate& v);
    KeyTemplate& operator=(const KeyTemplate& v);

    QString toString()const;
    QString licenseInfo()const;
    void reset();
    bool isValidate()const;
    bool isNull()const;

    QDate expirationDate() const
    {
        return m_expirationDate;
    }
    QString registrationName() const
    {
        return m_registrationName;
    }

    QByteArray machineID() const
    {
        return m_machineID;
    }
public:
    bool operator==(const KeyTemplate& v);
    int version() const
    {
        return m_version;
    }

    QString errorString() const
    {
        return m_errorString;
    }

public slots:
    void setExpirationDate(QDate expirationDate)
    {
        m_expirationDate = expirationDate;
    }
    void setRegistrationName(QString registrationName)
    {
        m_registrationName = registrationName;
    }
    void setMachineID(QByteArray machineID)
    {
        m_machineID = machineID;
    }
    void setVersion(int version)
    {
        m_version = version;
    }
private:
    void _updateErrorString(const QString& _msg)const;
private:
    QDate m_expirationDate;
    int m_version;
    QString m_registrationName;
    QByteArray m_machineID;
    mutable QString m_errorString;
};

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug, const KeyTemplate&);

#endif

#ifndef QT_NO_DATASTREAM
QDataStream& operator<<(QDataStream& s, const KeyTemplate& l);
QDataStream& operator>>(QDataStream& s, KeyTemplate& v);
#endif

#endif // KEYTEMPLATE_H
