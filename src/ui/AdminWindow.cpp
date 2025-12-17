#include "ui/AdminWindow.h"
#include "db/DatabaseManager.h"
#include "core/CourseManager.h"
#include "core/AppSettings.h" // ДОБАВЛЕНО
#include <QDateTime>

AdminWindow::AdminWindow(QWidget* parent)
    : QMainWindow(parent), m_currentChapterIndex(-1)
{
    setWindowTitle("Панель администратора - HTTP Proxy Course");
    setMinimumSize(900, 600);
    resize(1200, 800);

    setupUI();
    loadCourseData();
}

void AdminWindow::setupUI()
{
    m_tabWidget = new QTabWidget(this);
    setCentralWidget(m_tabWidget);

    setupStudentsTab();
    setupCourseEditorTab();
}

void AdminWindow::setupStudentsTab()
{
    m_studentsTab = new QWidget();
    m_tabWidget->addTab(m_studentsTab, "Студенты");

    QVBoxLayout* mainLayout = new QVBoxLayout(m_studentsTab);

    QLabel* titleLabel = new QLabel("Управление пользователями", m_studentsTab);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);

    QHBoxLayout* controlsLayout = new QHBoxLayout();

    QLabel* searchLabel = new QLabel("Поиск:", m_studentsTab);
    controlsLayout->addWidget(searchLabel);

    m_searchLineEdit = new QLineEdit(m_studentsTab);
    m_searchLineEdit->setPlaceholderText("Введите логин для поиска...");
    controlsLayout->addWidget(m_searchLineEdit);

    controlsLayout->addStretch();

    m_reportButton = new QPushButton("Создать отчет", m_studentsTab);
    m_reportButton->setStyleSheet(
        "QPushButton { background-color: #FF9800; color: white; padding: 8px 16px; border: none; border-radius: 4px; }"
        "QPushButton:hover { background-color: #F57C00; }");
    controlsLayout->addWidget(m_reportButton);

    mainLayout->addLayout(controlsLayout);

    m_studentsTableView = new QTableView(m_studentsTab);
    m_studentsTableView->setAlternatingRowColors(true);
    m_studentsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_studentsTableView->setSortingEnabled(true);

    DatabaseManager& db = DatabaseManager::getInstance();
    m_usersModel = db.getUsersModel();

    if (m_usersModel) {
        m_proxyModel = new QSortFilterProxyModel(this);
        m_proxyModel->setSourceModel(m_usersModel);
        m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
        m_proxyModel->setFilterKeyColumn(1);

        m_studentsTableView->setModel(m_proxyModel);
        m_studentsTableView->hideColumn(2);
        m_studentsTableView->horizontalHeader()->setStretchLastSection(true);
        m_studentsTableView->resizeColumnsToContents();
    }

    mainLayout->addWidget(m_studentsTableView);

    connect(m_searchLineEdit, &QLineEdit::textChanged, this, &AdminWindow::onSearchTextChanged);
    connect(m_reportButton, &QPushButton::clicked, this, &AdminWindow::onGenerateReportClicked);
}

void AdminWindow::setupCourseEditorTab()
{
    m_courseEditorTab = new QWidget();
    m_tabWidget->addTab(m_courseEditorTab, "Редактор курса");

    QVBoxLayout* mainLayout = new QVBoxLayout(m_courseEditorTab);

    QLabel* titleLabel = new QLabel("Редактирование содержимого курса", m_courseEditorTab);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);

    QSplitter* splitter = new QSplitter(Qt::Horizontal, m_courseEditorTab);

    QWidget* leftWidget = new QWidget();
    QVBoxLayout* leftLayout = new QVBoxLayout(leftWidget);

    QLabel* chaptersLabel = new QLabel("Главы курса:", leftWidget);
    chaptersLabel->setStyleSheet("font-weight: bold;");
    leftLayout->addWidget(chaptersLabel);

    m_chaptersListWidget = new QListWidget(leftWidget);
    m_chaptersListWidget->setMaximumWidth(300);
    leftLayout->addWidget(m_chaptersListWidget);

    splitter->addWidget(leftWidget);

    QWidget* rightWidget = new QWidget();
    QVBoxLayout* rightLayout = new QVBoxLayout(rightWidget);

    QLabel* titleEditLabel = new QLabel("Заголовок главы:", rightWidget);
    titleEditLabel->setStyleSheet("font-weight: bold;");
    rightLayout->addWidget(titleEditLabel);

    m_chapterTitleEdit = new QLineEdit(rightWidget);
    m_chapterTitleEdit->setPlaceholderText("Введите заголовок главы...");
    rightLayout->addWidget(m_chapterTitleEdit);

    QLabel* contentEditLabel = new QLabel("Содержимое главы:", rightWidget);
    contentEditLabel->setStyleSheet("font-weight: bold; margin-top: 10px;");
    rightLayout->addWidget(contentEditLabel);

    m_chapterContentEdit = new QTextEdit(rightWidget);
    m_chapterContentEdit->setPlaceholderText("Введите содержимое главы...");
    rightLayout->addWidget(m_chapterContentEdit);

    m_saveChangesButton = new QPushButton("Сохранить изменения", rightWidget);
    m_saveChangesButton->setStyleSheet(
        "QPushButton { background-color: #4CAF50; color: white; padding: 10px 20px; border: none; border-radius: 4px; font-weight: bold; }"
        "QPushButton:hover { background-color: #45a049; }");
    m_saveChangesButton->setEnabled(false);
    rightLayout->addWidget(m_saveChangesButton);

    splitter->addWidget(rightWidget);
    splitter->setStretchFactor(1, 1);

    mainLayout->addWidget(splitter);

    connect(m_chaptersListWidget, &QListWidget::currentRowChanged, this, &AdminWindow::onChapterSelectionChanged);
    connect(m_saveChangesButton, &QPushButton::clicked, this, &AdminWindow::onSaveChangesClicked);
}

void AdminWindow::loadCourseData()
{
    m_course = CourseManager::loadCourseFromBinary(
        AppSettings::getCourseBinaryPath(),
        AppSettings::ENCRYPTION_KEY
        );

    if (m_course.chapters.isEmpty()) {
        QMessageBox::warning(
            this,
            "Ошибка",
            QString("Не удалось загрузить данные курса.\nФайл должен находиться по пути:\n%1")
                .arg(AppSettings::getCourseBinaryPath())
            );
        return;
    }

    m_chaptersListWidget->clear();
    for (int i = 0; i < m_course.chapters.size(); ++i) {
        const Chapter& chapter = m_course.chapters[i];
        m_chaptersListWidget->addItem(
            QString("Глава %1: %2").arg(i + 1).arg(chapter.title)
            );
    }

    m_chaptersListWidget->setCurrentRow(0);
}

void AdminWindow::onSearchTextChanged(const QString& text)
{
    if (m_proxyModel) {
        m_proxyModel->setFilterFixedString(text);
    }
}

void AdminWindow::onGenerateReportClicked()
{
    DatabaseManager& db = DatabaseManager::getInstance();

    QString queryString = R"(
        SELECT
            u.login,
            COUNT(sp.chapter_id) FILTER (WHERE sp.status = 'completed') AS completed_chapters,
            MAX(sp.updated_at) AS last_activity
        FROM
            users u
        LEFT JOIN
            study_progress sp ON u.id = sp.user_id
        WHERE
            u.role = 'student'
        GROUP BY
            u.id, u.login
        ORDER BY
            completed_chapters DESC,
            u.login;
    )";

    QSqlQuery query = db.executeSelectQuery(queryString);

    if (!query.isActive()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось выполнить запрос к базе данных для отчета.");
        return;
    }

    QString reportContent;
    reportContent += "=== ОТЧЕТ ПО УСПЕВАЕМОСТИ СТУДЕНТОВ ===\n";
    reportContent += QString("Всего глав в курсе: %1\n").arg(m_course.chapters.size());
    reportContent += QString("Дата создания отчета: %1\n\n")
                         .arg(QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss"));

    reportContent += QString("%1 | %2 | %3\n")
                         .arg("Логин студента", -20)
                         .arg("Глав пройдено", -15)
                         .arg("Последняя активность");
    reportContent += QString(60, '-') + "\n";

    while (query.next()) {
        QString login = query.value(0).toString();
        int completedCount = query.value(1).toInt();
        QString lastActivity =
            query.value(2).toDateTime().isNull()
                ? "Нет активности"
                : query.value(2).toDateTime().toString("dd.MM.yyyy hh:mm");

        reportContent += QString("%1 | %2 | %3\n")
                             .arg(login, -20)
                             .arg(QString::number(completedCount), -15)
                             .arg(lastActivity);
    }

    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString defaultFileName =
        defaultPath + QString("/Report_%1.txt")
                          .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));

    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Сохранить отчет",
        defaultFileName,
        "Text Files (*.txt);;All Files (*)"
        );

    if (fileName.isEmpty()) {
        return;
    }

    QFile reportFile(fileName);
    if (reportFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&reportFile);
        out << reportContent;
        reportFile.close();

        QMessageBox::information(
            this,
            "Отчет создан",
            QString("Отчет успешно сохранен в файл:\n%1").arg(fileName)
            );
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось сохранить отчет в файл.");
    }
}


void AdminWindow::onChapterSelectionChanged()
{
    m_currentChapterIndex = m_chaptersListWidget->currentRow();
    if (m_currentChapterIndex >= 0 && m_currentChapterIndex < m_course.chapters.size()) {
        updateChapterContent();
        m_saveChangesButton->setEnabled(true);
    } else {
        m_saveChangesButton->setEnabled(false);
        m_chapterTitleEdit->clear();
        m_chapterContentEdit->clear();
    }
}

void AdminWindow::updateChapterContent()
{
    const Chapter& chapter = m_course.chapters[m_currentChapterIndex];
    m_chapterTitleEdit->setText(chapter.title);
    m_chapterContentEdit->setPlainText(chapter.content);
}

void AdminWindow::onSaveChangesClicked()
{
    if (m_currentChapterIndex < 0 || m_currentChapterIndex >= m_course.chapters.size()) {
        QMessageBox::warning(this, "Ошибка", "Не выбрана глава для сохранения.");
        return;
    }

    QString newTitle = m_chapterTitleEdit->text().trimmed();
    if (newTitle.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заголовок главы не может быть пустым.");
        return;
    }

    Chapter& chapter = m_course.chapters[m_currentChapterIndex];
    chapter.title = newTitle;
    chapter.content = m_chapterContentEdit->toPlainText();

    m_chaptersListWidget->item(m_currentChapterIndex)->setText(
        QString("Глава %1: %2").arg(m_currentChapterIndex + 1).arg(newTitle)
        );

    if (CourseManager::saveCourseToBinary(
            m_course,
            AppSettings::getCourseBinaryPath(),
            AppSettings::ENCRYPTION_KEY)) {

        QMessageBox::information(
            this,
            "Успех",
            QString("Изменения в главе \"%1\" успешно сохранены!").arg(newTitle)
            );
    } else {
        QMessageBox::critical(
            this,
            "Ошибка",
            QString("Не удалось сохранить изменения в файл.\nПроверьте права доступа к каталогу:\n%1")
                .arg(QFileInfo(AppSettings::getCourseBinaryPath()).absolutePath())
            );
    }
}
