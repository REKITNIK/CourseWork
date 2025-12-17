#ifndef STUDENTWINDOW_H
#define STUDENTWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QTextBrowser>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDebug>

#include "../models/Structures.h"
#include "../core/CourseManager.h"
#include "../db/DatabaseManager.h"

/**
 * @brief Главное окно студента.
 * Предоставляет интерфейс для изучения теоретического материала
 * и прохождения тестов по главам курса.
 */
class StudentWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор окна студента.
     * @param userId ID студента в базе данных
     * @param parent Родительский виджет
     */
    explicit StudentWindow(int userId, QWidget* parent = nullptr);
    
    /**
     * @brief Деструктор окна студента.
     */
    ~StudentWindow();

private slots:
    /**
     * @brief Обработчик нажатия кнопки начала тестирования.
     */
    void onTakeTestClicked();
    
    /**
     * @brief Обработчик выбора ответа на вопрос.
     */
    void onAnswerClicked();

    /**
     * @brief Обработчик нажатия кнопки "Назад к теории".
     */
    void onBackToTheoryClicked();

private:
    /**
     * @brief Настраивает пользовательский интерфейс.
     */
    void setupUI();
    
    /**
     * @brief Загружает данные курса из файла.
     */
    void loadCourse();
    
    /**
     * @brief Инициализирует прогресс студента.
     */
    void initializeProgress();
    
    /**
     * @brief Показывает страницу с теоретическим материалом.
     */
    void showTheoryPage();
    
    /**
     * @brief Показывает страницу с тестом.
     */
    void showTestPage();
    
    /**
     * @brief Загружает текущий вопрос теста.
     */
    void loadCurrentQuestion();
    
    /**
     * @brief Обрабатывает ответ студента.
     * @param isCorrect true если ответ правильный, false в противном случае
     */
    void processAnswer(bool isCorrect);
    
    /**
     * @brief Возвращает к изучению теории.
     */
    void resetToTheory();
    
    /**
     * @brief Переходит к следующей главе курса.
     */
    void moveToNextChapter();
    
    // Компоненты интерфейса
    QStackedWidget* m_stackedWidget;
    
    // Страница теории (страница 0)
    QWidget* m_theoryPage;
    QTextBrowser* m_theoryBrowser;
    QPushButton* m_takeTestButton;
    
    // Страница тестирования (страница 1)
    QWidget* m_testPage;
    QLabel* m_questionLabel;
    QButtonGroup* m_answerGroup;
    QList<QRadioButton*> m_answerButtons;
    QPushButton* m_answerButton;
    QPushButton* m_backToTheoryButton;
    
    // Переменные состояния
    int m_userId;
    int m_currentChapterIndex;
    int m_currentQuestionIndex;
    int m_errorsCount;
    Course m_course;
};

#endif // STUDENTWINDOW_H
