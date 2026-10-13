#pragma once
#include <QWidget>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QPieSeries>
#include <QtCharts/QBarSeries>
#include "../components/MetricCard.h"

QT_CHARTS_USE_NAMESPACE

class AnalyticsView : public QWidget {
    Q_OBJECT
public:
    explicit AnalyticsView(QWidget* parent = nullptr);

public slots:
    void refreshData();

private slots:
    void onPeriodChanged(int index);
    void onExportHtmlReport();

private:
    void setupUi();
    void updateAnalytics();

    QComboBox* m_periodCombo;
    MetricCard* m_dailyAvgCard;
    MetricCard* m_topCatCard;
    MetricCard* m_largestTxCard;
    MetricCard* m_txCountCard;

    QChartView* m_catChartView;
    QChart* m_catChart;

    QChartView* m_payChartView;
    QChart* m_payChart;
};
