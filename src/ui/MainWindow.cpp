#include "MainWindow.h"
#include "../database/DatabaseManager.h"
#include "../utils/CurrencyFormatter.h"
#include "dialogs/TransactionDialog.h"
#include "components/ToastNotification.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QShortcut>
#include <QKeySequence>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("ApexExpense — Modern Personal Finance & Expense Tracker");
    resize(1200, 780);
    setMinimumSize(1000, 650);

    setupUi();
    setupShortcuts();
}

void MainWindow::setupUi() {
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout* rootLayout = new QHBoxLayout(centralWidget);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // ==================== LEFT SIDEBAR ====================
    QFrame* sidebar = new QFrame(centralWidget);
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(240);
    sidebar->setStyleSheet(
        "QFrame#sidebar {"
        "  background-color: #0B0F19;"
        "  border-right: 1px solid #1E293B;"
        "}"
    );

    QVBoxLayout* sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(16, 24, 16, 20);
    sidebarLayout->setSpacing(8);

    // Brand / Logo
    QHBoxLayout* brandLayout = new QHBoxLayout();
    brandLayout->setSpacing(10);

    QLabel* logoIcon = new QLabel("✦", sidebar);
    logoIcon->setStyleSheet("font-size: 24px; color: #6366F1; font-weight: bold;");

    QVBoxLayout* brandTextLayout = new QVBoxLayout();
    QLabel* brandName = new QLabel("ApexExpense", sidebar);
    brandName->setStyleSheet("font-size: 18px; font-weight: 800; color: #F8FAFC; letter-spacing: 0.5px;");

    QLabel* brandTag = new QLabel("Personal Finance", sidebar);
    brandTag->setStyleSheet("font-size: 11px; color: #64748B; font-weight: 600; text-transform: uppercase; letter-spacing: 1px;");

    brandTextLayout->addWidget(brandName);
    brandTextLayout->addWidget(brandTag);
    brandLayout->addWidget(logoIcon);
    brandLayout->addLayout(brandTextLayout);
    brandLayout->addStretch();

    sidebarLayout->addLayout(brandLayout);
    sidebarLayout->addSpacing(24);

    // Navigation Category Label
    QLabel* navLabel = new QLabel("MENU", sidebar);
    navLabel->setStyleSheet("font-size: 11px; font-weight: 700; color: #475569; letter-spacing: 1px; padding-left: 8px; margin-bottom: 4px;");
    sidebarLayout->addWidget(navLabel);

    // Navigation buttons group
    m_navGroup = new QButtonGroup(this);
    m_navGroup->setExclusive(true);

    m_navButtons.append(createNavButton("📊", "Dashboard", 0));
    m_navButtons.append(createNavButton("💳", "Transactions", 1));
    m_navButtons.append(createNavButton("🎯", "Budgets", 2));
    m_navButtons.append(createNavButton("📈", "Analytics", 3));
    m_navButtons.append(createNavButton("⚙️", "Settings", 4));

    for (int i = 0; i < m_navButtons.size(); ++i) {
        sidebarLayout->addWidget(m_navButtons[i]);
    }

    sidebarLayout->addStretch();

    // Quick Add Button in Sidebar
    QPushButton* quickAddBtn = new QPushButton("＋ Quick Transaction", sidebar);
    quickAddBtn->setFixedHeight(42);
    quickAddBtn->setStyleSheet(
        "QPushButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #6366F1, stop:1 #4F46E5);"
        "  color: #FFFFFF;"
        "  border: none;"
        "  border-radius: 8px;"
        "  font-weight: 700;"
        "  font-size: 13px;"
        "}"
        "QPushButton:hover {"
        "  background: #4F46E5;"
        "}"
    );
    connect(quickAddBtn, &QPushButton::clicked, this, &MainWindow::onQuickAddTransaction);
    sidebarLayout->addWidget(quickAddBtn);
    sidebarLayout->addSpacing(12);

    // Database Status Indicator
    QFrame* statusBox = new QFrame(sidebar);
    statusBox->setStyleSheet("background: #111827; border: 1px solid #1F2937; border-radius: 8px; padding: 10px;");
    QVBoxLayout* statusLayout = new QVBoxLayout(statusBox);
    statusLayout->setContentsMargins(8, 8, 8, 8);
    statusLayout->setSpacing(4);

    QHBoxLayout* dotLayout = new QHBoxLayout();
    QLabel* greenDot = new QLabel("●", statusBox);
    greenDot->setStyleSheet("color: #10B981; font-size: 12px;");
    QLabel* dbStatus = new QLabel("SQLite Connected", statusBox);
    dbStatus->setStyleSheet("color: #94A3B8; font-size: 11px; font-weight: 600;");
    dotLayout->addWidget(greenDot);
    dotLayout->addWidget(dbStatus);
    dotLayout->addStretch();

    QLabel* verLabel = new QLabel("v1.0.0 • Qt 5.15 GUI", statusBox);
    verLabel->setStyleSheet("color: #475569; font-size: 10px; font-weight: 500;");

    statusLayout->addLayout(dotLayout);
    statusLayout->addWidget(verLabel);
    sidebarLayout->addWidget(statusBox);

    rootLayout->addWidget(sidebar);

    // ==================== MAIN CONTENT AREA ====================
    m_stackedWidget = new QStackedWidget(centralWidget);
    m_stackedWidget->setObjectName("contentStack");
    m_stackedWidget->setStyleSheet("background-color: #0F172A;");

    m_dashboardView = new DashboardView(this);
    m_transactionsView = new TransactionsView(this);
    m_budgetsView = new BudgetsView(this);
    m_analyticsView = new AnalyticsView(this);
    m_settingsView = new SettingsView(this);

    m_stackedWidget->addWidget(m_dashboardView);
    m_stackedWidget->addWidget(m_transactionsView);
    m_stackedWidget->addWidget(m_budgetsView);
    m_stackedWidget->addWidget(m_analyticsView);
    m_stackedWidget->addWidget(m_settingsView);

    rootLayout->addWidget(m_stackedWidget, 1);

    // Cross-view signals
    connect(m_dashboardView, &DashboardView::requestAddTransaction, this, [this](const QString& type) {
        m_transactionsView->openAddDialog(type);
    });
    connect(m_dashboardView, &DashboardView::requestOpenBudgets, this, [this]() {
        switchView(2);
    });
    connect(m_dashboardView, &DashboardView::requestOpenReports, this, [this]() {
        switchView(3);
    });

    // Default select Dashboard
    m_navButtons[0]->setChecked(true);
}

QPushButton* MainWindow::createNavButton(const QString& icon, const QString& title, int viewIndex) {
    QPushButton* btn = new QPushButton(QString("%1   %2").arg(icon, title), this);
    btn->setCheckable(true);
    btn->setFixedHeight(44);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setStyleSheet(
        "QPushButton {"
        "  background: transparent;"
        "  color: #94A3B8;"
        "  border: none;"
        "  border-radius: 8px;"
        "  text-align: left;"
        "  padding-left: 14px;"
        "  font-size: 14px;"
        "  font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "  background: #1E293B;"
        "  color: #F8FAFC;"
        "}"
        "QPushButton:checked {"
        "  background: #1E1B4B;"
        "  color: #818CF8;"
        "  font-weight: 700;"
        "  border-left: 3px solid #6366F1;"
        "}"
    );

    m_navGroup->addButton(btn, viewIndex);
    connect(btn, &QPushButton::clicked, this, [this, viewIndex]() {
        switchView(viewIndex);
    });

    return btn;
}

void MainWindow::switchView(int index) {
    if (index >= 0 && index < m_stackedWidget->count()) {
        m_stackedWidget->setCurrentIndex(index);
        if (index < m_navButtons.size()) {
            m_navButtons[index]->setChecked(true);
        }
    }
}

void MainWindow::onQuickAddTransaction() {
    TransactionDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        Transaction tx = dialog.getTransaction();
        if (DatabaseManager::instance().addTransaction(tx) > 0) {
            ToastNotification::showMessage(this, "Transaction recorded successfully!", ToastNotification::Success);
        }
    }
}

void MainWindow::setupShortcuts() {
    // Ctrl+N: Quick Add
    QShortcut* shortcutNew = new QShortcut(QKeySequence::New, this);
    connect(shortcutNew, &QShortcut::activated, this, &MainWindow::onQuickAddTransaction);

    // Ctrl+1 to Ctrl+5 for tabs
    for (int i = 0; i < 5; ++i) {
        QShortcut* tabShortcut = new QShortcut(QKeySequence(QString("Ctrl+%1").arg(i + 1)), this);
        connect(tabShortcut, &QShortcut::activated, this, [this, i]() {
            switchView(i);
        });
    }
}
