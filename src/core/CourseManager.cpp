#include "CourseManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDataStream>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include "CryptoUtils.h"

Course CourseManager::loadCourseFromJSON(const QString& jsonPath) {
    Course course;

    QFile file(jsonPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open JSON file:" << jsonPath;
        return course;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << parseError.errorString();
        return course;
    }

    if (!doc.isArray()) {
        qWarning() << "JSON document is not an array";
        return course;
    }

    QJsonArray chaptersArray = doc.array();

    // Обработка каждой главы курса
    for (const QJsonValue& chapterValue : chaptersArray) {
        if (!chapterValue.isObject()) {
            continue;
        }

        QJsonObject chapterObj = chapterValue.toObject();

        Chapter chapter;
        chapter.id = chapterObj["id"].toInt();
        chapter.title = chapterObj["title"].toString();
        chapter.content = chapterObj["content"].toString();

        // Парсинг вопросов для текущей главы
        QJsonArray questionsArray = chapterObj["questions"].toArray();
        for (const QJsonValue& questionValue : questionsArray) {
            if (!questionValue.isObject()) {
                continue;
            }

            QJsonObject questionObj = questionValue.toObject();

            Question question;
            question.q_text = questionObj["q_text"].toString();
            question.correct_index = questionObj["correct_index"].toInt();

            // Парсинг вариантов ответов
            QJsonArray optionsArray = questionObj["options"].toArray();
            for (const QJsonValue& optionValue : optionsArray) {
                question.options.append(optionValue.toString());
            }

            chapter.questions.append(question);
        }

        course.chapters.append(chapter);
    }

    return course;
}

bool CourseManager::saveCourseToBinary(const Course& course, const QString& binPath, const QString& key) {
    QFile file(binPath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Cannot open binary file for writing:" << binPath << "Error:" << file.errorString();
        return false;
    }

    QByteArray courseData;
    QDataStream courseStream(&courseData, QIODevice::WriteOnly);
    courseStream << course;

    QByteArray encryptedData = CryptoUtils::xorEncryptDecrypt(courseData, key);

    QDataStream fileStream(&file);
    fileStream << MAGIC_NUMBER;
    fileStream << encryptedData;

    file.close();
    qInfo() << "Курс успешно сохранен в:" << binPath;
    return true;
}


Course CourseManager::loadCourseFromBinary(const QString& binPath, const QString& key) {
    Course course;

    QFile file(binPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open binary file for reading:" << binPath;
        return course;
    }

    QDataStream fileStream(&file);

    // Чтение и проверка магического числа
    quint32 magicNumber;
    fileStream >> magicNumber;

    if (magicNumber != MAGIC_NUMBER) {
        qWarning() << "Invalid file format - magic number mismatch";
        file.close();
        return course;
    }

    // Чтение зашифрованных данных
    QByteArray encryptedData;
    fileStream >> encryptedData;
    file.close();

    // Расшифровка данных
    QByteArray decryptedData = CryptoUtils::xorEncryptDecrypt(encryptedData, key);

    // Десериализация курса из расшифрованных данных
    QDataStream courseStream(&decryptedData, QIODevice::ReadOnly);
    courseStream >> course;

    return course;
}
