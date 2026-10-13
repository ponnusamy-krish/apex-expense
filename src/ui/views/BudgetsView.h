#pragma once
#include <QWidget>
#include <QScrollArea>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDate>
#include "../../models/Budget.h"

class BudgetsView : public QWidget {
    Q_OBJECT
public:
    explicit BudgetsView(QWidget* parent = nullptr);

public slots:
    void refreshData();
    void openAddBudgetDialog();

private slots:
    void onPrevMonth();
    void onNextMonth();

private:
    void setupUi();
    void updateBudgetCards();

    QDate m_currentDate;
    QLabel* m_monthLabel;
    QLabel* m_summaryLabel;
    QProgressBar* m_totalProgressBar;

    QWidget* m_cardsContainer;
    QVBoxLayout* m_cardsLayout;
};
