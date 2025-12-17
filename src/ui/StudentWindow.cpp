#include "StudentWindow.h"
#include "core/AppSettings.h"

StudentWindow::StudentWindow(int userId, QWidget* parent)
    : QMainWindow(parent)
    , m_stackedWidget(nullptr)
    , m_theoryPage(nullptr)
    , m_theoryBrowser(nullptr)
    , m_takeTestButton(nullptr)
    , m_testPage(nullptr)
    , m_questionLabel(nullptr)
    , m_answerGroup(nullptr)
    , m_answerButton(nullptr)
    , m_backToTheoryButton(nullptr)
    , m_userId(userId)
    , m_currentChapterIndex(0)
    , m_currentQuestionIndex(0)
    , m_errorsCount(0)
{
    setWindowTitle("Система обучения HTTP Proxy - Студент");
    setMinimumSize(800, 600);
    
    setupUI();
    loadCourse();
    initializeProgress();
}

StudentWindow::~StudentWindow()
{
    // Qt автоматически управляет очисткой памяти
}

void StudentWindow::setupUI()
{
    // Создание основного стекового виджета
    m_stackedWidget = new QStackedWidget(this);
    setCentralWidget(m_stackedWidget);

    // ... (код для страницы теории остается без изменений) ...
    m_theoryPage = new QWidget();
    QVBoxLayout* theoryLayout = new QVBoxLayout(m_theoryPage);
    m_theoryBrowser = new QTextBrowser();
    m_theoryBrowser->setReadOnly(true);
    theoryLayout->addWidget(m_theoryBrowser);
    m_takeTestButton = new QPushButton("Пройти тест");
    m_takeTestButton->setMinimumHeight(40);
    connect(m_takeTestButton, &QPushButton::clicked, this, &StudentWindow::onTakeTestClicked);
    QHBoxLayout* theoryButtonLayout = new QHBoxLayout();
    theoryButtonLayout->addStretch();
    theoryButtonLayout->addWidget(m_takeTestButton);
    theoryButtonLayout->addStretch();
    theoryLayout->addLayout(theoryButtonLayout);
    m_stackedWidget->addWidget(m_theoryPage); // Индекс 0

    // Создание страницы тестирования (страница 1)
    m_testPage = new QWidget();
    QVBoxLayout* testLayout = new QVBoxLayout(m_testPage);

    m_questionLabel = new QLabel();
    m_questionLabel->setWordWrap(true);
    m_questionLabel->setStyleSheet("QLabel { font-size: 14px; font-weight: bold; margin: 10px; }");
    testLayout->addWidget(m_questionLabel);

    m_answerGroup = new QButtonGroup(this);

    QWidget* answersWidget = new QWidget();
    answersWidget->setObjectName("answersWidget");
    new QVBoxLayout(answersWidget);

    testLayout->addWidget(answersWidget);
    testLayout->addStretch();

    m_answerButton = new QPushButton("Ответить");
    m_answerButton->setMinimumHeight(40);
    connect(m_answerButton, &QPushButton::clicked, this, &StudentWindow::onAnswerClicked);

    m_backToTheoryButton = new QPushButton("Назад к теории");
    m_backToTheoryButton->setMinimumHeight(40);
    connect(m_backToTheoryButton, &QPushButton::clicked, this, &StudentWindow::onBackToTheoryClicked);

    QHBoxLayout* testButtonLayout = new QHBoxLayout();
    testButtonLayout->addStretch();
    testButtonLayout->addWidget(m_backToTheoryButton);
    testButtonLayout->addWidget(m_answerButton);
    testButtonLayout->addStretch();
    testLayout->addLayout(testButtonLayout);

    m_stackedWidget->addWidget(m_testPage); // Индекс 1
}

void StudentWindow::loadCourse()
{
    m_course = CourseManager::loadCourseFromBinary(AppSettings::getCourseBinaryPath(), AppSettings::ENCRYPTION_KEY);

    if (m_course.chapters.isEmpty()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось загрузить данные курса!");
        close();
        return;
    }

    qDebug() << "Course loaded successfully with" << m_course.chapters.size() << "chapters";
}

void StudentWindow::initializeProgress()
{
    DatabaseManager& db = DatabaseManager::getInstance();
    QPair<int, QString> lastProgress = db.getLastProgress(m_userId);
    
    int lastChapterId = lastProgress.first;
    QString lastStatus = lastProgress.second;
    
    if (lastChapterId == -1) {
        m_currentChapterIndex = 0;
    } else {
        if (lastStatus == "completed") {
            m_currentChapterIndex = lastChapterId + 1;
            if (m_currentChapterIndex >= m_course.chapters.size()) {
                // Course completed
                QMessageBox::information(this, "Поздравляем!", "Вы успешно завершили весь курс!");
                m_currentChapterIndex = m_course.chapters.size() - 1;
            }
        } else {
            m_currentChapterIndex = lastChapterId;
        }
    }

    if (m_currentChapterIndex < 0 || m_currentChapterIndex >= m_course.chapters.size()) {
        m_currentChapterIndex = 0;
    }
    
    showTheoryPage();
}

void StudentWindow::showTheoryPage()
{
    if (m_currentChapterIndex >= m_course.chapters.size()) {
        QMessageBox::information(this, "Курс завершен", "Вы прошли все главы курса!");
        return;
    }
    
    const Chapter& currentChapter = m_course.chapters[m_currentChapterIndex];
    
    setWindowTitle(QString("Система обучения HTTP Proxy - Глава %1: %2")
                   .arg(m_currentChapterIndex + 1)
                   .arg(currentChapter.title));
    
    QString theoryContent = QString("<h2>Глава %1: %2</h2><br>%3")
                           .arg(m_currentChapterIndex + 1)
                           .arg(currentChapter.title)
                           .arg(currentChapter.content);
    
    m_theoryBrowser->setHtml(theoryContent);
    
    m_takeTestButton->setEnabled(!currentChapter.questions.isEmpty());
    if (currentChapter.questions.isEmpty()) {
        m_takeTestButton->setText("Нет тестов для этой главы");
    } else {
        m_takeTestButton->setText("Пройти тест");
    }
    
    m_stackedWidget->setCurrentIndex(0);
}

void StudentWindow::showTestPage()
{
    m_currentQuestionIndex = 0;
    m_errorsCount = 0;
    loadCurrentQuestion();
    m_stackedWidget->setCurrentIndex(1);
}

void StudentWindow::loadCurrentQuestion()
{
    if (m_currentChapterIndex >= m_course.chapters.size()) {
        return;
    }

    const Chapter& currentChapter = m_course.chapters[m_currentChapterIndex];

    if (m_currentQuestionIndex >= currentChapter.questions.size()) {
        DatabaseManager& db = DatabaseManager::getInstance();
        db.saveProgress(m_userId, m_currentChapterIndex, 100, "completed");

        QMessageBox::information(
            this,
            "Тест пройден!",
            QString("Поздравляем! Вы успешно прошли тест по главе %1.")
                .arg(m_currentChapterIndex + 1)
            );

        moveToNextChapter();
        return;
    }

    const Question& currentQuestion = currentChapter.questions[m_currentQuestionIndex];

    QWidget* answersWidget = m_testPage->findChild<QWidget*>("answersWidget");
    if (answersWidget) {
        if (QLayout* layout = answersWidget->layout()) {
            QLayoutItem* item;
            while ((item = layout->takeAt(0)) != nullptr) {
                if (item->widget()) {
                    delete item->widget();
                }
                delete item;
            }
        }
    }
    m_answerButtons.clear();

    m_questionLabel->setText(QString("Вопрос %1 из %2:\n\n%3")
                                 .arg(m_currentQuestionIndex + 1)
                                 .arg(currentChapter.questions.size())
                                 .arg(currentQuestion.q_text));

    if (answersWidget) {
        QVBoxLayout* answersLayout = qobject_cast<QVBoxLayout*>(answersWidget->layout());
        if (!answersLayout) {
            answersLayout = new QVBoxLayout(answersWidget);
        }

        for (int i = 0; i < currentQuestion.options.size(); ++i) {
            QRadioButton* radioButton = new QRadioButton(currentQuestion.options[i]);
            radioButton->setStyleSheet("font-size: 13px; margin-left: 15px;");
            m_answerButtons.append(radioButton);
            m_answerGroup->addButton(radioButton, i);
            answersLayout->addWidget(radioButton);
        }
    }
}

void StudentWindow::onTakeTestClicked()
{
    if (m_currentChapterIndex >= m_course.chapters.size()) {
        return;
    }
    
    const Chapter& currentChapter = m_course.chapters[m_currentChapterIndex];
    
    if (currentChapter.questions.isEmpty()) {
        QMessageBox::information(this, "Нет тестов", "Для этой главы нет тестовых вопросов.");
        return;
    }
    
    showTestPage();
}

void StudentWindow::onAnswerClicked()
{
    if (m_currentChapterIndex >= m_course.chapters.size()) {
        return;
    }
    
    const Chapter& currentChapter = m_course.chapters[m_currentChapterIndex];
    
    if (m_currentQuestionIndex >= currentChapter.questions.size()) {
        return;
    }
    
    int selectedAnswer = m_answerGroup->checkedId();
    if (selectedAnswer == -1) {
        QMessageBox::warning(this, "Выберите ответ", "Пожалуйста, выберите один из вариантов ответа.");
        return;
    }
    
    const Question& currentQuestion = currentChapter.questions[m_currentQuestionIndex];
    bool isCorrect = (selectedAnswer == currentQuestion.correct_index);
    
    processAnswer(isCorrect);
}

void StudentWindow::processAnswer(bool isCorrect)
{
    if (isCorrect) {
        m_currentQuestionIndex++;
        loadCurrentQuestion();
    } else {
        m_errorsCount++;
        
        QMessageBox::warning(this, "Неверный ответ", 
                           QString("Ответ неверный. Ошибок: %1 из 3 допустимых.")
                           .arg(m_errorsCount));
        
        if (m_errorsCount >= 3) {
            DatabaseManager& db = DatabaseManager::getInstance();
            db.saveProgress(m_userId, m_currentChapterIndex, 0, "fail");
            
            QMessageBox::critical(this, "Тест не пройден", 
                                "Вы допустили 3 ошибки. Изучите теорию заново.");
            
            resetToTheory();
        } else {
            m_answerGroup->setExclusive(false);
            for (QRadioButton* button : m_answerButtons) {
                button->setChecked(false);
            }
            m_answerGroup->setExclusive(true);
        }
    }
}

void StudentWindow::resetToTheory()
{
    m_errorsCount = 0;
    m_currentQuestionIndex = 0;
    showTheoryPage();
}

void StudentWindow::moveToNextChapter()
{
    m_currentChapterIndex++;
    
    if (m_currentChapterIndex >= m_course.chapters.size()) {
        QMessageBox::information(this, "Курс завершен!", 
                               "Поздравляем! Вы успешно завершили весь курс обучения HTTP Proxy!");
        m_currentChapterIndex = m_course.chapters.size() - 1;
    }
    
    showTheoryPage();
}

void StudentWindow::onBackToTheoryClicked()
{
    // Мы можем просто переиспользовать существующую логику
    // для возврата к теории. Этот метод уже сбрасывает
    // счетчик ошибок, индекс вопроса и переключает виджет.
    resetToTheory();
}
