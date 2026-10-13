#include "DashboardView.h"
#include "../../database/DatabaseManager.h"
#include "../../utils/CurrencyFormatter.h"
#include "../components/CategoryPill.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QHeaderView>
#include <QDate>
#include <QGraphicsLayout>
#include <QtCharts/QPieSlice>

DashboardView::DashboardView(QWidget* parent) : QWidget(parent) {
    setupUi();
    connect(&DatabaseManager::instance(), &DatabaseManager::dataChanged, this, &DashboardView::refreshData);
    refreshData();
}

void DashboardView::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(20);

    // Header bar with Title and Quick Add Buttons
    QHBoxLayout* headerLayout = new QHBoxLayout();
    QVBoxLayout* titleLayout = new QVBoxLayout();

    QLabel* title = new QLabel("Financial Dashboard", this);
    title->setStyleSheet("font-size: 24px; font-weight: 800; color: #F8FAFC;");

    QLabel* subtitle = new QLabel("Real-time summary of your finances, cashflow, and budgets", this);
    subtitle->setStyleSheet("font-size: 13px; color: #94A3B8; font-weight: 500;");

    titleLayout->addWidget(title);
    titleLayout->addWidget(subtitle);
    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();

    // Quick Add Buttons
    QPushButton* addExpenseBtn = new QPushButton("– Add Expense", this);
    addExpenseBtn->setFixedHeight(40);
    addExpenseBtn->setStyleSheet(
        "QPushButton { background: #E11D48; color: #FFFFFF; border-radius: 8px; font-weight: 700; padding: 0 16px; }"
        "QPushButton:hover { background: #F43F5E; }"
    );
    connect(addExpenseBtn, &QPushButton::clicked, this, [this]() {
        emit requestAddTransaction("Expense");
    });

    QPushButton* addIncomeBtn = new QPushButton("+ Add Income", this);
    addIncomeBtn->setFixedHeight(40);
    addIncomeBtn->setStyleSheet(
        "QPushButton { background: #059669; color: #FFFFFF; border-radius: 8px; font-weight: 700; padding: 0 16px; }"
        "QPushButton:hover { background: #10B981; }"
    );
    connect(addIncomeBtn, &QPushButton::clicked, this, [this]() {
        emit requestAddTransaction("Income");
    });

    headerLayout->addWidget(addExpenseBtn);
    headerLayout->addWidget(addIncomeBtn);
    mainLayout->addLayout(headerLayout);

    // 1. Metric Cards Grid
    QGridLayout* cardGrid = new QGridLayout();
    cardGrid->setSpacing(16);

    m_balanceCard = new MetricCard("Total Balance", "$0.00", "Net cash on hand", "💎", MetricCard::AccentType::Neutral, this);
    m_incomeCard = new MetricCard("Monthly Income", "$0.00", "This month", "📈", MetricCard::AccentType::Income, this);
    m_expenseCard = new MetricCard("Monthly Expenses", "$0.00", "This month", "📉", MetricCard::AccentType::Expense, this);
    m_savingsCard = new MetricCard("Savings Rate", "0.0%", "Income retained", "🎯", MetricCard::AccentType::Accent, this);

    cardGrid->addWidget(m_balanceCard, 0, 0);
    cardGrid->addWidget(m_incomeCard, 0, 1);
    cardGrid->addWidget(m_expenseCard, 0, 2);
    cardGrid->addWidget(m_savingsCard, 0, 3);
    mainLayout->addLayout(cardGrid);

    // 2. Charts Section
    QHBoxLayout* chartsLayout = new QHBoxLayout();
    chartsLayout->setSpacing(16);

    // Left Chart: Category Donut Chart
    QFrame* pieFrame = new QFrame(this);
    pieFrame->setStyleSheet("background: #1E293B; border: 1px solid #334155; border-radius: 12px;");
    QVBoxLayout* pieLayout = new QVBoxLayout(pieFrame);
    pieLayout->setContentsMargins(16, 16, 16, 16);

    QLabel* pieTitle = new QLabel("Spending by Category", pieFrame);
    pieTitle->setStyleSheet("font-size: 15px; font-weight: 700; color: #F8FAFC; border: none;");
    pieLayout->addWidget(pieTitle);

    m_pieChart = new QChart();
    m_pieChart->setBackgroundBrush(QBrush(QColor("#1E293B")));
    m_pieChart->setBackgroundRoundness(0);
    m_pieChart->legend()->setVisible(true);
    m_pieChart->legend()->setAlignment(Qt::AlignRight);
    m_pieChart->legend()->setLabelColor(QColor("#94A3B8"));
    m_pieChart->layout()->setContentsMargins(0, 0, 0, 0);

    m_pieChartView = new QChartView(m_pieChart, pieFrame);
    m_pieChartView->setRenderHint(QPainter::Antialiasing);
    m_pieChartView->setStyleSheet("background: transparent; border: none;");
    m_pieChartView->setMinimumHeight(240);
    pieLayout->addWidget(m_pieChartView);

    // Right Chart: Monthly Trends Bar Chart
    QFrame* barFrame = new QFrame(this);
    barFrame->setStyleSheet("background: #1E293B; border: 1px solid #334155; border-radius: 12px;");
    QVBoxLayout* barLayout = new QVBoxLayout(barFrame);
    barLayout->setContentsMargins(16, 16, 16, 16);

    QLabel* barTitle = new QLabel("Income vs Expense Trend (Last 6 Months)", barFrame);
    barTitle->setStyleSheet("font-size: 15px; font-weight: 700; color: #F8FAFC; border: none;");
    barLayout->addWidget(barTitle);

    m_barChart = new QChart();
    m_barChart->setBackgroundBrush(QBrush(QColor("#1E293B")));
    m_barChart->setBackgroundRoundness(0);
    m_barChart->legend()->setVisible(true);
    m_barChart->legend()->setAlignment(Qt::AlignTop);
    m_barChart->legend()->setLabelColor(QColor("#94A3B8"));
    m_barChart->layout()->setContentsMargins(0, 0, 0, 0);

    m_barChartView = new QChartView(m_barChart, barFrame);
    m_barChartView->setRenderHint(QPainter::Antialiasing);
    m_barChartView->setStyleSheet("background: transparent; border: none;");
    m_barChartView->setMinimumHeight(240);
    barLayout->addWidget(m_barChartView);

    chartsLayout->addWidget(pieFrame, 1);
    chartsLayout->addWidget(barFrame, 1);
    mainLayout->addLayout(chartsLayout);

    // 3. Recent Transactions Table Frame
    QFrame* recentFrame = new QFrame(this);
    recentFrame->setStyleSheet("background: #1E293B; border: 1px solid #334155; border-radius: 12px;");
    QVBoxLayout* recentLayout = new QVBoxLayout(recentFrame);
    recentLayout->setContentsMargins(18, 18, 18, 18);

    QHBoxLayout* recentHeader = new QHBoxLayout();
    QLabel* recentTitle = new QLabel("Recent Activity", recentFrame);
    recentTitle->setStyleSheet("font-size: 16px; font-weight: 700; color: #F8FAFC; border: none;");
    recentHeader->addWidget(recentTitle);
    recentHeader->addStretch();

    recentLayout->addLayout(recentHeader);

    m_recentTable = new QTableWidget(recentFrame);
    m_recentTable->setColumnCount(6);
    m_recentTable->setHorizontalHeaderLabels({"Date", "Type", "Category", "Description", "Payment Method", "Amount"});
    m_recentTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_recentTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_recentTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_recentTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    m_recentTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_recentTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    m_recentTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_recentTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_recentTable->setAlternatingRowColors(true);
    m_recentTable->verticalHeader()->setVisible(false);
    m_recentTable->setStyleSheet(
        "QTableWidget {"
        "  background-color: #1E293B;"
        "  border: none;"
        "  gridline-color: #334155;"
        "  color: #F8FAFC;"
        "}"
        "QTableWidget::item {"
        "  padding: 8px;"
        "  border-bottom: 1px solid #334155;"
        "}"
        "QTableWidget::item:alternate {"
        "  background-color: #162032;"
        "}"
        "QHeaderView::section {"
        "  background-color: #0F172A;"
        "  color: #94A3B8;"
        "  font-weight: 700;"
        "  font-size: 12px;"
        "  padding: 8px;"
        "  border: none;"
        "  border-bottom: 2px solid #334155;"
        "}"
    );
    m_recentTable->setMinimumHeight(200);
    recentLayout->addWidget(m_recentTable);

    mainLayout->addWidget(recentFrame);
}

void DashboardView::refreshData() {
    DatabaseManager& db = DatabaseManager::instance();
    CurrencyFormatter& cf = CurrencyFormatter::instance();

    QDate today = QDate::currentDate();
    QDateTime startOfMonth(QDate(today.year(), today.month(), 1).startOfDay());
    QDateTime endOfMonth(QDate(today.year(), today.month(), today.daysInMonth()).endOfDay());

    // 1. Current Month Totals
    double curMonthIncome = 0.0, curMonthExpense = 0.0;
    db.getFinancialTotals(startOfMonth, endOfMonth, curMonthIncome, curMonthExpense);

    // 2. All Time Totals for Net Balance
    double allIncome = 0.0, allExpense = 0.0;
    db.getFinancialTotals(QDateTime(), QDateTime(), allIncome, allExpense);
    double totalBalance = allIncome - allExpense;

    // 3. Savings Rate this month
    double savingsRate = 0.0;
    if (curMonthIncome > 0.0) {
        savingsRate = ((curMonthIncome - curMonthExpense) / curMonthIncome) * 100.0;
        if (savingsRate < 0.0) savingsRate = 0.0;
    }

    m_balanceCard->setValue(cf.format(totalBalance));
    m_balanceCard->setAccent(totalBalance >= 0 ? MetricCard::AccentType::Income : MetricCard::AccentType::Expense);

    m_incomeCard->setValue(cf.format(curMonthIncome));
    m_expenseCard->setValue(cf.format(curMonthExpense));
    m_savingsCard->setValue(QString("%1%").arg(QString::number(savingsRate, 'f', 1)));

    updateCharts();
    updateRecentTable();
}

void DashboardView::updateCharts() {
    DatabaseManager& db = DatabaseManager::instance();
    CurrencyFormatter& cf = CurrencyFormatter::instance();

    // 1. Donut Pie Chart for Expenses
    m_pieChart->removeAllSeries();
    QPieSeries* pieSeries = new QPieSeries();
    pieSeries->setHoleSize(0.48); // Modern donut look

    QDate today = QDate::currentDate();
    QDateTime startOfMonth(QDate(today.year(), today.month(), 1).startOfDay());
    QDateTime endOfMonth(QDate(today.year(), today.month(), today.daysInMonth()).endOfDay());

    QList<CategoryStat> catStats = db.getCategoryExpenses(startOfMonth, endOfMonth);
    if (catStats.isEmpty()) {
        // Fallback to all time if current month is empty
        catStats = db.getCategoryExpenses(QDateTime(), QDateTime());
    }

    if (catStats.isEmpty()) {
        QPieSlice* slice = pieSeries->append("No Expenses", 1);
        slice->setColor(QColor("#475569"));
        slice->setLabelVisible(true);
        slice->setLabelColor(QColor("#94A3B8"));
    } else {
        int maxSlices = 6;
        double otherAmount = 0.0;
        for (int i = 0; i < catStats.size(); ++i) {
            const auto& cs = catStats[i];
            if (i < maxSlices) {
                QString label = QString("%1 (%2%)").arg(cs.name).arg(QString::number(cs.percentage, 'f', 0));
                QPieSlice* slice = pieSeries->append(label, cs.totalAmount);
                slice->setColor(QColor(cs.color));
                slice->setBorderColor(QColor("#1E293B"));
                slice->setBorderWidth(2);
                slice->setLabelColor(QColor("#F8FAFC"));
            } else {
                otherAmount += cs.totalAmount;
            }
        }
        if (otherAmount > 0.0) {
            QPieSlice* slice = pieSeries->append("Other", otherAmount);
            slice->setColor(QColor("#64748B"));
            slice->setBorderColor(QColor("#1E293B"));
            slice->setBorderWidth(2);
            slice->setLabelColor(QColor("#F8FAFC"));
        }
    }

    m_pieChart->addSeries(pieSeries);

    // 2. Monthly Trend Bar Chart
    m_barChart->removeAllSeries();
    const auto axes = m_barChart->axes();
    for (auto ax : axes) {
        m_barChart->removeAxis(ax);
    }

    QBarSeries* barSeries = new QBarSeries();
    QBarSet* incomeSet = new QBarSet("Income");
    QBarSet* expenseSet = new QBarSet("Expenses");

    incomeSet->setColor(QColor("#10B981"));
    expenseSet->setColor(QColor("#F43F5E"));
    incomeSet->setBorderColor(QColor("#10B981"));
    expenseSet->setBorderColor(QColor("#F43F5E"));

    QStringList categories;
    QList<MonthlyStat> trends = db.getMonthlyTrends(6);
    double maxVal = 100.0;

    for (const auto& stat : trends) {
        categories << stat.monthLabel;
        *incomeSet << stat.totalIncome;
        *expenseSet << stat.totalExpense;
        if (stat.totalIncome > maxVal) maxVal = stat.totalIncome;
        if (stat.totalExpense > maxVal) maxVal = stat.totalExpense;
    }

    barSeries->append(incomeSet);
    barSeries->append(expenseSet);
    m_barChart->addSeries(barSeries);

    QBarCategoryAxis* axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setLabelsColor(QColor("#94A3B8"));
    axisX->setGridLineColor(QColor("#334155"));
    axisX->setLinePenColor(QColor("#475569"));
    m_barChart->addAxis(axisX, Qt::AlignBottom);
    barSeries->attachAxis(axisX);

    QValueAxis* axisY = new QValueAxis();
    axisY->setRange(0, maxVal * 1.15);
    axisY->setLabelsColor(QColor("#94A3B8"));
    axisY->setGridLineColor(QColor("#334155"));
    axisY->setLinePenColor(QColor("#475569"));
    m_barChart->addAxis(axisY, Qt::AlignLeft);
    barSeries->attachAxis(axisY);
}

void DashboardView::updateRecentTable() {
    DatabaseManager& db = DatabaseManager::instance();
    CurrencyFormatter& cf = CurrencyFormatter::instance();

    QList<Transaction> recents = db.getRecentTransactions(8);
    m_recentTable->setRowCount(recents.size());

    for (int row = 0; row < recents.size(); ++row) {
        const auto& t = recents[row];

        // Date
        QTableWidgetItem* dateItem = new QTableWidgetItem(t.dateTime.toString("yyyy-MM-dd"));
        dateItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        m_recentTable->setItem(row, 0, dateItem);

        // Type
        QTableWidgetItem* typeItem = new QTableWidgetItem(t.type);
        typeItem->setTextAlignment(Qt::AlignCenter);
        if (t.type == "Income") {
            typeItem->setForeground(QColor("#10B981"));
        } else {
            typeItem->setForeground(QColor("#F43F5E"));
        }
        m_recentTable->setItem(row, 1, typeItem);

        // Category
        QTableWidgetItem* catItem = new QTableWidgetItem(QString("%1 %2").arg(t.categoryIcon, t.categoryName));
        catItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        m_recentTable->setItem(row, 2, catItem);

        // Description
        QTableWidgetItem* descItem = new QTableWidgetItem(t.description);
        descItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        m_recentTable->setItem(row, 3, descItem);

        // Payment Method
        QTableWidgetItem* payItem = new QTableWidgetItem(t.paymentMethod);
        payItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        m_recentTable->setItem(row, 4, payItem);

        // Amount
        QTableWidgetItem* amountItem = new QTableWidgetItem(cf.formatSigned(t.amount, t.type));
        amountItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
        amountItem->setFont(QFont("sans-serif", 10, QFont::Bold));
        if (t.type == "Income") {
            amountItem->setForeground(QColor("#10B981"));
        } else {
            amountItem->setForeground(QColor("#F43F5E"));
        }
        m_recentTable->setItem(row, 5, amountItem);
    }
}
