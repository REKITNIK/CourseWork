#ifndef ADMINWINDOW_H
#define ADMINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableView>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QTextEdit>
#include <QLabel>
#include <QSplitter>
#include <QMessageBox>
#include <QFileDialog>
#include <QSqlTableModel>
#include <QSortFilterProxyModel>
#include <QHeaderView>
#include <QFile>
#include <QTextStream>
#include <QSqlQuery>
#include <QFileDialog>
#include <QDateTime>

#include "models/Structures.h"

/**
 * @brief Главное окно администратора.
 * Предоставляет интерфейс для управления студентами и редактирования курса.
 * Содержит две вкладки: просмотр студентов и редактор курса.
 */
class AdminWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор окна администратора.
     * @param parent Родительский виджет
     */
    explicit AdminWindow(QWidget *parent = nullptr);

private slots:
    /**
     * @brief Обработчик изменения текста поиска студентов.
     * @param text Новый текст для поиска
     */
    void onSearchTextChanged(const QString& text);
    
    /**
     * @brief Обработчик нажатия кнопки генерации отчета.
     */
    void onGenerateReportClicked();
    
    /**
     * @brief Обработчик изменения выбранной главы в редакторе.
     */
    void onChapterSelectionChanged();
    
    /**
     * @brief Обработчик сохранения изменений в курсе.
     */
    void onSaveChangesClicked();

private:
    /**
     * @brief Настраивает пользовательский интерфейс.
     */
    void setupUI();
    
    /**
     * @brief Настраивает вкладку со студентами.
     */
    void setupStudentsTab();
    
    /**
     * @brief Настраивает вкладку редактора курса.
     */
    void setupCourseEditorTab();
    
    /**
     * @brief Загружает данные курса из файла.
     */
    void loadCourseData();
    
    /**
     * @brief Обновляет содержимое выбранной главы.
     */
    void updateChapterContent();
    
    // Основные виджеты
    QTabWidget* m_tabWidget;
    
    // Виджеты вкладки студентов
    QWidget* m_studentsTab;
    QTableView* m_studentsTableView;
    QLineEdit* m_searchLineEdit;
    QPushButton* m_reportButton;
    QSqlTableModel* m_usersModel;
    QSortFilterProxyModel* m_proxyModel;
    
    // Виджеты вкладки редактора курса
    QWidget* m_courseEditorTab;
    QListWidget* m_chaptersListWidget;
    QLineEdit* m_chapterTitleEdit;
    QTextEdit* m_chapterContentEdit;
    QPushButton* m_saveChangesButton;
    
    // Данные курса
    Course m_course;
    int m_currentChapterIndex;
};

#endif // ADMINWINDOW_H
