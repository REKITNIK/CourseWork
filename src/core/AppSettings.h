#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QString>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

namespace AppSettings {

// Ключ шифрования для файла курса
const QString ENCRYPTION_KEY = "SECRET_KEY_123";

/**
* @brief Возвращает полный, унифицированный путь к файлу course.bin.
* Файл располагается в системном каталоге для данных приложения.
* Эта функция - единственный источник пути к файлу в приложении.
* @return QString с абсолютным путем к course.bin
*/
inline QString getCourseBinaryPath() {
    // Получаем платформо-независимый путь к каталогу данных приложения
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dataDir(dataPath);

    // Гарантируем, что каталог существует. Хотя main() его уже создает,
    // эта проверка обеспечивает дополнительную надежность.
    if (!dataDir.exists()) {
        qWarning() << "AppDataLocation directory does not exist, creating:" << dataPath;
        if (!dataDir.mkpath(".")) {
            qCritical() << "Failed to create AppDataLocation directory!";
        }
    }
    return dataDir.filePath("course.bin");
}

} // namespace AppSettings

#endif // APPSETTINGS_H
