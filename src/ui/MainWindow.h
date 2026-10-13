#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QButtonGroup>
#include "views/DashboardView.h"
#include "views/TransactionsView.h"
#include "views/BudgetsView.h"
#include "views/AnalyticsView.h"
#include "views/SettingsView.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

public slots:
    void switchView(int index);
    void onQuickAddTransaction();

private:
    void setupUi();
    void setupShortcuts();
    QPushButton* createNavButton(const QString& icon, const QString& title, int viewIndex);

    QStackedWidget* m_stackedWidget;
    QButtonGroup* m_navGroup;
    QList<QPushButton*> m_navButtons;

    DashboardView* m_dashboardView;
    TransactionsView* m_transactionsView;
    BudgetsView* m_budgetsView;
    AnalyticsView* m_analyticsView;
    SettingsView* m_settingsView;
};
