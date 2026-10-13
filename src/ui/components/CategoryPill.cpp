#include "CategoryPill.h"
#include <QHBoxLayout>

CategoryPill::CategoryPill(const QString& name, const QString& icon, const QString& hexColor, QWidget* parent)
    : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_label = new QLabel(this);
    m_label->setTextInteractionFlags(Qt::NoTextInteraction);
    layout->addWidget(m_label);

    setCategory(name, icon, hexColor);
}

void CategoryPill::setCategory(const QString& name, const QString& icon, const QString& hexColor) {
    m_label->setText(QString("%1 %2").arg(icon, name));
    m_label->setStyleSheet(QString(
        "padding: 4px 10px;"
        "border-radius: 12px;"
        "font-size: 12px;"
        "font-weight: 600;"
        "color: %1;"
        "background-color: %122;"
        "border: 1px solid %144;"
    ).arg(hexColor.isEmpty() ? "#6366F1" : hexColor));
}
