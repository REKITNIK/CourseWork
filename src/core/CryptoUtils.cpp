#include "CryptoUtils.h"
#include <QCryptographicHash>

QByteArray CryptoUtils::xorEncryptDecrypt(const QByteArray& data, const QString& key) {
    if (data.isEmpty() || key.isEmpty()) {
        return data;
    }

    QByteArray keyBytes = key.toUtf8();
    QByteArray result = data;

    // Применение XOR шифрования с циклическим использованием ключа
    for (int i = 0; i < result.size(); ++i) {
        result[i] = result[i] ^ keyBytes[i % keyBytes.size()];
    }

    return result;
}

QString CryptoUtils::hashPassword(const QString& password) {
    QByteArray passwordBytes = password.toUtf8();
    QByteArray hash = QCryptographicHash::hash(passwordBytes, QCryptographicHash::Sha256);
    return hash.toHex();
}