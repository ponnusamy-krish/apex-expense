#pragma once
#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

class ToastNotification : public QWidget {
    Q_OBJECT
public:
    enum Type {
        Success,
        Error,
        Info
    };

    explicit ToastNotification(QWidget* parent = nullptr);
    static void showMessage(QWidget* parent, const QString& message, Type type = Success);

private:
    void display(const QString& message, Type type);

    QLabel* m_label;
    QTimer* m_timer;
    QGraphicsOpacityEffect* m_opacityEffect;
    QPropertyAnimation* m_anim;
};
