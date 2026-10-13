#include "TransactionDialog.h"
#include "../../database/DatabaseManager.h"
#include "../../utils/CurrencyFormatter.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QListView>
#include <QMessageBox>

TransactionDialog::TransactionDialog(QWidget* parent, const Transaction& tx, bool isEdit)
    : QDialog(parent), m_transaction(tx), m_isEdit(isEdit)
{
    setWindowTitle(m_isEdit ? "Edit Transaction" : "Add New Transaction");
    setMinimumWidth(440);
    setStyleSheet("QDialog { background-color: #0F172A; }");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(18);

    // Dialog Header
    QLabel* titleLabel = new QLabel(m_isEdit ? "Edit Transaction" : "New Transaction", this);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: 700; color: #F8FAFC; margin-bottom: 4px;");
    mainLayout->addWidget(titleLabel);

    // Segmented Type Selector (Expense vs Income)
    QHBoxLayout* typeLayout = new QHBoxLayout();
    typeLayout->setSpacing(8);

    m_expenseBtn = new QPushButton("💸 Expense", this);
    m_incomeBtn = new QPushButton("💰 Income", this);
    m_expenseBtn->setCheckable(true);
    m_incomeBtn->setCheckable(true);
    m_expenseBtn->setFixedHeight(38);
    m_incomeBtn->setFixedHeight(38);

    typeLayout->addWidget(m_expenseBtn);
    typeLayout->addWidget(m_incomeBtn);
    mainLayout->addLayout(typeLayout);

    // Form fields
    QFormLayout* form = new QFormLayout();
    form->setSpacing(14);
    form->setLabelAlignment(Qt::AlignLeft);

    auto makeLabel = [this](const QString& text) {
        QLabel* l = new QLabel(text, this);
        l->setStyleSheet("font-size: 13px; font-weight: 600; color: #94A3B8;");
        return l;
    };

    // Amount
    m_amountSpin = new QDoubleSpinBox(this);
    m_amountSpin->setRange(0.01, 10000000.0);
    m_amountSpin->setDecimals(2);
    m_amountSpin->setPrefix(CurrencyFormatter::instance().currencySymbol() + " ");
    m_amountSpin->setValue(m_isEdit ? m_transaction.amount : 0.0);
    m_amountSpin->setFixedHeight(40);
    m_amountSpin->setStyleSheet("font-size: 16px; font-weight: 700; color: #F8FAFC;");
    form->addRow(makeLabel("Amount *"), m_amountSpin);

    // Description
    m_descEdit = new QLineEdit(this);
    m_descEdit->setPlaceholderText("e.g. Grocery Store, Dinner, Salary");
    m_descEdit->setText(m_isEdit ? m_transaction.description : "");
    m_descEdit->setFixedHeight(38);
    form->addRow(makeLabel("Description *"), m_descEdit);

    // Category
    m_categoryCombo = new QComboBox(this);
    m_categoryCombo->setView(new QListView(m_categoryCombo));
    m_categoryCombo->setFixedHeight(38);
    form->addRow(makeLabel("Category *"), m_categoryCombo);

    // Date & Time
    m_dateTimeEdit = new QDateTimeEdit(this);
    m_dateTimeEdit->setCalendarPopup(true);
    m_dateTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
    m_dateTimeEdit->setDateTime(m_isEdit ? m_transaction.dateTime : QDateTime::currentDateTime());
    m_dateTimeEdit->setFixedHeight(38);
    form->addRow(makeLabel("Date & Time"), m_dateTimeEdit);

    // Payment Method
    m_paymentCombo = new QComboBox(this);
    m_paymentCombo->setView(new QListView(m_paymentCombo));
    m_paymentCombo->addItems({"Credit Card", "Debit Card", "Cash", "Bank Transfer", "PayPal", "Crypto", "Other"});
    if (m_isEdit && !m_transaction.paymentMethod.isEmpty()) {
        int idx = m_paymentCombo->findText(m_transaction.paymentMethod);
        if (idx >= 0) m_paymentCombo->setCurrentIndex(idx);
    }
    m_paymentCombo->setFixedHeight(38);
    form->addRow(makeLabel("Payment Method"), m_paymentCombo);

    // Notes
    m_notesEdit = new QTextEdit(this);
    m_notesEdit->setPlaceholderText("Additional notes, receipt info or tags (optional)...");
    m_notesEdit->setFixedHeight(70);
    m_notesEdit->setText(m_isEdit ? m_transaction.notes : "");
    form->addRow(makeLabel("Notes"), m_notesEdit);

    mainLayout->addLayout(form);

    // Buttons
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);
    btnLayout->addStretch();

    QPushButton* cancelBtn = new QPushButton("Cancel", this);
    cancelBtn->setFixedHeight(38);
    cancelBtn->setStyleSheet("background-color: #334155; color: #CBD5E1; border-radius: 8px; padding: 0 18px; font-weight: 600;");
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    QPushButton* saveBtn = new QPushButton(m_isEdit ? "Update Transaction" : "Add Transaction", this);
    saveBtn->setFixedHeight(38);
    saveBtn->setStyleSheet("background-color: #6366F1; color: #FFFFFF; border-radius: 8px; padding: 0 22px; font-weight: 700;");
    connect(saveBtn, &QPushButton::clicked, this, &TransactionDialog::validateAndAccept);

    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(saveBtn);
    mainLayout->addLayout(btnLayout);

    // Setup type
    QString initialType = m_isEdit ? m_transaction.type : "Expense";
    onTypeChanged(initialType);

    connect(m_expenseBtn, &QPushButton::clicked, this, [this]() { onTypeChanged("Expense"); });
    connect(m_incomeBtn, &QPushButton::clicked, this, [this]() { onTypeChanged("Income"); });
}

void TransactionDialog::onTypeChanged(const QString& type) {
    m_selectedType = type;
    if (type == "Expense") {
        m_expenseBtn->setChecked(true);
        m_incomeBtn->setChecked(false);
        m_expenseBtn->setStyleSheet("background-color: #F43F5E; color: white; font-weight: bold; border-radius: 8px; border: none;");
        m_incomeBtn->setStyleSheet("background-color: #1E293B; color: #94A3B8; font-weight: 600; border-radius: 8px; border: 1px solid #334155;");
    } else {
        m_expenseBtn->setChecked(false);
        m_incomeBtn->setChecked(true);
        m_expenseBtn->setStyleSheet("background-color: #1E293B; color: #94A3B8; font-weight: 600; border-radius: 8px; border: 1px solid #334155;");
        m_incomeBtn->setStyleSheet("background-color: #10B981; color: white; font-weight: bold; border-radius: 8px; border: none;");
    }
    populateCategories(type);
}

void TransactionDialog::populateCategories(const QString& type) {
    m_categoryCombo->clear();
    bool isIncome = (type == "Income");
    QList<Category> cats = DatabaseManager::instance().getCategories(isIncome);
    for (const auto& c : cats) {
        m_categoryCombo->addItem(QString("%1 %2").arg(c.icon, c.name), c.id);
    }

    if (m_isEdit && m_transaction.categoryId > 0) {
        int idx = m_categoryCombo->findData(m_transaction.categoryId);
        if (idx >= 0) m_categoryCombo->setCurrentIndex(idx);
    }
}

void TransactionDialog::validateAndAccept() {
    if (m_amountSpin->value() <= 0.0) {
        QMessageBox::warning(this, "Validation Error", "Please enter an amount greater than 0.");
        m_amountSpin->setFocus();
        return;
    }

    if (m_descEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Please enter a description for the transaction.");
        m_descEdit->setFocus();
        return;
    }

    if (m_categoryCombo->currentIndex() < 0) {
        QMessageBox::warning(this, "Validation Error", "Please select a category.");
        return;
    }

    m_transaction.type = m_selectedType;
    m_transaction.amount = m_amountSpin->value();
    m_transaction.description = m_descEdit->text().trimmed();
    m_transaction.categoryId = m_categoryCombo->currentData().toInt();
    m_transaction.categoryName = m_categoryCombo->currentText();
    m_transaction.dateTime = m_dateTimeEdit->dateTime();
    m_transaction.paymentMethod = m_paymentCombo->currentText();
    m_transaction.notes = m_notesEdit->toPlainText().trimmed();

    accept();
}

Transaction TransactionDialog::getTransaction() const {
    return m_transaction;
}
