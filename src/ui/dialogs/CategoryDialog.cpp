#include "CategoryDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QListView>
#include <QMessageBox>

CategoryDialog::CategoryDialog(QWidget* parent, const Category& initialCat)
    : QDialog(parent), m_category(initialCat)
{
    setWindowTitle(initialCat.id > 0 ? "Edit Category" : "Add Custom Category");
    setMinimumWidth(380);
    setStyleSheet("QDialog { background-color: #0F172A; }");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(18);

    QLabel* titleLabel = new QLabel(initialCat.id > 0 ? "Edit Category" : "New Category", this);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: 700; color: #F8FAFC;");
    mainLayout->addWidget(titleLabel);

    QFormLayout* form = new QFormLayout();
    form->setSpacing(14);

    auto makeLabel = [this](const QString& text) {
        QLabel* l = new QLabel(text, this);
        l->setStyleSheet("font-size: 13px; font-weight: 600; color: #94A3B8;");
        return l;
    };

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setText(initialCat.name);
    m_nameEdit->setPlaceholderText("e.g. Pet Care, Subscriptions, Freelance");
    m_nameEdit->setFixedHeight(38);
    form->addRow(makeLabel("Category Name *"), m_nameEdit);

    m_typeCombo = new QComboBox(this);
    m_typeCombo->setView(new QListView(m_typeCombo));
    m_typeCombo->addItem("Expense", false);
    m_typeCombo->addItem("Income", true);
    m_typeCombo->setCurrentIndex(initialCat.isIncome ? 1 : 0);
    m_typeCombo->setFixedHeight(38);
    form->addRow(makeLabel("Category Type"), m_typeCombo);

    m_iconCombo = new QComboBox(this);
    m_iconCombo->setView(new QListView(m_iconCombo));
    QStringList icons = {"📁", "🍔", "🛒", "🏠", "⚡", "🚗", "🛍️", "🎬", "💊", "📚", "✈️",
                         "🎮", "🐶", "☕", "🏋️", "🎁", "💼", "💻", "📈", "💰", "🧾", "📦"};
    for (const auto& ic : icons) {
        m_iconCombo->addItem(ic);
    }
    int icIdx = m_iconCombo->findText(initialCat.icon);
    if (icIdx >= 0) m_iconCombo->setCurrentIndex(icIdx);
    m_iconCombo->setFixedHeight(38);
    form->addRow(makeLabel("Icon Emoji"), m_iconCombo);

    m_colorCombo = new QComboBox(this);
    m_colorCombo->setView(new QListView(m_colorCombo));
    struct ColorOption { QString name; QString hex; };
    QList<ColorOption> colors = {
        {"Indigo", "#6366F1"}, {"Emerald", "#10B981"}, {"Rose", "#F43F5E"},
        {"Amber", "#F59E0B"}, {"Sky Blue", "#0EA5E9"}, {"Purple", "#8B5CF6"},
        {"Pink", "#EC4899"}, {"Teal", "#14B8A6"}, {"Slate", "#64748B"}
    };
    for (const auto& opt : colors) {
        m_colorCombo->addItem(opt.name, opt.hex);
    }
    for (int i = 0; i < m_colorCombo->count(); ++i) {
        if (m_colorCombo->itemData(i).toString().toLower() == initialCat.color.toLower()) {
            m_colorCombo->setCurrentIndex(i);
            break;
        }
    }
    m_colorCombo->setFixedHeight(38);
    form->addRow(makeLabel("Color Theme"), m_colorCombo);

    mainLayout->addLayout(form);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);
    btnLayout->addStretch();

    QPushButton* cancelBtn = new QPushButton("Cancel", this);
    cancelBtn->setFixedHeight(38);
    cancelBtn->setStyleSheet("background-color: #334155; color: #CBD5E1; border-radius: 8px; padding: 0 16px; font-weight: 600;");
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    QPushButton* saveBtn = new QPushButton("Save Category", this);
    saveBtn->setFixedHeight(38);
    saveBtn->setStyleSheet("background-color: #6366F1; color: #FFFFFF; border-radius: 8px; padding: 0 20px; font-weight: 700;");
    connect(saveBtn, &QPushButton::clicked, this, [this]() {
        if (m_nameEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Validation", "Category name cannot be empty.");
            return;
        }
        m_category.name = m_nameEdit->text().trimmed();
        m_category.isIncome = m_typeCombo->currentData().toBool();
        m_category.icon = m_iconCombo->currentText();
        m_category.color = m_colorCombo->currentData().toString();
        accept();
    });

    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(saveBtn);
    mainLayout->addLayout(btnLayout);
}

Category CategoryDialog::getCategory() const {
    return m_category;
}
