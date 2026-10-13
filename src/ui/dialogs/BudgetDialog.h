#pragma once
#include <QDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include "../../models/Budget.h"

class BudgetDialog : public QDialog {
    Q_OBJECT
public:
    explicit BudgetDialog(QWidget* parent = nullptr, int defaultMonth = -1, int defaultYear = -1, const Budget& initialBudget = Budget());

    int getCategoryId() const;
    double getLimit() const;
    int getMonth() const;
    int getYear() const;

private:
    QComboBox* m_categoryCombo;
    QDoubleSpinBox* m_limitSpin;
    QComboBox* m_monthCombo;
    QSpinBox* m_yearSpin;
};
