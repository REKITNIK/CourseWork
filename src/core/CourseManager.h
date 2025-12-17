#ifndef COURSEMANAGER_H
#define COURSEMANAGER_H

#include <QString>
#include "models/Structures.h"

/**
 * @brief Класс для управления курсами.
 * Предоставляет статические методы для загрузки курсов из JSON,
 * сохранения в зашифрованный бинарный формат и загрузки из него.
 */
class CourseManager
{
public:
    /**
     * @brief Загружает курс из JSON файла.
     * @param jsonPath Путь к JSON файлу с данными курса
     * @return Объект Course с загруженными данными
     */
    static Course loadCourseFromJSON(const QString& jsonPath);
    
    /**
     * @brief Сохраняет курс в зашифрованный бинарный файл.
     * @param course Объект курса для сохранения
     * @param binPath Путь к бинарному файлу для сохранения
     * @param key Ключ для шифрования данных
     * @return true если сохранение прошло успешно, false в противном случае
     */
    static bool saveCourseToBinary(const Course& course, const QString& binPath, const QString& key);
    
    /**
     * @brief Загружает курс из зашифрованного бинарного файла.
     * @param binPath Путь к бинарному файлу
     * @param key Ключ для расшифровки данных
     * @return Объект Course с загруженными данными
     */
    static Course loadCourseFromBinary(const QString& binPath, const QString& key);

private:
    static const quint32 MAGIC_NUMBER = 0x434F5253; // "CORS" in hex
    CourseManager() = delete;
};

#endif // COURSEMANAGER_H