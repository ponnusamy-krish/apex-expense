#include "BudgetDialog.h"
#include "../../database/DatabaseManager.h"
#include "../../utils/CurrencyFormatter.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QListView>
#include <QDate>
#include <QMessageBox>

BudgetDialog::BudgetDialog(QWidget* parent, int defaultMonth, int defaultYear, const Budget& initialBudget)
    : QDialog(parent)
{
    setWindowTitle(initialBudget.id > 0 ? "Edit Budget Limit" : "Set New Budget");
    setMinimumWidth(380);
    setStyleSheet("QDialog { background-color: #0F172A; }");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(18);

    QLabel* titleLabel = new QLabel(initialBudget.id > 0 ? "Edit Category Budget" : "New Category Budget", this);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: 700; color: #F8FAFC;");
    mainLayout->addWidget(titleLabel);

    QFormLayout* form = new QFormLayout();
    form->setSpacing(14);

    auto makeLabel = [this](const QString& text) {
        QLabel* l = new QLabel(text, this);
        l->setStyleSheet("font-size: 13px; font-weight: 600; color: #94A3B8;");
        return l;
    };

    // Category
    m_categoryCombo = new QComboBox(this);
    m_categoryCombo->setView(new QListView(m_categoryCombo));
    m_categoryCombo->setFixedHeight(38);
    QList<Category> expenseCats = DatabaseManager::instance().getCategories(false);
    for (const auto& c : expenseCats) {
        m_categoryCombo->addItem(QString("%1 %2").arg(c.icon, c.name), c.id);
    }
    if (initialBudget.categoryId > 0) {
        int idx = m_categoryCombo->findData(initialBudget.categoryId);
        if (idx >= 0) m_categoryCombo->setCurrentIndex(idx);
    }
    form->addRow(makeLabel("Category"), m_categoryCombo);

    // Limit Amount
    m_limitSpin = new QDoubleSpinBox(this);
    m_limitSpin->setRange(1.0, 1000000.0);
    m_limitSpin->setDecimals(2);
    m_limitSpin->setPrefix(CurrencyFormatter::instance().currencySymbol() + " ");
    m_limitSpin->setValue(initialBudget.monthlyLimit > 0 ? initialBudget.monthlyLimit : 300.0);
    m_limitSpin->setFixedHeight(38);
    form->addRow(makeLabel("Monthly Limit"), m_limitSpin);

    // Month
    m_monthCombo = new QComboBox(this);
    m_monthCombo->setView(new QListView(m_monthCombo));
    QStringList months = {"January", "February", "March", "April", "May", "June",
                          "July", "August", "September", "October", "November", "December"};
    for (int i = 0; i < months.size(); ++i) {
        m_monthCombo->addItem(months[i], i + 1);
    }
    int curM = (defaultMonth > 0) ? defaultMonth : QDate::currentDate().month();
    m_monthCombo->setCurrentIndex(curM - 1);
    m_monthCombo->setFixedHeight(38);
    form->addRow(makeLabel("Target Month"), m_monthCombo);

    // Year
    m_yearSpin = new QSpinBox(this);
    m_yearSpin->setRange(2020, 2040);
    int curY = (defaultYear > 0) ? defaultYear : QDate::currentDate().year();
    m_yearSpin->setValue(curY);
    m_yearSpin->setFixedHeight(38);
    form->addRow(makeLabel("Target Year"), m_yearSpin);

    mainLayout->addLayout(form);

    // Buttons
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);
    btnLayout->addStretch();

    QPushButton* cancelBtn = new QPushButton("Cancel", this);
    cancelBtn->setFixedHeight(38);
    cancelBtn->setStyleSheet("background-color: #334155; color: #CBD5E1; border-radius: 8px; padding: 0 16px; font-weight: 600;");
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    QPushButton* saveBtn = new QPushButton("Save Budget", this);
    saveBtn->setFixedHeight(38);
    saveBtn->setStyleSheet("background-color: #6366F1; color: #FFFFFF; border-radius: 8px; padding: 0 20px; font-weight: 700;");
    connect(saveBtn, &QPushButton::clicked, this, [this]() {
        if (m_limitSpin->value() <= 0.0) {
            QMessageBox::warning(this, "Validation", "Budget limit must be greater than zero.");
            return;
        }
        accept();
    });

    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(saveBtn);
    mainLayout->addLayout(btnLayout);
}

int BudgetDialog::getCategoryId() const {
    return m_categoryCombo->currentData().toInt();
}

double BudgetDialog::getLimit() const {
    return m_limitSpin->value();
}

int BudgetDialog::getMonth() const {
    return m_monthCombo->currentData().toInt();
}

int BudgetDialog::getYear() const {
    return m_yearSpin->value();
}
