#pragma once
#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

class MetricCard : public QFrame {
    Q_OBJECT
public:
    enum class AccentType {
        Neutral,
        Income,
        Expense,
        Accent,
        Warning
    };

    explicit MetricCard(const QString& title,
                        const QString& initialValue,
                        const QString& subtext,
                        const QString& icon,
                        AccentType accent = AccentType::Neutral,
                        QWidget* parent = nullptr);

    void setValue(const QString& value);
    void setSubtext(const QString& subtext);
    void setAccent(AccentType accent);

private:
    void applyStyle(AccentType accent);

    QLabel* m_iconLabel;
    QLabel* m_titleLabel;
    QLabel* m_valueLabel;
    QLabel* m_subtextLabel;
    AccentType m_accent;
};
