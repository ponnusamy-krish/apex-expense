#pragma once
#include <QObject>
#include <QSqlDatabase>
#include <QList>
#include <QMap>
#include <QDateTime>
#include "../models/Category.h"
#include "../models/Transaction.h"
#include "../models/Budget.h"

struct MonthlyStat {
    int year;
    int month;
    QString monthLabel; // e.g. "Oct 2026"
    double totalIncome = 0.0;
    double totalExpense = 0.0;
};

struct CategoryStat {
    QString name;
    QString color;
    QString icon;
    double totalAmount = 0.0;
    double percentage = 0.0;
};

class DatabaseManager : public QObject {
    Q_OBJECT
public:
    static DatabaseManager& instance();

    bool initialize(const QString& dbPath = "");
    QString databasePath() const { return m_dbPath; }

    // Categories
    QList<Category> getAllCategories();
    QList<Category> getCategories(bool isIncome);
    Category getCategoryById(int id);
    int addCategory(const Category& category);
    bool updateCategory(const Category& category);
    bool deleteCategory(int id);

    // Transactions
    QList<Transaction> getAllTransactions();
    QList<Transaction> getRecentTransactions(int limit = 10);
    QList<Transaction> getFilteredTransactions(const QDateTime& from, const QDateTime& to,
                                               int categoryId = -1,
                                               const QString& type = "All",
                                               const QString& paymentMethod = "All",
                                               const QString& searchQuery = "");
    Transaction getTransactionById(int id);
    int addTransaction(const Transaction& tx);
    bool updateTransaction(const Transaction& tx);
    bool deleteTransaction(int id);

    // Budgets
    QList<Budget> getBudgets(int month, int year);
    bool setBudget(int categoryId, double limit, int month, int year);
    bool deleteBudget(int id);

    // Analytics & Metrics
    void getFinancialTotals(const QDateTime& from, const QDateTime& to,
                            double& totalIncome, double& totalExpense);
    QList<CategoryStat> getCategoryExpenses(const QDateTime& from, const QDateTime& to);
    QList<MonthlyStat> getMonthlyTrends(int monthsCount = 6);
    QMap<QString, double> getPaymentMethodBreakdown(const QDateTime& from, const QDateTime& to);

    // Settings
    QString getSetting(const QString& key, const QString& defaultValue = "");
    bool setSetting(const QString& key, const QString& value);

    // Bulk / Demo operations
    void seedDefaultCategories();
    void loadSampleData();
    bool clearAllTransactionsAndBudgets();

signals:
    void dataChanged();

private:
    DatabaseManager(QObject* parent = nullptr);
    ~DatabaseManager();

    bool createTables();
    Transaction transactionFromQuery(class QSqlQuery& query);

    QSqlDatabase m_db;
    QString m_dbPath;
};
