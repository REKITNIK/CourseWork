#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QString>

/**
 * @brief Диалог авторизации пользователя.
 * Предоставляет интерфейс для входа в систему и регистрации новых пользователей.
 */
class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор диалога авторизации.
     * @param parent Родительский виджет
     */
    explicit LoginDialog(QWidget *parent = nullptr);
    
    /**
     * @brief Получает роль авторизованного пользователя.
     * @return Роль пользователя ("admin" или "student")
     */
    QString getRole() const;
    
    /**
     * @brief Получает ID авторизованного пользователя.
     * @return ID пользователя в базе данных
     */
    int getUserId() const;

private slots:
    /**
     * @brief Обработчик нажатия кнопки входа.
     */
    void onLoginClicked();
    
    /**
     * @brief Обработчик нажатия кнопки регистрации.
     */
    void onRegisterClicked();

private:
    /**
     * @brief Настраивает пользовательский интерфейс диалога.
     */
    void setupUI();
    
    QLineEdit* m_loginEdit;
    QLineEdit* m_passwordEdit;
    QPushButton* m_loginButton;
    QPushButton* m_registerButton;
    
    QString m_userRole;
    int m_userId;
};

#endif // LOGINDIALOG_H