#include "AnalyticsView.h"
#include "../../database/DatabaseManager.h"
#include "../../utils/CurrencyFormatter.h"
#include "../../utils/HtmlReportGenerator.h"
#include "../components/ToastNotification.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QFileDialog>
#include <QMessageBox>
#include <QDate>
#include <QListView>
#include <QtCharts/QPieSlice>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

AnalyticsView::AnalyticsView(QWidget* parent) : QWidget(parent) {
    setupUi();
    connect(&DatabaseManager::instance(), &DatabaseManager::dataChanged, this, &AnalyticsView::refreshData);
    refreshData();
}

void AnalyticsView::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(18);

    // Header with Title & Period Selector
    QHBoxLayout* topHeader = new QHBoxLayout();
    QVBoxLayout* titleLayout = new QVBoxLayout();

    QLabel* title = new QLabel("Financial Analytics & Insights", this);
    title->setStyleSheet("font-size: 24px; font-weight: 800; color: #F8FAFC;");

    QLabel* subtitle = new QLabel("In-depth spending patterns, payment methods distribution, and reports", this);
    subtitle->setStyleSheet("font-size: 13px; color: #94A3B8; font-weight: 500;");

    titleLayout->addWidget(title);
    titleLayout->addWidget(subtitle);
    topHeader->addLayout(titleLayout);
    topHeader->addStretch();

    m_periodCombo = new QComboBox(this);
    m_periodCombo->setView(new QListView(m_periodCombo));
    m_periodCombo->addItems({"This Month", "Last 3 Months", "Last 6 Months", "This Year", "All Time"});
    m_periodCombo->setCurrentIndex(0);
    m_periodCombo->setFixedHeight(38);
    connect(m_periodCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AnalyticsView::onPeriodChanged);
    topHeader->addWidget(m_periodCombo);

    QPushButton* reportBtn = new QPushButton("📄 Generate HTML Report", this);
    reportBtn->setFixedHeight(38);
    reportBtn->setStyleSheet("background: #6366F1; color: white; border-radius: 8px; font-weight: 700; padding: 0 16px;");
    connect(reportBtn, &QPushButton::clicked, this, &AnalyticsView::onExportHtmlReport);
    topHeader->addWidget(reportBtn);

    mainLayout->addLayout(topHeader);

    // 4 Key Analytics Cards
    QGridLayout* grid = new QGridLayout();
    grid->setSpacing(16);

    m_dailyAvgCard = new MetricCard("Daily Average Spend", "$0.00", "Across period", "📅", MetricCard::AccentType::Expense, this);
    m_topCatCard = new MetricCard("Top Expense Category", "None", "Highest share", "🏆", MetricCard::AccentType::Warning, this);
    m_largestTxCard = new MetricCard("Largest Expense", "$0.00", "Single peak transaction", "⚡", MetricCard::AccentType::Expense, this);
    m_txCountCard = new MetricCard("Total Transactions", "0", "Records analyzed", "📊", MetricCard::AccentType::Neutral, this);

    grid->addWidget(m_dailyAvgCard, 0, 0);
    grid->addWidget(m_topCatCard, 0, 1);
    grid->addWidget(m_largestTxCard, 0, 2);
    grid->addWidget(m_txCountCard, 0, 3);
    mainLayout->addLayout(grid);

    // Charts Section
    QHBoxLayout* chartsLayout = new QHBoxLayout();
    chartsLayout->setSpacing(16);

    // Left Chart: Category Distribution
    QFrame* catFrame = new QFrame(this);
    catFrame->setStyleSheet("background: #1E293B; border: 1px solid #334155; border-radius: 12px;");
    QVBoxLayout* catLayout = new QVBoxLayout(catFrame);
    catLayout->setContentsMargins(16, 16, 16, 16);

    QLabel* catTitle = new QLabel("Category Expense Allocation", catFrame);
    catTitle->setStyleSheet("font-size: 15px; font-weight: 700; color: #F8FAFC; border: none;");
    catLayout->addWidget(catTitle);

    m_catChart = new QChart();
    m_catChart->setBackgroundBrush(QBrush(QColor("#1E293B")));
    m_catChart->legend()->setVisible(true);
    m_catChart->legend()->setAlignment(Qt::AlignRight);
    m_catChart->legend()->setLabelColor(QColor("#94A3B8"));

    m_catChartView = new QChartView(m_catChart, catFrame);
    m_catChartView->setRenderHint(QPainter::Antialiasing);
    m_catChartView->setStyleSheet("background: transparent; border: none;");
    m_catChartView->setMinimumHeight(280);
    catLayout->addWidget(m_catChartView);

    // Right Chart: Payment Methods Distribution
    QFrame* payFrame = new QFrame(this);
    payFrame->setStyleSheet("background: #1E293B; border: 1px solid #334155; border-radius: 12px;");
    QVBoxLayout* payLayout = new QVBoxLayout(payFrame);
    payLayout->setContentsMargins(16, 16, 16, 16);

    QLabel* payTitle = new QLabel("Payment Methods Usage", payFrame);
    payTitle->setStyleSheet("font-size: 15px; font-weight: 700; color: #F8FAFC; border: none;");
    payLayout->addWidget(payTitle);

    m_payChart = new QChart();
    m_payChart->setBackgroundBrush(QBrush(QColor("#1E293B")));
    m_payChart->legend()->setVisible(true);
    m_payChart->legend()->setAlignment(Qt::AlignRight);
    m_payChart->legend()->setLabelColor(QColor("#94A3B8"));

    m_payChartView = new QChartView(m_payChart, payFrame);
    m_payChartView->setRenderHint(QPainter::Antialiasing);
    m_payChartView->setStyleSheet("background: transparent; border: none;");
    m_payChartView->setMinimumHeight(280);
    payLayout->addWidget(m_payChartView);

    chartsLayout->addWidget(catFrame, 1);
    chartsLayout->addWidget(payFrame, 1);
    mainLayout->addLayout(chartsLayout);
}

void AnalyticsView::onPeriodChanged(int /*index*/) {
    refreshData();
}

void AnalyticsView::refreshData() {
    updateAnalytics();
}

void AnalyticsView::updateAnalytics() {
    DatabaseManager& db = DatabaseManager::instance();
    CurrencyFormatter& cf = CurrencyFormatter::instance();

    QDate today = QDate::currentDate();
    QDateTime fromDt;
    QDateTime toDt;

    int daysInPeriod = 30;
    int idx = m_periodCombo->currentIndex();
    if (idx == 0) { // This Month
        fromDt = QDate(today.year(), today.month(), 1).startOfDay();
        toDt = QDate(today.year(), today.month(), today.daysInMonth()).endOfDay();
        daysInPeriod = today.day();
    } else if (idx == 1) { // Last 3 Months
        fromDt = today.addMonths(-3).startOfDay();
        toDt = today.endOfDay();
        daysInPeriod = 90;
    } else if (idx == 2) { // Last 6 Months
        fromDt = today.addMonths(-6).startOfDay();
        toDt = today.endOfDay();
        daysInPeriod = 180;
    } else if (idx == 3) { // This Year
        fromDt = QDate(today.year(), 1, 1).startOfDay();
        toDt = QDate(today.year(), 12, 31).endOfDay();
        daysInPeriod = today.dayOfYear();
    } else { // All Time
        fromDt = QDateTime();
        toDt = QDateTime();
        daysInPeriod = 365;
    }

    if (daysInPeriod <= 0) daysInPeriod = 1;

    double totalIncome = 0.0, totalExpense = 0.0;
    db.getFinancialTotals(fromDt, toDt, totalIncome, totalExpense);

    double dailyAvg = totalExpense / static_cast<double>(daysInPeriod);
    m_dailyAvgCard->setValue(cf.format(dailyAvg));
    m_dailyAvgCard->setSubtext(QString("Over %1 days").arg(daysInPeriod));

    QList<Transaction> transactions = db.getFilteredTransactions(fromDt, toDt);
    m_txCountCard->setValue(QString::number(transactions.size()));

    double largestExpense = 0.0;
    QString largestExpenseDesc = "None";
    for (const auto& t : transactions) {
        if (t.type == "Expense" && t.amount > largestExpense) {
            largestExpense = t.amount;
            largestExpenseDesc = t.description;
        }
    }
    m_largestTxCard->setValue(cf.format(largestExpense));
    m_largestTxCard->setSubtext(largestExpenseDesc.length() > 20 ? largestExpenseDesc.left(17) + "..." : largestExpenseDesc);

    QList<CategoryStat> catStats = db.getCategoryExpenses(fromDt, toDt);
    if (!catStats.isEmpty()) {
        m_topCatCard->setValue(catStats[0].name);
        m_topCatCard->setSubtext(QString("%1 (%2%)").arg(cf.format(catStats[0].totalAmount)).arg(QString::number(catStats[0].percentage, 'f', 1)));
    } else {
        m_topCatCard->setValue("None");
        m_topCatCard->setSubtext("No records");
    }

    // Update Category Donut Chart
    m_catChart->removeAllSeries();
    QPieSeries* catSeries = new QPieSeries();
    catSeries->setHoleSize(0.42);

    if (catStats.isEmpty()) {
        QPieSlice* s = catSeries->append("No Data", 1);
        s->setColor(QColor("#475569"));
    } else {
        int maxItems = 6;
        double otherAmt = 0.0;
        for (int i = 0; i < catStats.size(); ++i) {
            const auto& cs = catStats[i];
            if (i < maxItems) {
                QPieSlice* s = catSeries->append(QString("%1 %2%").arg(cs.name).arg(QString::number(cs.percentage, 'f', 0)), cs.totalAmount);
                s->setColor(QColor(cs.color));
                s->setBorderColor(QColor("#1E293B"));
                s->setBorderWidth(2);
            } else {
                otherAmt += cs.totalAmount;
            }
        }
        if (otherAmt > 0.0) {
            QPieSlice* s = catSeries->append("Other", otherAmt);
            s->setColor(QColor("#64748B"));
            s->setBorderColor(QColor("#1E293B"));
            s->setBorderWidth(2);
        }
    }
    m_catChart->addSeries(catSeries);

    // Update Payment Methods Donut Chart
    m_payChart->removeAllSeries();
    QPieSeries* paySeries = new QPieSeries();
    paySeries->setHoleSize(0.42);

    QMap<QString, double> payBreakdown = db.getPaymentMethodBreakdown(fromDt, toDt);
    if (payBreakdown.isEmpty()) {
        QPieSlice* s = paySeries->append("No Data", 1);
        s->setColor(QColor("#475569"));
    } else {
        QStringList colors = {"#3B82F6", "#10B981", "#F59E0B", "#8B5CF6", "#EC4899", "#06B6D4", "#64748B"};
        int colIdx = 0;
        for (auto it = payBreakdown.begin(); it != payBreakdown.end(); ++it) {
            QPieSlice* s = paySeries->append(it.key(), it.value());
            s->setColor(QColor(colors[colIdx % colors.size()]));
            s->setBorderColor(QColor("#1E293B"));
            s->setBorderWidth(2);
            colIdx++;
        }
    }
    m_payChart->addSeries(paySeries);
}

void AnalyticsView::onExportHtmlReport() {
    QDate today = QDate::currentDate();
    QDateTime fromDt;
    QDateTime toDt;

    int idx = m_periodCombo->currentIndex();
    if (idx == 0) { // This Month
        fromDt = QDate(today.year(), today.month(), 1).startOfDay();
        toDt = QDate(today.year(), today.month(), today.daysInMonth()).endOfDay();
    } else if (idx == 1) { // Last 3 Months
        fromDt = today.addMonths(-3).startOfDay();
        toDt = today.endOfDay();
    } else if (idx == 2) { // Last 6 Months
        fromDt = today.addMonths(-6).startOfDay();
        toDt = today.endOfDay();
    } else if (idx == 3) { // This Year
        fromDt = QDate(today.year(), 1, 1).startOfDay();
        toDt = QDate(today.year(), 12, 31).endOfDay();
    }

    QString defaultName = QString("Financial_Report_%1.html").arg(today.toString("yyyyMMdd"));
    QString filePath = QFileDialog::getSaveFileName(this, "Save Financial Report", defaultName, "HTML Files (*.html)");

    if (filePath.isEmpty()) return;

    if (HtmlReportGenerator::generateReport(filePath, fromDt, toDt, QString("Financial Analytics Report &bull; %1").arg(m_periodCombo->currentText()))) {
        ToastNotification::showMessage(window(), "HTML report generated successfully!", ToastNotification::Success);
    } else {
        QMessageBox::critical(this, "Export Error", "Failed to generate HTML report.");
    }
}
