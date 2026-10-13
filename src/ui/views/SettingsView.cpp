#include "SettingsView.h"
#include "../../database/DatabaseManager.h"
#include "../../utils/CurrencyFormatter.h"
#include "../../utils/JsonHandler.h"
#include "../dialogs/CategoryDialog.h"
#include "../components/ToastNotification.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFrame>
#include <QScrollArea>
#include <QListView>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QDate>

SettingsView::SettingsView(QWidget* parent) : QWidget(parent) {
    setupUi();
    connect(&DatabaseManager::instance(), &DatabaseManager::dataChanged, this, &SettingsView::refreshData);
    refreshData();
}

void SettingsView::setupUi() {
    QVBoxLayout* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);

    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("QScrollArea { border: none; background: transparent; }");

    QWidget* scrollContent = new QWidget(scrollArea);
    scrollContent->setStyleSheet("background: transparent;");
    QVBoxLayout* mainLayout = new QVBoxLayout(scrollContent);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(20);

    // Header
    QVBoxLayout* titleLayout = new QVBoxLayout();
    QLabel* title = new QLabel("Settings & Data Management", scrollContent);
    title->setStyleSheet("font-size: 24px; font-weight: 800; color: #F8FAFC;");

    QLabel* subtitle = new QLabel("Manage currency, JSON backup/restore, SQLite database, and custom categories", scrollContent);
    subtitle->setStyleSheet("font-size: 13px; color: #94A3B8; font-weight: 500;");

    titleLayout->addWidget(title);
    titleLayout->addWidget(subtitle);
    mainLayout->addLayout(titleLayout);

    // Preferences & Database Card
    QFrame* prefFrame = new QFrame(scrollContent);
    prefFrame->setObjectName("prefCard");
    prefFrame->setStyleSheet("QFrame#prefCard { background: #1E293B; border: 1px solid #334155; border-radius: 12px; }");
    QVBoxLayout* prefLayout = new QVBoxLayout(prefFrame);
    prefLayout->setContentsMargins(18, 18, 18, 18);
    prefLayout->setSpacing(14);

    QLabel* prefTitle = new QLabel("General Preferences & Storage", prefFrame);
    prefTitle->setStyleSheet("font-size: 16px; font-weight: 700; color: #F8FAFC; border: none;");
    prefLayout->addWidget(prefTitle);

    QHBoxLayout* row1 = new QHBoxLayout();
    QLabel* currLabel = new QLabel("Display Currency Symbol:", prefFrame);
    currLabel->setStyleSheet("font-size: 14px; color: #94A3B8; font-weight: 600; border: none;");

    m_currencyCombo = new QComboBox(prefFrame);
    m_currencyCombo->setObjectName("currencyCombo");
    m_currencyCombo->setView(new QListView(m_currencyCombo));
    m_currencyCombo->addItems({"$ (USD / CAD / AUD)", "€ (EUR)", "£ (GBP)", "¥ (JPY / CNY)", "₹ (INR)", "CHF (CHF)", "kr (SEK/NOK)"});
    m_currencyCombo->setFixedHeight(38);
    m_currencyCombo->setMinimumWidth(220);

    QString curSym = CurrencyFormatter::instance().currencySymbol();
    for (int i = 0; i < m_currencyCombo->count(); ++i) {
        if (m_currencyCombo->itemText(i).startsWith(curSym)) {
            m_currencyCombo->setCurrentIndex(i);
            break;
        }
    }
    connect(m_currencyCombo, &QComboBox::currentTextChanged, this, &SettingsView::onCurrencyChanged);

    row1->addWidget(currLabel);
    row1->addWidget(m_currencyCombo);
    row1->addStretch();
    prefLayout->addLayout(row1);

    QHBoxLayout* row2 = new QHBoxLayout();
    QLabel* dbLabel = new QLabel("SQLite Database File:", prefFrame);
    dbLabel->setStyleSheet("font-size: 14px; color: #94A3B8; font-weight: 600; border: none;");

    m_dbLocationLabel = new QLabel(DatabaseManager::instance().databasePath(), prefFrame);
    m_dbLocationLabel->setStyleSheet("font-size: 13px; color: #6366F1; font-weight: 600; background: #0F172A; padding: 6px 12px; border-radius: 6px; border: 1px solid #334155;");

    row2->addWidget(dbLabel);
    row2->addWidget(m_dbLocationLabel);
    row2->addStretch();
    prefLayout->addLayout(row2);

    mainLayout->addWidget(prefFrame);

    // Data Backup & Maintenance Card
    QFrame* backupFrame = new QFrame(scrollContent);
    backupFrame->setObjectName("backupCard");
    backupFrame->setStyleSheet("QFrame#backupCard { background: #1E293B; border: 1px solid #334155; border-radius: 12px; }");
    QVBoxLayout* backupLayout = new QVBoxLayout(backupFrame);
    backupLayout->setContentsMargins(18, 18, 18, 18);
    backupLayout->setSpacing(14);

    QLabel* backupTitle = new QLabel("Backup & Migration (JSON / Demo)", backupFrame);
    backupTitle->setStyleSheet("font-size: 16px; font-weight: 700; color: #F8FAFC; border: none;");
    backupLayout->addWidget(backupTitle);

    QHBoxLayout* actionsLayout = new QHBoxLayout();
    actionsLayout->setSpacing(12);

    QPushButton* exportJsonBtn = new QPushButton("📦 Export Backup (JSON)", backupFrame);
    exportJsonBtn->setFixedHeight(38);
    exportJsonBtn->setStyleSheet("background: #6366F1; color: white; border-radius: 8px; font-weight: 700; padding: 0 16px;");
    connect(exportJsonBtn, &QPushButton::clicked, this, &SettingsView::onExportJsonBackup);

    QPushButton* importJsonBtn = new QPushButton("📥 Restore Backup (JSON)", backupFrame);
    importJsonBtn->setFixedHeight(38);
    importJsonBtn->setStyleSheet("background: #334155; color: #F8FAFC; border-radius: 8px; font-weight: 600; padding: 0 16px;");
    connect(importJsonBtn, &QPushButton::clicked, this, &SettingsView::onImportJsonBackup);

    QPushButton* sampleDataBtn = new QPushButton("✨ Load Sample Data", backupFrame);
    sampleDataBtn->setFixedHeight(38);
    sampleDataBtn->setStyleSheet("background: #059669; color: white; border-radius: 8px; font-weight: 700; padding: 0 16px;");
    connect(sampleDataBtn, &QPushButton::clicked, this, &SettingsView::onLoadSampleData);

    QPushButton* clearDataBtn = new QPushButton("⚠️ Clear All Data", backupFrame);
    clearDataBtn->setFixedHeight(38);
    clearDataBtn->setStyleSheet("background: #881337; color: #FDA4AF; border-radius: 8px; font-weight: 700; padding: 0 16px; border: 1px solid #E11D48;");
    connect(clearDataBtn, &QPushButton::clicked, this, &SettingsView::onClearAllData);

    actionsLayout->addWidget(exportJsonBtn);
    actionsLayout->addWidget(importJsonBtn);
    actionsLayout->addWidget(sampleDataBtn);
    actionsLayout->addWidget(clearDataBtn);
    actionsLayout->addStretch();
    backupLayout->addLayout(actionsLayout);

    mainLayout->addWidget(backupFrame);

    // Categories Management Card
    QFrame* catFrame = new QFrame(scrollContent);
    catFrame->setObjectName("catCard");
    catFrame->setStyleSheet("QFrame#catCard { background: #1E293B; border: 1px solid #334155; border-radius: 12px; }");
    QVBoxLayout* catLayout = new QVBoxLayout(catFrame);
    catLayout->setContentsMargins(18, 18, 18, 18);
    catLayout->setSpacing(12);

    QHBoxLayout* catHeader = new QHBoxLayout();
    QLabel* catTitle = new QLabel("Categories Configuration", catFrame);
    catTitle->setStyleSheet("font-size: 16px; font-weight: 700; color: #F8FAFC; border: none;");
    catHeader->addWidget(catTitle);
    catHeader->addStretch();

    QPushButton* addCatBtn = new QPushButton("+ Add Custom Category", catFrame);
    addCatBtn->setFixedHeight(34);
    addCatBtn->setStyleSheet("background: #6366F1; color: white; border-radius: 6px; font-weight: 700; padding: 0 14px;");
    connect(addCatBtn, &QPushButton::clicked, this, &SettingsView::onAddCategory);
    catHeader->addWidget(addCatBtn);

    catLayout->addLayout(catHeader);

    m_categoriesTable = new QTableWidget(catFrame);
    m_categoriesTable->setColumnCount(5);
    m_categoriesTable->setHorizontalHeaderLabels({"ID", "Icon", "Name", "Type", "Color Code"});
    m_categoriesTable->setColumnHidden(0, true);
    m_categoriesTable->horizontalHeader()->setStretchLastSection(true);
    m_categoriesTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_categoriesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_categoriesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_categoriesTable->verticalHeader()->setVisible(false);
    m_categoriesTable->setStyleSheet(
        "QTableWidget { background-color: #0F172A; border: 1px solid #334155; border-radius: 8px; color: #F8FAFC; }"
        "QTableWidget::item { padding: 6px; border-bottom: 1px solid #334155; }"
        "QHeaderView::section { background-color: #1E293B; color: #94A3B8; font-weight: bold; padding: 8px; border: none; border-bottom: 2px solid #334155; }"
    );
    m_categoriesTable->setMinimumHeight(180);
    catLayout->addWidget(m_categoriesTable);

    QHBoxLayout* catBottomLayout = new QHBoxLayout();
    catBottomLayout->addStretch();
    QPushButton* delCatBtn = new QPushButton("Delete Selected Category", catFrame);
    delCatBtn->setFixedHeight(32);
    delCatBtn->setStyleSheet("background: #334155; color: #FDA4AF; border-radius: 6px; font-weight: 600; padding: 0 12px;");
    connect(delCatBtn, &QPushButton::clicked, this, &SettingsView::onDeleteCategory);
    catBottomLayout->addWidget(delCatBtn);
    catLayout->addLayout(catBottomLayout);

    mainLayout->addWidget(catFrame);
    rootLayout->addWidget(scrollArea);
}

void SettingsView::refreshData() {
    updateCategoriesTable();
}

void SettingsView::updateCategoriesTable() {
    QList<Category> cats = DatabaseManager::instance().getAllCategories();
    m_categoriesTable->setRowCount(cats.size());

    for (int row = 0; row < cats.size(); ++row) {
        const auto& c = cats[row];

        m_categoriesTable->setItem(row, 0, new QTableWidgetItem(QString::number(c.id)));

        QTableWidgetItem* icItem = new QTableWidgetItem(c.icon);
        icItem->setTextAlignment(Qt::AlignCenter);
        m_categoriesTable->setItem(row, 1, icItem);

        QTableWidgetItem* nameItem = new QTableWidgetItem(c.name);
        nameItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        m_categoriesTable->setItem(row, 2, nameItem);

        QTableWidgetItem* typeItem = new QTableWidgetItem(c.isIncome ? "Income" : "Expense");
        typeItem->setTextAlignment(Qt::AlignCenter);
        typeItem->setForeground(QColor(c.isIncome ? "#10B981" : "#F43F5E"));
        m_categoriesTable->setItem(row, 3, typeItem);

        QTableWidgetItem* colItem = new QTableWidgetItem(c.color);
        colItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        m_categoriesTable->setItem(row, 4, colItem);
    }
}

void SettingsView::onCurrencyChanged(const QString& text) {
    QString symbol = "$";
    if (text.startsWith("€")) symbol = "€";
    else if (text.startsWith("£")) symbol = "£";
    else if (text.startsWith("¥")) symbol = "¥";
    else if (text.startsWith("₹")) symbol = "₹";
    else if (text.startsWith("CHF")) symbol = "CHF";
    else if (text.startsWith("kr")) symbol = "kr";

    CurrencyFormatter::instance().setCurrencySymbol(symbol);
    DatabaseManager::instance().setSetting("currency", symbol);
    emit DatabaseManager::instance().dataChanged();
    ToastNotification::showMessage(window(), QString("Currency set to %1").arg(symbol), ToastNotification::Success);
}

void SettingsView::onExportJsonBackup() {
    QString fileName = QFileDialog::getSaveFileName(
        this, "Save JSON Backup",
        QString("ExpenseBackup_%1.json").arg(QDate::currentDate().toString("yyyyMMdd")),
        "JSON Files (*.json)"
    );

    if (fileName.isEmpty()) return;

    if (JsonHandler::exportBackup(fileName)) {
        ToastNotification::showMessage(window(), "Backup exported successfully!", ToastNotification::Success);
    } else {
        QMessageBox::critical(this, "Export Error", "Failed to write backup file.");
    }
}

void SettingsView::onImportJsonBackup() {
    QString fileName = QFileDialog::getOpenFileName(
        this, "Select JSON Backup File",
        QString(),
        "JSON Files (*.json)"
    );

    if (fileName.isEmpty()) return;

    auto reply = QMessageBox::question(
        this, "Restore Mode",
        "Do you want to MERGE with current data, or REPLACE current data?\n\nClick 'Yes' to Merge, 'No' to Replace.",
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel
    );

    if (reply == QMessageBox::Cancel) return;
    bool merge = (reply == QMessageBox::Yes);

    JsonImportResult result = JsonHandler::importBackup(fileName, merge);
    if (result.success) {
        emit DatabaseManager::instance().dataChanged();
        ToastNotification::showMessage(
            window(),
            QString("Backup restored! (%1 tx, %2 budgets, %3 categories)").arg(result.importedTransactions).arg(result.importedBudgets).arg(result.importedCategories),
            ToastNotification::Success
        );
    } else {
        QMessageBox::critical(this, "Restore Failed", result.errorMessage);
    }
}

void SettingsView::onLoadSampleData() {
    auto reply = QMessageBox::question(
        this, "Load Sample Data",
        "This will load realistic sample expenses, incomes, and budgets for testing.\n\nContinue?",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        DatabaseManager::instance().loadSampleData();
        ToastNotification::showMessage(window(), "Sample data loaded successfully!", ToastNotification::Success);
    }
}

void SettingsView::onClearAllData() {
    auto reply = QMessageBox::warning(
        this, "Confirm Reset",
        "Are you sure you want to delete all transactions and budgets? This action cannot be undone.",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        if (DatabaseManager::instance().clearAllTransactionsAndBudgets()) {
            ToastNotification::showMessage(window(), "All records cleared.", ToastNotification::Info);
        }
    }
}

void SettingsView::onAddCategory() {
    CategoryDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        Category c = dlg.getCategory();
        if (DatabaseManager::instance().addCategory(c) > 0) {
            ToastNotification::showMessage(window(), "Category added!", ToastNotification::Success);
        }
    }
}

void SettingsView::onDeleteCategory() {
    int curRow = m_categoriesTable->currentRow();
    if (curRow < 0) {
        QMessageBox::information(this, "Select Item", "Please select a category to delete.");
        return;
    }

    int id = m_categoriesTable->item(curRow, 0)->text().toInt();
    QString name = m_categoriesTable->item(curRow, 2)->text();

    auto reply = QMessageBox::question(
        this, "Delete Category",
        QString("Are you sure you want to delete category '%1'?\n\n(Note: Categories with linked transactions cannot be deleted)").arg(name),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        if (DatabaseManager::instance().deleteCategory(id)) {
            ToastNotification::showMessage(window(), "Category deleted.", ToastNotification::Info);
        } else {
            QMessageBox::warning(this, "Cannot Delete", "This category has transactions linked to it or is a system category.");
        }
    }
}
