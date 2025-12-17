#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QStandardPaths>

#include "core/CourseManager.h"
#include "core/CryptoUtils.h"
#include "core/AppSettings.h"
#include "db/DatabaseManager.h"
#include "ui/LoginDialog.h"
#include "ui/AdminWindow.h"
#include "ui/StudentWindow.h"

bool initializeCourse() {
    qDebug() << "\n2. Checking course data...";

    const QString binaryWritePath = AppSettings::getCourseBinaryPath();

    const QString jsonResourcePath = ":/course.json";

    Course course = CourseManager::loadCourseFromBinary(binaryWritePath, AppSettings::ENCRYPTION_KEY);

    if (course.chapters.isEmpty()) {
        qInfo() << "Binary course file not found at" << binaryWritePath << ". Creating from source...";

        Course sourceCourse = CourseManager::loadCourseFromJSON(jsonResourcePath);

        if (sourceCourse.chapters.isEmpty()) {
            QMessageBox::critical(nullptr, "Критическая ошибка",
                                  QString("Не удалось загрузить данные из внутреннего ресурса:\n%1\n\nПриложение повреждено.").arg(jsonResourcePath));
            return false;
        }

        if (!CourseManager::saveCourseToBinary(sourceCourse, binaryWritePath, AppSettings::ENCRYPTION_KEY)) {
            QMessageBox::critical(nullptr, "Критическая ошибка",
                                  QString("Не удалось сохранить файл курса в:\n%1").arg(binaryWritePath));
            return false;
        }
        qInfo() << "Course successfully created and saved to" << binaryWritePath;
    } else {
        qInfo() << "Binary course file loaded successfully from" << binaryWritePath;
    }

    return true;
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setOrganizationName("Courseware");
    app.setApplicationName("HttpProxyCourse");

    qDebug() << "=== HTTP Proxy Learning System - GUI Application ===";

    qDebug() << "\n1. Initializing database connection...";
    DatabaseManager& db = DatabaseManager::getInstance();

    if (!db.connectToDatabase()) {
        QMessageBox::critical(nullptr, "Ошибка базы данных",
                              QString("Не удалось подключиться к базе данных:\n%1\n\nПроверьте настройки PostgreSQL.")
                                  .arg(db.getLastError()));
        return 1;
    }

    if (!db.initDatabase()) {
        QMessageBox::critical(nullptr, "Ошибка инициализации",
                              QString("Не удалось инициализировать базу данных:\n%1")
                                  .arg(db.getLastError()));
        return 1;
    }
    qDebug() << "Database initialized successfully";

    if (!initializeCourse()) {
        return 1;
    }

    qDebug() << "\n3. Starting authentication...";
    LoginDialog loginDialog;
    if (loginDialog.exec() != QDialog::Accepted) {
        qDebug() << "User cancelled login";
        return 0;
    }

    QMainWindow* mainWindow = nullptr;

    QString userRole = loginDialog.getRole();
    int userId = loginDialog.getUserId();
    qDebug() << "User authenticated with role:" << userRole << "and ID:" << userId;

    if (userRole == "admin") {
        qDebug() << "Launching admin interface...";
        mainWindow = new AdminWindow();
    } else if (userRole == "student") {
        qDebug() << "Launching student interface...";
        mainWindow = new StudentWindow(userId);
    } else {
        QMessageBox::warning(nullptr, "Неизвестная роль",
                             QString("Неизвестная роль пользователя: %1").arg(userRole));
        return 1;
    }

    if (mainWindow) {
        mainWindow->setAttribute(Qt::WA_DeleteOnClose);
        mainWindow->show();
        return app.exec();
    }

    return 1; // unreachable по логике, оставлено явно
}
