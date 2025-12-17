#ifndef CRYPTOUTILS_H
#define CRYPTOUTILS_H

#include <QByteArray>
#include <QString>

/**
 * @brief Класс для криптографических операций.
 * Предоставляет статические методы для шифрования/дешифрования данных
 * и хеширования паролей.
 */
class CryptoUtils
{
public:
    /**
     * @brief Шифрует или дешифрует данные с помощью XOR алгоритма.
     * @param data Данные для шифрования/дешифрования
     * @param key Ключ для шифрования
     * @return Зашифрованные/расшифрованные данные
     */
    static QByteArray xorEncryptDecrypt(const QByteArray& data, const QString& key);
    
    /**
     * @brief Хеширует пароль для безопасного хранения.
     * @param password Пароль для хеширования
     * @return Хешированный пароль в виде строки
     */
    static QString hashPassword(const QString& password);

private:
    CryptoUtils() = delete;
};

#endif // CRYPTOUTILS_H