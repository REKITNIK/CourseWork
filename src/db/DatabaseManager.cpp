#include "db/DatabaseManager.h"

const QString DatabaseManager::DB_HOSTNAME = "localhost";
const QString DatabaseManager::DB_NAME = "course_db";
const QString DatabaseManager::DB_USERNAME = "postgres";
const QString DatabaseManager::DB_PASSWORD = "";
const int DatabaseManager::DB_PORT = 5432;

DatabaseManager::DatabaseManager(QObject* parent)
    : QObject(parent), m_connected(false) {
    m_database = QSqlDatabase::addDatabase("QPSQL");
}

DatabaseManager::~DatabaseManager() {
    if (m_database.isOpen()) {
        m_database.close();
    }
}

DatabaseManager& DatabaseManager::getInstance() {
    static DatabaseManager instance;
    return instance;
}

bool DatabaseManager::connectToDatabase() {
    if (m_connected && m_database.isOpen()) {
        return true;
    }

    m_database.setHostName(DB_HOSTNAME);
    m_database.setPort(DB_PORT);
    m_database.setDatabaseName(DB_NAME);
    m_database.setUserName(DB_USERNAME);
    m_database.setPassword(DB_PASSWORD);

    if (!m_database.open()) {
        m_lastError = QString("Failed to connect to database: %1").arg(m_database.lastError().text());
        qDebug() << m_lastError;
        m_connected = false;
        return false;
    }

    m_connected = true;
    m_lastError.clear();
    qDebug() << "Successfully connected to PostgreSQL database:" << DB_NAME;
    return true;
}

bool DatabaseManager::initDatabase() {
    if (!isConnected()) {
        m_lastError = "Database not connected";
        return false;
    }

    // Попытка загрузки схемы из файла
    if (loadSchemaFromFile()) {
        qDebug() << "Database schema loaded from file successfully";
        return true;
    }

    // Резервный вариант с жестко заданной схемой
    qDebug() << "Loading schema from file failed, using hardcoded schema";
    return createTables();
}

bool DatabaseManager::loadSchemaFromFile() {
    QFile schemaFile(":/schema.sql");

    if (!schemaFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = "Cannot open schema file from resources (:/schema.sql)";
        qDebug() << m_lastError;
        return false;
    }

    QTextStream in(&schemaFile);
    QString schemaContent = in.readAll();
    schemaFile.close();

    QStringList statements = schemaContent.split(';', Qt::SkipEmptyParts);

    for (const QString& statement : statements) {
        QString trimmedStatement = statement.trimmed();
        if (trimmedStatement.isEmpty() || trimmedStatement.startsWith("--")) {
            continue;
        }

        QSqlQuery query(m_database);
        if (!query.exec(trimmedStatement)) {
            m_lastError = QString("Failed to execute schema statement: %1")
            .arg(query.lastError().text());
            qDebug() << m_lastError;
            qDebug() << "Statement:" << trimmedStatement;
            return false;
        }
    }

    return true;
}


bool DatabaseManager::createTables() {
    QSqlQuery query(m_database);

    // Создание таблицы пользователей
    QString createUsersTable = R"(
        CREATE TABLE IF NOT EXISTS users (
            id SERIAL PRIMARY KEY,
            login TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL,
            role TEXT NOT NULL DEFAULT 'student',
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    )";

    if (!query.exec(createUsersTable)) {
        m_lastError = QString("Failed to create users table: %1")
        .arg(query.lastError().text());
        qDebug() << m_lastError;
        return false;
    }

    // Создание таблицы прогресса обучения
    QString createProgressTable = R"(
        CREATE TABLE IF NOT EXISTS study_progress (
            user_id INTEGER NOT NULL,
            chapter_id INTEGER NOT NULL,
            status TEXT NOT NULL DEFAULT 'not_started',
            last_score INTEGER DEFAULT 0,
            updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            PRIMARY KEY (user_id, chapter_id),
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
        )
    )";

    if (!query.exec(createProgressTable)) {
        m_lastError = QString("Failed to create study_progress table: %1")
        .arg(query.lastError().text());
        qDebug() << m_lastError;
        return false;
    }

    QString createAdmin = R"(
        INSERT INTO users (login, password_hash, role)
        VALUES (
            'admin',
            '8c6976e5b5410415bde908bd4dee15dfb167a9c873fc4bb8a81f6f2ab448a918',
            'admin'
        )
        ON CONFLICT (login) DO NOTHING
    )";
    // SHA-256("admin")

    if (!query.exec(createAdmin)) {
        m_lastError = QString("Failed to create default admin user: %1")
        .arg(query.lastError().text());
        qDebug() << m_lastError;
    } else {
        if (query.numRowsAffected() > 0) {
            qDebug() << "Default admin user 'admin' with password 'admin' created.";
        } else {
            qDebug() << "Default admin user 'admin' already exists.";
        }
    }

    // Создание индексов для оптимизации запросов
    query.exec("CREATE INDEX IF NOT EXISTS idx_users_login ON users(login)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_study_progress_user_id ON study_progress(user_id)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_study_progress_chapter_id ON study_progress(chapter_id)");

    qDebug() << "Database tables created successfully";
    return true;
}


bool DatabaseManager::isConnected() const {
    return m_connected && m_database.isOpen();
}

QStringList DatabaseManager::getTableList() const {
    if (!isConnected()) {
        return QStringList();
    }

    return m_database.tables();
}

QString DatabaseManager::getLastError() const {
    return m_lastError;
}

bool DatabaseManager::executeQuery(const QString& query) {
    if (!isConnected()) {
        m_lastError = "Database not connected";
        return false;
    }

    QSqlQuery sqlQuery(m_database);
    if (!sqlQuery.exec(query)) {
        m_lastError = QString("Query execution failed: %1").arg(sqlQuery.lastError().text());
        return false;
    }

    return true;
}

QSqlQuery DatabaseManager::executeSelectQuery(const QString& query) {
    QSqlQuery sqlQuery(m_database);
    if (isConnected()) {
        sqlQuery.exec(query);
    }
    return sqlQuery;
}

bool DatabaseManager::registerUser(const QString& login, const QString& passwordHash, const QString& role) {
    if (!isConnected()) {
        m_lastError = "Database not connected";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("INSERT INTO users (login, password_hash, role) VALUES (?, ?, ?)");
    query.addBindValue(login);
    query.addBindValue(passwordHash);
    query.addBindValue(role);

    if (!query.exec()) {
        m_lastError = QString("Failed to register user: %1").arg(query.lastError().text());
        qDebug() << m_lastError;
        return false;
    }

    qDebug() << "User registered successfully:" << login << "with role:" << role;
    return true;
}

QString DatabaseManager::authenticateUser(const QString& login, const QString& passwordHash) {
    if (!isConnected()) {
        m_lastError = "Database not connected";
        return QString();
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT role FROM users WHERE login = ? AND password_hash = ?");
    query.addBindValue(login);
    query.addBindValue(passwordHash);

    if (!query.exec()) {
        m_lastError = QString("Authentication query failed: %1").arg(query.lastError().text());
        qDebug() << m_lastError;
        return QString();
    }

    if (query.next()) {
        QString role = query.value(0).toString();
        qDebug() << "User authenticated successfully:" << login << "with role:" << role;
        return role;
    }

    qDebug() << "Authentication failed for user:" << login;
    return QString();
}

QPair<QString, int> DatabaseManager::authenticateUserWithId(const QString& login, const QString& passwordHash) {
    if (!isConnected()) {
        m_lastError = "Database not connected";
        return QPair<QString, int>(QString(), -1);
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT id, role FROM users WHERE login = ? AND password_hash = ?");
    query.addBindValue(login);
    query.addBindValue(passwordHash);

    if (!query.exec()) {
        m_lastError = QString("Authentication query failed: %1").arg(query.lastError().text());
        qDebug() << m_lastError;
        return QPair<QString, int>(QString(), -1);
    }

    if (query.next()) {
        int userId = query.value(0).toInt();
        QString role = query.value(1).toString();
        qDebug() << "User authenticated successfully:" << login << "with role:" << role << "and ID:" << userId;
        return QPair<QString, int>(role, userId);
    }

    qDebug() << "Authentication failed for user:" << login;
    return QPair<QString, int>(QString(), -1);
}

QSqlTableModel* DatabaseManager::getUsersModel() {
    if (!isConnected()) {
        m_lastError = "Database not connected";
        return nullptr;
    }

    QSqlTableModel* model = new QSqlTableModel(nullptr, m_database);
    model->setTable("users");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);

    // Установка заголовков столбцов
    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Login");
    model->setHeaderData(2, Qt::Horizontal, "Password Hash");
    model->setHeaderData(3, Qt::Horizontal, "Role");
    model->setHeaderData(4, Qt::Horizontal, "Created At");

    model->select();
    return model;
}

void DatabaseManager::saveProgress(int userId, int chapterId, int score, const QString& status) {
    if (!isConnected()) {
        m_lastError = "Database not connected";
        qDebug() << m_lastError;
        return;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT user_id FROM study_progress WHERE user_id = ? AND chapter_id = ?");
    query.addBindValue(userId);
    query.addBindValue(chapterId);

    if (!query.exec()) {
        m_lastError = QString("Failed to check existing progress: %1").arg(query.lastError().text());
        qDebug() << m_lastError;
        return;
    }

    if (query.next()) {
        // Обновление существующей записи
        QSqlQuery updateQuery(m_database);
        updateQuery.prepare("UPDATE study_progress SET last_score = ?, status = ?, updated_at = CURRENT_TIMESTAMP WHERE user_id = ? AND chapter_id = ?");
        updateQuery.addBindValue(score);
        updateQuery.addBindValue(status);
        updateQuery.addBindValue(userId);
        updateQuery.addBindValue(chapterId);

        if (!updateQuery.exec()) {
            m_lastError = QString("Failed to update progress: %1").arg(updateQuery.lastError().text());
            qDebug() << m_lastError;
        } else {
            qDebug() << "Progress updated for user" << userId << "chapter" << chapterId << "status:" << status;
        }
    } else {
        // Вставка новой записи
        QSqlQuery insertQuery(m_database);
        insertQuery.prepare("INSERT INTO study_progress (user_id, chapter_id, last_score, status) VALUES (?, ?, ?, ?)");
        insertQuery.addBindValue(userId);
        insertQuery.addBindValue(chapterId);
        insertQuery.addBindValue(score);
        insertQuery.addBindValue(status);

        if (!insertQuery.exec()) {
            m_lastError = QString("Failed to insert progress: %1").arg(insertQuery.lastError().text());
            qDebug() << m_lastError;
        } else {
            qDebug() << "Progress saved for user" << userId << "chapter" << chapterId << "status:" << status;
        }
    }
}

QPair<int, QString> DatabaseManager::getLastProgress(int userId) {
    if (!isConnected()) {
        m_lastError = "Database not connected";
        qDebug() << m_lastError;
        return QPair<int, QString>(-1, QString());
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT chapter_id, status FROM study_progress WHERE user_id = ? ORDER BY chapter_id DESC LIMIT 1");
    query.addBindValue(userId);

    if (!query.exec()) {
        m_lastError = QString("Failed to get last progress: %1").arg(query.lastError().text());
        qDebug() << m_lastError;
        return QPair<int, QString>(-1, QString());
    }

    if (query.next()) {
        int chapterId = query.value(0).toInt();
        QString status = query.value(1).toString();
        qDebug() << "Last progress for user" << userId << ": chapter" << chapterId << "status:" << status;
        return QPair<int, QString>(chapterId, status);
    }

    qDebug() << "No progress found for user" << userId << ", starting from chapter 0";
    return QPair<int, QString>(0, "not_started"); // Возвращаем корректный начальный статус
}
