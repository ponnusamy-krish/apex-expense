#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QChart>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include "../components/MetricCard.h"

QT_CHARTS_USE_NAMESPACE

class DashboardView : public QWidget {
    Q_OBJECT
public:
    explicit DashboardView(QWidget* parent = nullptr);

public slots:
    void refreshData();

signals:
    void requestAddTransaction(const QString& type);
    void requestOpenBudgets();
    void requestOpenReports();

private:
    void setupUi();
    void updateCharts();
    void updateRecentTable();

    MetricCard* m_balanceCard;
    MetricCard* m_incomeCard;
    MetricCard* m_expenseCard;
    MetricCard* m_savingsCard;

    QChartView* m_pieChartView;
    QChart* m_pieChart;

    QChartView* m_barChartView;
    QChart* m_barChart;

    QTableWidget* m_recentTable;
};
