#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlTableModel>
#include <QString>
#include <QStringList>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QPair>

/**
 * @brief Класс для управления базой данных.
 * Реализует паттерн Singleton для работы с PostgreSQL базой данных.
 * Обеспечивает аутентификацию пользователей и отслеживание прогресса обучения.
 */
class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Получает единственный экземпляр класса (Singleton).
     * @return Ссылка на экземпляр DatabaseManager
     */
    static DatabaseManager& getInstance();
    
    /**
     * @brief Устанавливает соединение с базой данных.
     * @return true если соединение установлено успешно, false в противном случае
     */
    bool connectToDatabase();
    
    /**
     * @brief Инициализирует структуру базы данных.
     * @return true если инициализация прошла успешно, false в противном случае
     */
    bool initDatabase();
    
    /**
     * @brief Проверяет состояние соединения с базой данных.
     * @return true если соединение активно, false в противном случае
     */
    bool isConnected() const;
    
    /**
     * @brief Получает список таблиц в базе данных.
     * @return Список названий таблиц
     */
    QStringList getTableList() const;
    
    /**
     * @brief Получает текст последней ошибки.
     * @return Строка с описанием последней ошибки
     */
    QString getLastError() const;
    
    /**
     * @brief Выполняет SQL запрос без возврата данных.
     * @param query SQL запрос для выполнения
     * @return true если запрос выполнен успешно, false в противном случае
     */
    bool executeQuery(const QString& query);
    
    /**
     * @brief Выполняет SELECT запрос с возвратом данных.
     * @param query SQL SELECT запрос
     * @return Объект QSqlQuery с результатами запроса
     */
    QSqlQuery executeSelectQuery(const QString& query);
    
    /**
     * @brief Регистрирует нового пользователя в системе.
     * @param login Логин пользователя
     * @param passwordHash Хешированный пароль
     * @param role Роль пользователя (по умолчанию "student")
     * @return true если регистрация прошла успешно, false в противном случае
     */
    bool registerUser(const QString& login, const QString& passwordHash, const QString& role = "student");
    
    /**
     * @brief Аутентифицирует пользователя по логину и паролю.
     * @param login Логин пользователя
     * @param passwordHash Хешированный пароль
     * @return Роль пользователя или пустая строка при неудаче
     */
    QString authenticateUser(const QString& login, const QString& passwordHash);
    
    /**
     * @brief Аутентифицирует пользователя и возвращает роль с ID.
     * @param login Логин пользователя
     * @param passwordHash Хешированный пароль
     * @return Пара (роль, ID пользователя)
     */
    QPair<QString, int> authenticateUserWithId(const QString& login, const QString& passwordHash);
    
    /**
     * @brief Получает модель данных пользователей для отображения в таблице.
     * @return Указатель на QSqlTableModel с данными пользователей
     */
    QSqlTableModel* getUsersModel();
    
    /**
     * @brief Сохраняет прогресс студента по главе.
     * @param userId ID пользователя
     * @param chapterId ID главы
     * @param score Количество баллов
     * @param status Статус прохождения
     */
    void saveProgress(int userId, int chapterId, int score, const QString &status);
    
    /**
     * @brief Получает последний прогресс студента.
     * @param userId ID пользователя
     * @return Пара (ID последней главы, статус)
     */
    QPair<int, QString> getLastProgress(int userId);

    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();
    
    bool createTables();
    bool loadSchemaFromFile();

    static const QString DB_HOSTNAME;
    static const QString DB_NAME;
    static const QString DB_USERNAME;
    static const QString DB_PASSWORD;
    static const int DB_PORT;
    
    QSqlDatabase m_database;
    QString m_lastError;
    bool m_connected;
};

#endif // DATABASEMANAGER_H
