#include "ToastNotification.h"
#include <QHBoxLayout>

ToastNotification::ToastNotification(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_ShowWithoutActivating);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 10, 16, 10);

    m_label = new QLabel(this);
    m_label->setStyleSheet("color: #FFFFFF; font-weight: 600; font-size: 13px;");
    layout->addWidget(m_label);

    m_opacityEffect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(m_opacityEffect);

    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, [this]() {
        QPropertyAnimation* fadeOut = new QPropertyAnimation(m_opacityEffect, "opacity", this);
        fadeOut->setDuration(400);
        fadeOut->setStartValue(1.0);
        fadeOut->setEndValue(0.0);
        connect(fadeOut, &QPropertyAnimation::finished, this, &QWidget::hide);
        fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
    });

    hide();
}

void ToastNotification::display(const QString& message, Type type) {
    m_label->setText(message);

    QString bgCol = "#1E293B";
    QString borderCol = "#6366F1";
    QString icon = "ℹ️";

    if (type == Success) {
        borderCol = "#10B981";
        icon = "✓";
        m_label->setText(QString("%1  %2").arg(icon, message));
    } else if (type == Error) {
        borderCol = "#F43F5E";
        icon = "✕";
        m_label->setText(QString("%1  %2").arg(icon, message));
    } else {
        borderCol = "#6366F1";
        m_label->setText(QString("%1  %2").arg(icon, message));
    }

    setStyleSheet(QString(
        "ToastNotification {"
        "  background-color: %1;"
        "  border: 1px solid %2;"
        "  border-radius: 8px;"
        "}"
    ).arg(bgCol, borderCol));

    adjustSize();

    if (parentWidget()) {
        int x = parentWidget()->width() - width() - 24;
        int y = parentWidget()->height() - height() - 24;
        move(x, y);
    }

    m_opacityEffect->setOpacity(1.0);
    show();
    raise();
    m_timer->start(3000);
}

void ToastNotification::showMessage(QWidget* parent, const QString& message, Type type) {
    if (!parent) return;
    ToastNotification* toast = parent->findChild<ToastNotification*>("globalToast");
    if (!toast) {
        toast = new ToastNotification(parent);
        toast->setObjectName("globalToast");
    }
    toast->display(message, type);
}
