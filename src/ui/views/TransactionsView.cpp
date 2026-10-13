#include "TransactionsView.h"
#include "../../database/DatabaseManager.h"
#include "../../utils/CurrencyFormatter.h"
#include "../../utils/CsvHandler.h"
#include "../dialogs/TransactionDialog.h"
#include "../components/ToastNotification.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QListView>
#include <QMessageBox>
#include <QFileDialog>
#include <QDate>

TransactionsView::TransactionsView(QWidget* parent) : QWidget(parent) {
    setupUi();
    connect(&DatabaseManager::instance(), &DatabaseManager::dataChanged, this, &TransactionsView::refreshData);
    refreshData();
}

void TransactionsView::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(16);

    // Header with Title & Action Buttons
    QHBoxLayout* topHeader = new QHBoxLayout();
    QVBoxLayout* titleLayout = new QVBoxLayout();

    QLabel* title = new QLabel("Transactions Ledger", this);
    title->setStyleSheet("font-size: 24px; font-weight: 800; color: #F8FAFC;");

    QLabel* subtitle = new QLabel("View, filter, edit, and export your complete transaction history", this);
    subtitle->setStyleSheet("font-size: 13px; color: #94A3B8; font-weight: 500;");

    titleLayout->addWidget(title);
    titleLayout->addWidget(subtitle);
    topHeader->addLayout(titleLayout);
    topHeader->addStretch();

    QPushButton* importBtn = new QPushButton("📂 Import CSV", this);
    importBtn->setFixedHeight(38);
    importBtn->setStyleSheet("background: #334155; color: #F8FAFC; border-radius: 8px; font-weight: 600; padding: 0 14px;");
    connect(importBtn, &QPushButton::clicked, this, &TransactionsView::onImportCsvClicked);

    QPushButton* exportBtn = new QPushButton("💾 Export CSV", this);
    exportBtn->setFixedHeight(38);
    exportBtn->setStyleSheet("background: #334155; color: #F8FAFC; border-radius: 8px; font-weight: 600; padding: 0 14px;");
    connect(exportBtn, &QPushButton::clicked, this, &TransactionsView::onExportCsvClicked);

    QPushButton* addBtn = new QPushButton("+ Add Transaction", this);
    addBtn->setFixedHeight(38);
    addBtn->setStyleSheet("background: #6366F1; color: #FFFFFF; border-radius: 8px; font-weight: 700; padding: 0 18px;");
    connect(addBtn, &QPushButton::clicked, this, [this]() { openAddDialog("Expense"); });

    topHeader->addWidget(importBtn);
    topHeader->addWidget(exportBtn);
    topHeader->addWidget(addBtn);
    mainLayout->addLayout(topHeader);

    // Filter Bar Frame
    QFrame* filterFrame = new QFrame(this);
    filterFrame->setStyleSheet("background: #1E293B; border: 1px solid #334155; border-radius: 10px;");
    QHBoxLayout* filterLayout = new QHBoxLayout(filterFrame);
    filterLayout->setContentsMargins(14, 12, 14, 12);
    filterLayout->setSpacing(10);

    // Search query box
    m_searchEdit = new QLineEdit(filterFrame);
    m_searchEdit->setPlaceholderText("🔍 Search description, notes...");
    m_searchEdit->setFixedHeight(36);
    m_searchEdit->setStyleSheet("background: #0F172A; border: 1px solid #334155; border-radius: 6px; padding: 0 10px; color: #F8FAFC;");
    connect(m_searchEdit, &QLineEdit::textChanged, this, &TransactionsView::onSearchChanged);
    filterLayout->addWidget(m_searchEdit, 2);

    // Date range preset
    m_datePresetCombo = new QComboBox(filterFrame);
    m_datePresetCombo->setView(new QListView(m_datePresetCombo));
    m_datePresetCombo->addItems({"All Time", "Today", "This Week", "This Month", "Last Month", "This Year", "Custom Range"});
    m_datePresetCombo->setCurrentIndex(0); // All time
    m_datePresetCombo->setFixedHeight(36);
    connect(m_datePresetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TransactionsView::onDateFilterChanged);
    filterLayout->addWidget(m_datePresetCombo, 1);

    m_startDateEdit = new QDateEdit(QDate::currentDate().addMonths(-1), filterFrame);
    m_startDateEdit->setCalendarPopup(true);
    m_startDateEdit->setFixedHeight(36);
    m_startDateEdit->setVisible(false);
    connect(m_startDateEdit, &QDateEdit::dateChanged, this, &TransactionsView::onSearchChanged);
    filterLayout->addWidget(m_startDateEdit, 1);

    m_endDateEdit = new QDateEdit(QDate::currentDate(), filterFrame);
    m_endDateEdit->setCalendarPopup(true);
    m_endDateEdit->setFixedHeight(36);
    m_endDateEdit->setVisible(false);
    connect(m_endDateEdit, &QDateEdit::dateChanged, this, &TransactionsView::onSearchChanged);
    filterLayout->addWidget(m_endDateEdit, 1);

    // Type filter
    m_typeCombo = new QComboBox(filterFrame);
    m_typeCombo->setView(new QListView(m_typeCombo));
    m_typeCombo->addItems({"All Types", "Expense", "Income"});
    m_typeCombo->setFixedHeight(36);
    connect(m_typeCombo, &QComboBox::currentTextChanged, this, &TransactionsView::onSearchChanged);
    filterLayout->addWidget(m_typeCombo, 1);

    // Category filter
    m_categoryCombo = new QComboBox(filterFrame);
    m_categoryCombo->setView(new QListView(m_categoryCombo));
    m_categoryCombo->setFixedHeight(36);
    connect(m_categoryCombo, &QComboBox::currentTextChanged, this, &TransactionsView::onSearchChanged);
    filterLayout->addWidget(m_categoryCombo, 1);

    // Payment method filter
    m_paymentCombo = new QComboBox(filterFrame);
    m_paymentCombo->setView(new QListView(m_paymentCombo));
    m_paymentCombo->addItems({"All Methods", "Credit Card", "Debit Card", "Cash", "Bank Transfer", "PayPal", "Crypto", "Other"});
    m_paymentCombo->setFixedHeight(36);
    connect(m_paymentCombo, &QComboBox::currentTextChanged, this, &TransactionsView::onSearchChanged);
    filterLayout->addWidget(m_paymentCombo, 1);

    mainLayout->addWidget(filterFrame);

    // Transactions Table
    m_table = new QTableWidget(this);
    m_table->setColumnCount(7);
    m_table->setHorizontalHeaderLabels({"ID", "Date & Time", "Type", "Category", "Description", "Payment Method", "Amount"});
    m_table->setColumnHidden(0, true); // Hide ID column
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setAlternatingRowColors(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->setStyleSheet(
        "QTableWidget {"
        "  background-color: #1E293B;"
        "  border: 1px solid #334155;"
        "  border-radius: 10px;"
        "  gridline-color: #334155;"
        "  color: #F8FAFC;"
        "}"
        "QTableWidget::item {"
        "  padding: 8px 10px;"
        "  border-bottom: 1px solid #334155;"
        "}"
        "QTableWidget::item:selected {"
        "  background-color: #312E81;"
        "  color: #FFFFFF;"
        "}"
        "QTableWidget::item:alternate {"
        "  background-color: #162032;"
        "}"
        "QHeaderView::section {"
        "  background-color: #0F172A;"
        "  color: #94A3B8;"
        "  font-weight: 700;"
        "  font-size: 12px;"
        "  padding: 10px;"
        "  border: none;"
        "  border-bottom: 2px solid #334155;"
        "}"
    );
    connect(m_table, &QTableWidget::cellDoubleClicked, this, &TransactionsView::onTableRowDoubleClicked);
    mainLayout->addWidget(m_table);

    // Bottom Action & Summary Bar
    QHBoxLayout* bottomLayout = new QHBoxLayout();

    m_summaryLabel = new QLabel(this);
    m_summaryLabel->setStyleSheet("font-size: 13px; color: #94A3B8; font-weight: 600;");
    bottomLayout->addWidget(m_summaryLabel);
    bottomLayout->addStretch();

    QPushButton* editBtn = new QPushButton("✏️ Edit Selected", this);
    editBtn->setFixedHeight(36);
    editBtn->setStyleSheet("background: #334155; color: #F8FAFC; border-radius: 6px; font-weight: 600; padding: 0 14px;");
    connect(editBtn, &QPushButton::clicked, this, &TransactionsView::onEditClicked);

    QPushButton* deleteBtn = new QPushButton("🗑️ Delete Selected", this);
    deleteBtn->setFixedHeight(36);
    deleteBtn->setStyleSheet("background: #881337; color: #FDA4AF; border-radius: 6px; font-weight: 600; padding: 0 14px; border: 1px solid #E11D48;");
    connect(deleteBtn, &QPushButton::clicked, this, &TransactionsView::onDeleteClicked);

    bottomLayout->addWidget(editBtn);
    bottomLayout->addWidget(deleteBtn);
    mainLayout->addLayout(bottomLayout);

    populateCategories();
}

void TransactionsView::populateCategories() {
    m_categoryCombo->clear();
    m_categoryCombo->addItem("All Categories", -1);
    QList<Category> cats = DatabaseManager::instance().getAllCategories();
    for (const auto& c : cats) {
        m_categoryCombo->addItem(QString("%1 %2").arg(c.icon, c.name), c.id);
    }
}

void TransactionsView::onDateFilterChanged(int index) {
    bool isCustom = (index == 6); // "Custom Range"
    m_startDateEdit->setVisible(isCustom);
    m_endDateEdit->setVisible(isCustom);
    onSearchChanged();
}

void TransactionsView::onSearchChanged() {
    updateTable();
}

void TransactionsView::refreshData() {
    populateCategories();
    updateTable();
}

void TransactionsView::updateTable() {
    DatabaseManager& db = DatabaseManager::instance();
    CurrencyFormatter& cf = CurrencyFormatter::instance();

    QDate today = QDate::currentDate();
    QDateTime fromDt;
    QDateTime toDt;

    int dateIdx = m_datePresetCombo->currentIndex();
    if (dateIdx == 1) { // Today
        fromDt = today.startOfDay();
        toDt = today.endOfDay();
    } else if (dateIdx == 2) { // This Week
        fromDt = today.addDays(-(today.dayOfWeek() - 1)).startOfDay();
        toDt = today.endOfDay();
    } else if (dateIdx == 3) { // This Month
        fromDt = QDate(today.year(), today.month(), 1).startOfDay();
        toDt = QDate(today.year(), today.month(), today.daysInMonth()).endOfDay();
    } else if (dateIdx == 4) { // Last Month
        QDate lastMonth = today.addMonths(-1);
        fromDt = QDate(lastMonth.year(), lastMonth.month(), 1).startOfDay();
        toDt = QDate(lastMonth.year(), lastMonth.month(), lastMonth.daysInMonth()).endOfDay();
    } else if (dateIdx == 5) { // This Year
        fromDt = QDate(today.year(), 1, 1).startOfDay();
        toDt = QDate(today.year(), 12, 31).endOfDay();
    } else if (dateIdx == 6) { // Custom Range
        fromDt = m_startDateEdit->date().startOfDay();
        toDt = m_endDateEdit->date().endOfDay();
    }

    int catId = m_categoryCombo->currentData().toInt();
    QString type = m_typeCombo->currentText();
    if (type.startsWith("All")) type = "All";
    QString paymentMethod = m_paymentCombo->currentText();
    if (paymentMethod.startsWith("All")) paymentMethod = "All";
    QString searchQuery = m_searchEdit->text();

    m_currentTransactions = db.getFilteredTransactions(fromDt, toDt, catId, type, paymentMethod, searchQuery);

    m_table->setRowCount(m_currentTransactions.size());

    double totalIncome = 0.0, totalExpense = 0.0;

    for (int row = 0; row < m_currentTransactions.size(); ++row) {
        const auto& t = m_currentTransactions[row];

        if (t.type == "Income") {
            totalIncome += t.amount;
        } else {
            totalExpense += t.amount;
        }

        // ID (hidden)
        m_table->setItem(row, 0, new QTableWidgetItem(QString::number(t.id)));

        // Date
        QTableWidgetItem* dtItem = new QTableWidgetItem(t.dateTime.toString("yyyy-MM-dd HH:mm"));
        dtItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        m_table->setItem(row, 1, dtItem);

        // Type
        QTableWidgetItem* typeItem = new QTableWidgetItem(t.type);
        typeItem->setTextAlignment(Qt::AlignCenter);
        if (t.type == "Income") {
            typeItem->setForeground(QColor("#10B981"));
        } else {
            typeItem->setForeground(QColor("#F43F5E"));
        }
        m_table->setItem(row, 2, typeItem);

        // Category
        QTableWidgetItem* catItem = new QTableWidgetItem(QString("%1 %2").arg(t.categoryIcon, t.categoryName));
        catItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        m_table->setItem(row, 3, catItem);

        // Description
        QString descText = t.description;
        if (!t.notes.isEmpty()) {
            descText += QString(" (%1)").arg(t.notes);
        }
        QTableWidgetItem* descItem = new QTableWidgetItem(descText);
        descItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        m_table->setItem(row, 4, descItem);

        // Payment Method
        QTableWidgetItem* payItem = new QTableWidgetItem(t.paymentMethod);
        payItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        m_table->setItem(row, 5, payItem);

        // Amount
        QTableWidgetItem* amountItem = new QTableWidgetItem(cf.formatSigned(t.amount, t.type));
        amountItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
        amountItem->setFont(QFont("sans-serif", 10, QFont::Bold));
        if (t.type == "Income") {
            amountItem->setForeground(QColor("#10B981"));
        } else {
            amountItem->setForeground(QColor("#F43F5E"));
        }
        m_table->setItem(row, 6, amountItem);
    }

    m_summaryLabel->setText(QString(
        "Showing %1 transactions  |  Income: <span style='color: #10B981;'>%2</span>  |  Expense: <span style='color: #F43F5E;'>%3</span>  |  Net: <b>%4</b>"
    ).arg(QString::number(m_currentTransactions.size()))
     .arg(cf.format(totalIncome))
     .arg(cf.format(totalExpense))
     .arg(cf.format(totalIncome - totalExpense)));
}

void TransactionsView::openAddDialog(const QString& defaultType) {
    Transaction t;
    t.type = defaultType;
    TransactionDialog dialog(this, t, false);
    if (dialog.exec() == QDialog::Accepted) {
        Transaction newTx = dialog.getTransaction();
        if (DatabaseManager::instance().addTransaction(newTx) > 0) {
            ToastNotification::showMessage(window(), "Transaction added successfully!", ToastNotification::Success);
        }
    }
}

void TransactionsView::onTableRowDoubleClicked(int row, int /*column*/) {
    if (row < 0 || row >= m_currentTransactions.size()) return;
    int id = m_table->item(row, 0)->text().toInt();
    Transaction t = DatabaseManager::instance().getTransactionById(id);

    TransactionDialog dialog(this, t, true);
    if (dialog.exec() == QDialog::Accepted) {
        Transaction updated = dialog.getTransaction();
        if (DatabaseManager::instance().updateTransaction(updated)) {
            ToastNotification::showMessage(window(), "Transaction updated!", ToastNotification::Success);
        }
    }
}

void TransactionsView::onEditClicked() {
    int curRow = m_table->currentRow();
    if (curRow < 0) {
        QMessageBox::information(this, "Select Item", "Please select a transaction to edit.");
        return;
    }
    onTableRowDoubleClicked(curRow, 0);
}

void TransactionsView::onDeleteClicked() {
    int curRow = m_table->currentRow();
    if (curRow < 0) {
        QMessageBox::information(this, "Select Item", "Please select a transaction to delete.");
        return;
    }

    int id = m_table->item(curRow, 0)->text().toInt();
    QString desc = m_table->item(curRow, 4)->text();
    QString amt = m_table->item(curRow, 6)->text();

    auto reply = QMessageBox::question(
        this, "Confirm Deletion",
        QString("Are you sure you want to delete this transaction?\n\n\"%1\" (%2)").arg(desc, amt),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        if (DatabaseManager::instance().deleteTransaction(id)) {
            ToastNotification::showMessage(window(), "Transaction deleted.", ToastNotification::Info);
        }
    }
}

void TransactionsView::onExportCsvClicked() {
    QString fileName = QFileDialog::getSaveFileName(
        this, "Export Transactions to CSV",
        QString("expenses_%1.csv").arg(QDate::currentDate().toString("yyyyMMdd")),
        "CSV Files (*.csv)"
    );

    if (fileName.isEmpty()) return;

    if (CsvHandler::exportTransactions(fileName, m_currentTransactions)) {
        ToastNotification::showMessage(window(), QString("Exported %1 rows to CSV!").arg(m_currentTransactions.size()), ToastNotification::Success);
    } else {
        QMessageBox::critical(this, "Export Failed", "Failed to write CSV file.");
    }
}

void TransactionsView::onImportCsvClicked() {
    QString fileName = QFileDialog::getOpenFileName(
        this, "Import Transactions from CSV",
        QString(),
        "CSV Files (*.csv)"
    );

    if (fileName.isEmpty()) return;

    CsvImportResult result = CsvHandler::importTransactions(fileName);
    if (result.success) {
        ToastNotification::showMessage(
            window(),
            QString("Successfully imported %1 transactions! (%2 skipped)").arg(result.importedCount).arg(result.skippedCount),
            ToastNotification::Success
        );
    } else {
        QMessageBox::warning(this, "Import Failed", result.errorMessage.isEmpty() ? "No valid transactions could be imported." : result.errorMessage);
    }
}
