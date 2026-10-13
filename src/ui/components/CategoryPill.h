#pragma once
#include <QWidget>
#include <QLabel>

class CategoryPill : public QWidget {
    Q_OBJECT
public:
    explicit CategoryPill(const QString& name, const QString& icon, const QString& hexColor, QWidget* parent = nullptr);
    void setCategory(const QString& name, const QString& icon, const QString& hexColor);

private:
    QLabel* m_label;
};
