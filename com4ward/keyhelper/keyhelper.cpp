/****************************************************************************
**
** Copyright© 2014-2015 WUHAN PHONEYOU INFORMATICS CO,.LTD.
** Web-site: http://www.phoneyou.net/
** E-Mail:phoneyou@phoneyou.net
** This file is part of Roshan Framework.
**
****************************************************************************/
#include "keyhelper.h"
#include "simplecrypt.h"
#include <QCryptographicHash>

static const quint64 privateKey = 0xabcd;

QByteArray KeyHelper::encode(const KeyTemplate& _keyTemplate)
{
    QByteArray _rawData;
    QDataStream _stream(&_rawData, QIODevice::WriteOnly);
    _stream << _keyTemplate;
    SimpleCrypt _crypt(privateKey);
    return _crypt.encryptToByteArray(_rawData).toBase64();
}

void KeyHelper::decode(KeyTemplate& _keyTemplate, const QByteArray& _byteArray)
{
    SimpleCrypt _crypt(privateKey);
    QDataStream _stream(_crypt.decryptToByteArray(QByteArray::fromBase64(_byteArray)));
    _stream >> _keyTemplate;
}

bool GetMachineIdImpl(const QString& sid_string,
    int volume_id,
    QByteArray* machine_id)
{
    machine_id->clear();

    QByteArray id_binary;
    if (!sid_string.isEmpty()) {

        QCryptographicHash _hash(QCryptographicHash::Sha1);
        _hash.addData(sid_string.toLocal8Bit());
        id_binary = _hash.result();
    }

    // Convert from int to binary (makes big-endian).
    for (size_t i = 0; i < sizeof(int); i++) {
        int shift_bits = 8 * (sizeof(int) - i - 1);
        id_binary.append(static_cast<unsigned char>((volume_id >> shift_bits) & 0xFF));
    }

    // Append the checksum byte.
    if (!sid_string.isEmpty() || (0 != volume_id)){
        id_binary.append(qChecksum(id_binary.data(),id_binary.length()));
    }

    *machine_id = id_binary;
    return true;
}


bool KeyHelper::getMachineId(QByteArray* machine_id)
{
    if (!machine_id)
        return false;

    static QByteArray calculated_id;
    static bool calculated = false;
    if (calculated) {
        *machine_id = calculated_id;
        return true;
    }

    QString sid_string;
    int volume_id;
    if (!getRawMachineId(&sid_string, &volume_id))
        return false;

    if (!GetMachineIdImpl(sid_string, volume_id, machine_id))
        return false;

    calculated = true;
    calculated_id = *machine_id;
    return true;
}

