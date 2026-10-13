#include "BudgetsView.h"
#include "../../database/DatabaseManager.h"
#include "../../utils/CurrencyFormatter.h"
#include "../dialogs/BudgetDialog.h"
#include "../components/ToastNotification.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QMessageBox>

BudgetsView::BudgetsView(QWidget* parent)
    : QWidget(parent), m_currentDate(QDate::currentDate())
{
    setupUi();
    connect(&DatabaseManager::instance(), &DatabaseManager::dataChanged, this, &BudgetsView::refreshData);
    refreshData();
}

void BudgetsView::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(18);

    // Top Header & Actions
    QHBoxLayout* topHeader = new QHBoxLayout();
    QVBoxLayout* titleLayout = new QVBoxLayout();

    QLabel* title = new QLabel("Monthly Budgets", this);
    title->setStyleSheet("font-size: 24px; font-weight: 800; color: #F8FAFC;");

    QLabel* subtitle = new QLabel("Track category spending limits and maintain your financial discipline", this);
    subtitle->setStyleSheet("font-size: 13px; color: #94A3B8; font-weight: 500;");

    titleLayout->addWidget(title);
    titleLayout->addWidget(subtitle);
    topHeader->addLayout(titleLayout);
    topHeader->addStretch();

    // Month Selector Navigator
    QHBoxLayout* navLayout = new QHBoxLayout();
    navLayout->setSpacing(8);

    QPushButton* prevBtn = new QPushButton("<", this);
    prevBtn->setFixedSize(36, 36);
    prevBtn->setStyleSheet("background: #1E293B; color: #F8FAFC; border: 1px solid #334155; border-radius: 8px; font-size: 16px; font-weight: bold; padding: 0;");
    connect(prevBtn, &QPushButton::clicked, this, &BudgetsView::onPrevMonth);

    m_monthLabel = new QLabel(this);
    m_monthLabel->setStyleSheet("font-size: 16px; font-weight: 700; color: #F8FAFC; padding: 0 14px;");

    QPushButton* nextBtn = new QPushButton(">", this);
    nextBtn->setFixedSize(36, 36);
    nextBtn->setStyleSheet("background: #1E293B; color: #F8FAFC; border: 1px solid #334155; border-radius: 8px; font-size: 16px; font-weight: bold; padding: 0;");
    connect(nextBtn, &QPushButton::clicked, this, &BudgetsView::onNextMonth);

    navLayout->addWidget(prevBtn);
    navLayout->addWidget(m_monthLabel);
    navLayout->addWidget(nextBtn);
    topHeader->addLayout(navLayout);

    QPushButton* addBudgetBtn = new QPushButton("+ Set Budget", this);
    addBudgetBtn->setFixedHeight(38);
    addBudgetBtn->setStyleSheet("background: #6366F1; color: #FFFFFF; border-radius: 8px; font-weight: 700; padding: 0 18px;");
    connect(addBudgetBtn, &QPushButton::clicked, this, &BudgetsView::openAddBudgetDialog);
    topHeader->addWidget(addBudgetBtn);

    mainLayout->addLayout(topHeader);

    // Summary Card
    QFrame* summaryCard = new QFrame(this);
    summaryCard->setStyleSheet("background: #1E293B; border: 1px solid #334155; border-radius: 12px; padding: 16px;");
    QVBoxLayout* sumLayout = new QVBoxLayout(summaryCard);
    sumLayout->setSpacing(10);

    m_summaryLabel = new QLabel(summaryCard);
    m_summaryLabel->setStyleSheet("font-size: 14px; font-weight: 700; color: #F8FAFC;");
    sumLayout->addWidget(m_summaryLabel);

    m_totalProgressBar = new QProgressBar(summaryCard);
    m_totalProgressBar->setFixedHeight(12);
    m_totalProgressBar->setTextVisible(false);
    m_totalProgressBar->setStyleSheet(
        "QProgressBar {"
        "  background: #334155;"
        "  border-radius: 6px;"
        "  border: none;"
        "}"
        "QProgressBar::chunk {"
        "  background: #6366F1;"
        "  border-radius: 6px;"
        "}"
    );
    sumLayout->addWidget(m_totalProgressBar);
    mainLayout->addWidget(summaryCard);

    // Scroll Area for Budget Cards
    QScrollArea* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("QScrollArea { border: none; background: transparent; }");

    m_cardsContainer = new QWidget();
    m_cardsContainer->setStyleSheet("background: transparent;");
    m_cardsLayout = new QVBoxLayout(m_cardsContainer);
    m_cardsLayout->setContentsMargins(0, 0, 0, 0);
    m_cardsLayout->setSpacing(12);
    m_cardsLayout->addStretch();

    scroll->setWidget(m_cardsContainer);
    mainLayout->addWidget(scroll);
}

void BudgetsView::onPrevMonth() {
    m_currentDate = m_currentDate.addMonths(-1);
    refreshData();
}

void BudgetsView::onNextMonth() {
    m_currentDate = m_currentDate.addMonths(1);
    refreshData();
}

void BudgetsView::refreshData() {
    m_monthLabel->setText(m_currentDate.toString("MMMM yyyy"));
    updateBudgetCards();
}

void BudgetsView::updateBudgetCards() {
    DatabaseManager& db = DatabaseManager::instance();
    CurrencyFormatter& cf = CurrencyFormatter::instance();

    int month = m_currentDate.month();
    int year = m_currentDate.year();

    QList<Budget> budgets = db.getBudgets(month, year);

    // Clear old items in layout (keep the trailing stretch)
    QLayoutItem* item;
    while ((item = m_cardsLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }

    double totalLimit = 0.0;
    double totalSpent = 0.0;

    if (budgets.isEmpty()) {
        QFrame* emptyCard = new QFrame(m_cardsContainer);
        emptyCard->setStyleSheet("background: #1E293B; border: 1px dashed #334155; border-radius: 12px; padding: 40px;");
        QVBoxLayout* empLayout = new QVBoxLayout(emptyCard);
        empLayout->setAlignment(Qt::AlignCenter);

        QLabel* emptyIcon = new QLabel("🎯", emptyCard);
        emptyIcon->setStyleSheet("font-size: 36px;");
        emptyIcon->setAlignment(Qt::AlignCenter);

        QLabel* emptyText = new QLabel(QString("No spending limits set for %1.").arg(m_currentDate.toString("MMMM yyyy")), emptyCard);
        emptyText->setStyleSheet("font-size: 15px; color: #94A3B8; font-weight: 600; margin-top: 10px;");
        emptyText->setAlignment(Qt::AlignCenter);

        QPushButton* setBtn = new QPushButton("Set First Budget", emptyCard);
        setBtn->setFixedSize(160, 36);
        setBtn->setStyleSheet("background: #6366F1; color: white; border-radius: 6px; font-weight: bold; margin-top: 15px;");
        connect(setBtn, &QPushButton::clicked, this, &BudgetsView::openAddBudgetDialog);

        empLayout->addWidget(emptyIcon);
        empLayout->addWidget(emptyText);
        empLayout->addWidget(setBtn, 0, Qt::AlignCenter);

        m_cardsLayout->addWidget(emptyCard);
    } else {
        for (const auto& b : budgets) {
            totalLimit += b.monthlyLimit;
            totalSpent += b.spentAmount;

            double pct = (b.monthlyLimit > 0.0) ? (b.spentAmount / b.monthlyLimit * 100.0) : 0.0;
            bool isOver = (b.spentAmount > b.monthlyLimit);

            QFrame* card = new QFrame(m_cardsContainer);
            card->setStyleSheet(QString(
                "QFrame {"
                "  background: #1E293B;"
                "  border: 1px solid %1;"
                "  border-radius: 12px;"
                "  padding: 14px 18px;"
                "}"
            ).arg(isOver ? "#E11D48" : "#334155"));

            QVBoxLayout* cLayout = new QVBoxLayout(card);
            cLayout->setSpacing(10);

            // Row 1: Category Info & Amounts
            QHBoxLayout* r1 = new QHBoxLayout();

            QLabel* catInfo = new QLabel(QString("<span style='font-size: 18px;'>%1</span> <b style='font-size: 15px; color: #F8FAFC;'>%2</b>").arg(b.categoryIcon, b.categoryName), card);
            r1->addWidget(catInfo);

            if (isOver) {
                QLabel* alertBadge = new QLabel("⚠️ OVER BUDGET", card);
                alertBadge->setStyleSheet("background: rgba(244, 63, 94, 0.2); color: #F43F5E; border: 1px solid #F43F5E; font-size: 11px; font-weight: 800; border-radius: 6px; padding: 3px 8px;");
                r1->addWidget(alertBadge);
            }

            r1->addStretch();

            QLabel* amountLabel = new QLabel(QString(
                "<span style='color: %1; font-weight: 700; font-size: 15px;'>%2</span> <span style='color: #64748B;'>/ %3</span>"
            ).arg(isOver ? "#F43F5E" : "#F8FAFC")
             .arg(cf.format(b.spentAmount))
             .arg(cf.format(b.monthlyLimit)), card);
            r1->addWidget(amountLabel);

            // Edit & Delete button
            QPushButton* editBtn = new QPushButton("Edit", card);
            editBtn->setFixedSize(50, 28);
            editBtn->setStyleSheet("background: #334155; color: #CBD5E1; border-radius: 4px; font-size: 11px; font-weight: 600;");
            connect(editBtn, &QPushButton::clicked, this, [this, b]() {
                BudgetDialog dlg(this, b.month, b.year, b);
                if (dlg.exec() == QDialog::Accepted) {
                    DatabaseManager::instance().setBudget(dlg.getCategoryId(), dlg.getLimit(), dlg.getMonth(), dlg.getYear());
                }
            });
            r1->addWidget(editBtn);

            QPushButton* delBtn = new QPushButton("✕", card);
            delBtn->setFixedSize(28, 28);
            delBtn->setStyleSheet("background: rgba(244, 63, 94, 0.15); color: #F43F5E; border-radius: 4px; font-weight: bold;");
            connect(delBtn, &QPushButton::clicked, this, [this, b]() {
                auto reply = QMessageBox::question(this, "Remove Budget", QString("Remove budget limit for %1?").arg(b.categoryName));
                if (reply == QMessageBox::Yes) {
                    DatabaseManager::instance().deleteBudget(b.id);
                }
            });
            r1->addWidget(delBtn);

            cLayout->addLayout(r1);

            // Row 2: Progress Bar
            QProgressBar* pBar = new QProgressBar(card);
            pBar->setFixedHeight(8);
            pBar->setTextVisible(false);
            pBar->setRange(0, 100);
            pBar->setValue(qMin(100, static_cast<int>(pct)));

            QString barColor = "#10B981"; // Emerald
            if (pct >= 100.0) {
                barColor = "#F43F5E"; // Red
            } else if (pct >= 75.0) {
                barColor = "#F59E0B"; // Amber
            }

            pBar->setStyleSheet(QString(
                "QProgressBar { background: #334155; border-radius: 4px; border: none; }"
                "QProgressBar::chunk { background: %1; border-radius: 4px; }"
            ).arg(barColor));

            cLayout->addWidget(pBar);

            // Row 3: Subtext
            QHBoxLayout* r3 = new QHBoxLayout();
            double remaining = b.monthlyLimit - b.spentAmount;
            QString subtext;
            if (remaining >= 0) {
                subtext = QString("%1 remaining (%2% used)").arg(cf.format(remaining)).arg(QString::number(pct, 'f', 1));
            } else {
                subtext = QString("<span style='color: #F43F5E;'>Exceeded by %1 (%2% used)</span>").arg(cf.format(qAbs(remaining))).arg(QString::number(pct, 'f', 1));
            }

            QLabel* subLabel = new QLabel(subtext, card);
            subLabel->setStyleSheet("font-size: 12px; color: #94A3B8; font-weight: 500;");
            r3->addWidget(subLabel);
            r3->addStretch();

            cLayout->addLayout(r3);
            m_cardsLayout->addWidget(card);
        }
    }

    m_cardsLayout->addStretch();

    // Update Overall Summary Bar
    double totalPct = (totalLimit > 0.0) ? (totalSpent / totalLimit * 100.0) : 0.0;
    m_totalProgressBar->setValue(qMin(100, static_cast<int>(totalPct)));

    QString totalColor = "#6366F1";
    if (totalPct >= 100.0) totalColor = "#F43F5E";
    else if (totalPct >= 80.0) totalColor = "#F59E0B";

    m_totalProgressBar->setStyleSheet(QString(
        "QProgressBar { background: #334155; border-radius: 6px; border: none; }"
        "QProgressBar::chunk { background: %1; border-radius: 6px; }"
    ).arg(totalColor));

    m_summaryLabel->setText(QString(
        "Total Budget: <b>%1</b>  |  Total Spent: <span style='color: %2;'><b>%3</b></span>  |  Remaining: <b>%4</b> (%5% Used)"
    ).arg(cf.format(totalLimit))
     .arg(totalSpent > totalLimit ? "#F43F5E" : "#10B981")
     .arg(cf.format(totalSpent))
     .arg(cf.format(totalLimit - totalSpent))
     .arg(QString::number(totalPct, 'f', 1)));
}

void BudgetsView::openAddBudgetDialog() {
    BudgetDialog dlg(this, m_currentDate.month(), m_currentDate.year());
    if (dlg.exec() == QDialog::Accepted) {
        if (DatabaseManager::instance().setBudget(dlg.getCategoryId(), dlg.getLimit(), dlg.getMonth(), dlg.getYear())) {
            ToastNotification::showMessage(window(), "Budget updated successfully!", ToastNotification::Success);
        }
    }
}
