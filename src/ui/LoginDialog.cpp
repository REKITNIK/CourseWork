#include "ui/LoginDialog.h"
#include "db/DatabaseManager.h"
#include "core/CryptoUtils.h"

LoginDialog::LoginDialog(QWidget* parent)
    : QDialog(parent), m_userId(-1) {
    setWindowTitle("Авторизация - HTTP Proxy Course");
    setFixedSize(350, 200);
    setModal(true);

    setupUI();
}

void LoginDialog::setupUI() {
    // Основной макет диалога
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Заголовок приложения
    QLabel* titleLabel = new QLabel("Система обучения HTTP Proxy", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 14px; font-weight: bold; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);

    // Форма для полей ввода
    QFormLayout* formLayout = new QFormLayout();

    m_loginEdit = new QLineEdit(this);
    m_loginEdit->setPlaceholderText("Введите логин");
    formLayout->addRow("Логин:", m_loginEdit);

    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText("Введите пароль");
    formLayout->addRow("Пароль:", m_passwordEdit);

    mainLayout->addLayout(formLayout);

    // Макет для кнопок
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    m_loginButton = new QPushButton("Войти", this);
    m_loginButton->setDefault(true);
    m_loginButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; padding: 8px; border: none; border-radius: 4px; } QPushButton:hover { background-color: #45a049; }");

    m_registerButton = new QPushButton("Регистрация", this);
    m_registerButton->setStyleSheet("QPushButton { background-color: #2196F3; color: white; padding: 8px; border: none; border-radius: 4px; } QPushButton:hover { background-color: #1976D2; }");

    buttonLayout->addWidget(m_loginButton);
    buttonLayout->addWidget(m_registerButton);

    mainLayout->addLayout(buttonLayout);

    // Подключение сигналов к слотам
    connect(m_loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    connect(m_registerButton, &QPushButton::clicked, this, &LoginDialog::onRegisterClicked);

    // Обработка нажатия Enter для входа
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &LoginDialog::onLoginClicked);
}

QString LoginDialog::getRole() const {
    return m_userRole;
}

int LoginDialog::getUserId() const {
    return m_userId;
}

void LoginDialog::onLoginClicked() {
    QString login = m_loginEdit->text().trimmed();
    QString password = m_passwordEdit->text();

    if (login.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Пожалуйста, заполните все поля.");
        return;
    }

    // Хеширование пароля для безопасности
    QString passwordHash = CryptoUtils::hashPassword(password);

    // Аутентификация пользователя в базе данных
    DatabaseManager& db = DatabaseManager::getInstance();
    QPair<QString, int> authResult = db.authenticateUserWithId(login, passwordHash);

    if (authResult.first.isEmpty() || authResult.second == -1) {
        QMessageBox::warning(this, "Ошибка авторизации",
                           "Неверный логин или пароль.\nПроверьте введенные данные и попробуйте снова.");
        m_passwordEdit->clear();
        m_passwordEdit->setFocus();
        return;
    }

    m_userRole = authResult.first;
    m_userId = authResult.second;
    accept();
}

void LoginDialog::onRegisterClicked() {
    QString login = m_loginEdit->text().trimmed();
    QString password = m_passwordEdit->text();

    if (login.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Пожалуйста, заполните все поля.");
        return;
    }

    // Валидация длины логина и пароля
    if (login.length() < 3) {
        QMessageBox::warning(this, "Ошибка", "Логин должен содержать минимум 3 символа.");
        return;
    }

    if (password.length() < 4) {
        QMessageBox::warning(this, "Ошибка", "Пароль должен содержать минимум 4 символа.");
        return;
    }

    // Хеширование пароля перед сохранением
    QString passwordHash = CryptoUtils::hashPassword(password);

    // Регистрация пользователя с ролью "student" по умолчанию
    DatabaseManager& db = DatabaseManager::getInstance();
    if (db.registerUser(login, passwordHash, "student")) {
        QMessageBox::information(this, "Успех",
                               QString("Пользователь '%1' успешно зарегистрирован!\nТеперь вы можете войти в систему.").arg(login));
        m_passwordEdit->clear();
    } else {
        QMessageBox::critical(this, "Ошибка регистрации",
                            QString("Не удалось зарегистрировать пользователя.\nВозможно, такой логин уже существует.\n\nОшибка: %1").arg(db.getLastError()));
    }
}