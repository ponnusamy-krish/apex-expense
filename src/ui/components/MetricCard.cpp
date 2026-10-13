#include "MetricCard.h"

MetricCard::MetricCard(const QString& title,
                       const QString& initialValue,
                       const QString& subtext,
                       const QString& icon,
                       AccentType accent,
                       QWidget* parent)
    : QFrame(parent), m_accent(accent)
{
    setObjectName("metricCard");
    setAttribute(Qt::WA_StyledBackground, true);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(18, 18, 18, 18);
    mainLayout->setSpacing(8);

    QHBoxLayout* topLayout = new QHBoxLayout();
    topLayout->setSpacing(10);

    m_titleLabel = new QLabel(title.toUpper(), this);
    m_titleLabel->setStyleSheet("font-size: 11px; font-weight: 700; color: #94A3B8; letter-spacing: 0.8px;");

    m_iconLabel = new QLabel(icon, this);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setFixedSize(32, 32);

    topLayout->addWidget(m_titleLabel);
    topLayout->addStretch();
    topLayout->addWidget(m_iconLabel);

    m_valueLabel = new QLabel(initialValue, this);
    m_valueLabel->setStyleSheet("font-size: 26px; font-weight: 800;");

    m_subtextLabel = new QLabel(subtext, this);
    m_subtextLabel->setStyleSheet("font-size: 12px; color: #64748B; font-weight: 500;");

    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(m_valueLabel);
    mainLayout->addWidget(m_subtextLabel);

    applyStyle(accent);
}

void MetricCard::setValue(const QString& value) {
    m_valueLabel->setText(value);
}

void MetricCard::setSubtext(const QString& subtext) {
    m_subtextLabel->setText(subtext);
}

void MetricCard::setAccent(AccentType accent) {
    m_accent = accent;
    applyStyle(accent);
}

void MetricCard::applyStyle(AccentType accent) {
    QString borderCol = "#334155";
    QString valCol = "#F8FAFC";
    QString iconBg = "rgba(51, 65, 85, 0.4)";

    switch (accent) {
    case AccentType::Income:
        borderCol = "#059669";
        valCol = "#10B981";
        iconBg = "rgba(16, 185, 129, 0.15)";
        break;
    case AccentType::Expense:
        borderCol = "#E11D48";
        valCol = "#F43F5E";
        iconBg = "rgba(244, 63, 94, 0.15)";
        break;
    case AccentType::Accent:
        borderCol = "#6366F1";
        valCol = "#818CF8";
        iconBg = "rgba(99, 102, 241, 0.15)";
        break;
    case AccentType::Warning:
        borderCol = "#D97706";
        valCol = "#F59E0B";
        iconBg = "rgba(245, 158, 11, 0.15)";
        break;
    default:
        borderCol = "#334155";
        valCol = "#F8FAFC";
        iconBg = "rgba(51, 65, 85, 0.5)";
        break;
    }

    setStyleSheet(QString(
        "QFrame#metricCard {"
        "  background: #1E293B;"
        "  border: 1px solid %1;"
        "  border-radius: 12px;"
        "}"
        "QFrame#metricCard:hover {"
        "  border: 1px solid %2;"
        "}"
    ).arg(borderCol, valCol));

    m_valueLabel->setStyleSheet(QString("font-size: 26px; font-weight: 800; color: %1;").arg(valCol));
    m_iconLabel->setStyleSheet(QString(
        "background-color: %1;"
        "border-radius: 8px;"
        "font-size: 16px;"
    ).arg(iconBg));
}
