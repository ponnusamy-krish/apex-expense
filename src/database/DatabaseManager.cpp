#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QDate>
#include <cmath>

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager(QObject* parent) : QObject(parent) {
}

DatabaseManager::~DatabaseManager() {
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool DatabaseManager::initialize(const QString& dbPath) {
    if (dbPath.isEmpty()) {
        QString appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        if (appDataDir.isEmpty()) {
            appDataDir = QDir::currentPath();
        }
        QDir dir(appDataDir);
        if (!dir.exists()) {
            dir.mkpath(".");
        }
        m_dbPath = dir.filePath("expenses.db");
    } else {
        m_dbPath = dbPath;
    }

    // Ensure directory exists
    QFileInfo fileInfo(m_dbPath);
    QDir().mkpath(fileInfo.absolutePath());

    m_db = QSqlDatabase::addDatabase("QSQLITE", "expense_tracker_conn");
    m_db.setDatabaseName(m_dbPath);

    if (!m_db.open()) {
        qCritical() << "Failed to open SQLite database:" << m_db.lastError().text();
        return false;
    }

    // Enable foreign keys
    QSqlQuery pragmaQuery(m_db);
    pragmaQuery.exec("PRAGMA foreign_keys = ON;");

    if (!createTables()) {
        return false;
    }

    // Check if categories table is empty and seed if necessary
    QSqlQuery checkCat("SELECT COUNT(*) FROM categories", m_db);
    if (checkCat.next() && checkCat.value(0).toInt() == 0) {
        seedDefaultCategories();
    }

    return true;
}

bool DatabaseManager::createTables() {
    QSqlQuery query(m_db);

    bool ok = query.exec(
        "CREATE TABLE IF NOT EXISTS categories ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL UNIQUE,"
        "color TEXT NOT NULL,"
        "icon TEXT NOT NULL,"
        "is_income INTEGER NOT NULL DEFAULT 0"
        ");"
    );
    if (!ok) {
        qCritical() << "Error creating categories table:" << query.lastError().text();
        return false;
    }

    ok = query.exec(
        "CREATE TABLE IF NOT EXISTS transactions ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "date_time TEXT NOT NULL,"
        "type TEXT NOT NULL," // 'Expense' or 'Income'
        "category_id INTEGER NOT NULL,"
        "amount REAL NOT NULL,"
        "payment_method TEXT NOT NULL,"
        "description TEXT NOT NULL,"
        "notes TEXT,"
        "FOREIGN KEY (category_id) REFERENCES categories(id) ON DELETE RESTRICT"
        ");"
    );
    if (!ok) {
        qCritical() << "Error creating transactions table:" << query.lastError().text();
        return false;
    }

    ok = query.exec(
        "CREATE TABLE IF NOT EXISTS budgets ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "category_id INTEGER NOT NULL,"
        "monthly_limit REAL NOT NULL,"
        "month INTEGER NOT NULL,"
        "year INTEGER NOT NULL,"
        "UNIQUE(category_id, month, year),"
        "FOREIGN KEY (category_id) REFERENCES categories(id) ON DELETE CASCADE"
        ");"
    );
    if (!ok) {
        qCritical() << "Error creating budgets table:" << query.lastError().text();
        return false;
    }

    ok = query.exec(
        "CREATE TABLE IF NOT EXISTS settings ("
        "key TEXT PRIMARY KEY,"
        "value TEXT NOT NULL"
        ");"
    );
    if (!ok) {
        qCritical() << "Error creating settings table:" << query.lastError().text();
        return false;
    }

    return true;
}

void DatabaseManager::seedDefaultCategories() {
    struct CatDef {
        QString name;
        QString color;
        QString icon;
        int isIncome;
    };

    QList<CatDef> defs = {
        {"Food & Dining", "#EF4444", "🍔", 0},
        {"Groceries", "#F59E0B", "🛒", 0},
        {"Housing & Rent", "#3B82F6", "🏠", 0},
        {"Utilities", "#06B6D4", "⚡", 0},
        {"Transportation", "#8B5CF6", "🚗", 0},
        {"Shopping", "#EC4899", "🛍️", 0},
        {"Entertainment", "#F97316", "🎬", 0},
        {"Healthcare", "#10B981", "💊", 0},
        {"Education", "#6366F1", "📚", 0},
        {"Travel", "#14B8A6", "✈️", 0},
        {"Other Expense", "#64748B", "📦", 0},
        {"Salary", "#10B981", "💼", 1},
        {"Freelance", "#059669", "💻", 1},
        {"Investments", "#84CC16", "📈", 1},
        {"Bonus & Gifts", "#22C55E", "🎁", 1},
        {"Other Income", "#34D399", "💰", 1}
    };

    QSqlQuery q(m_db);
    q.prepare("INSERT OR IGNORE INTO categories (name, color, icon, is_income) VALUES (?, ?, ?, ?)");
    for (const auto& item : defs) {
        q.addBindValue(item.name);
        q.addBindValue(item.color);
        q.addBindValue(item.icon);
        q.addBindValue(item.isIncome);
        q.exec();
    }
}

QList<Category> DatabaseManager::getAllCategories() {
    QList<Category> list;
    QSqlQuery q("SELECT id, name, color, icon, is_income FROM categories ORDER BY is_income ASC, name ASC", m_db);
    while (q.next()) {
        Category c;
        c.id = q.value(0).toInt();
        c.name = q.value(1).toString();
        c.color = q.value(2).toString();
        c.icon = q.value(3).toString();
        c.isIncome = (q.value(4).toInt() == 1);
        list.append(c);
    }
    return list;
}

QList<Category> DatabaseManager::getCategories(bool isIncome) {
    QList<Category> list;
    QSqlQuery q(m_db);
    q.prepare("SELECT id, name, color, icon, is_income FROM categories WHERE is_income = ? ORDER BY name ASC");
    q.addBindValue(isIncome ? 1 : 0);
    q.exec();
    while (q.next()) {
        Category c;
        c.id = q.value(0).toInt();
        c.name = q.value(1).toString();
        c.color = q.value(2).toString();
        c.icon = q.value(3).toString();
        c.isIncome = (q.value(4).toInt() == 1);
        list.append(c);
    }
    return list;
}

Category DatabaseManager::getCategoryById(int id) {
    Category c;
    QSqlQuery q(m_db);
    q.prepare("SELECT id, name, color, icon, is_income FROM categories WHERE id = ?");
    q.addBindValue(id);
    if (q.exec() && q.next()) {
        c.id = q.value(0).toInt();
        c.name = q.value(1).toString();
        c.color = q.value(2).toString();
        c.icon = q.value(3).toString();
        c.isIncome = (q.value(4).toInt() == 1);
    }
    return c;
}

int DatabaseManager::addCategory(const Category& category) {
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO categories (name, color, icon, is_income) VALUES (?, ?, ?, ?)");
    q.addBindValue(category.name.trimmed());
    q.addBindValue(category.color);
    q.addBindValue(category.icon);
    q.addBindValue(category.isIncome ? 1 : 0);
    if (q.exec()) {
        emit dataChanged();
        return q.lastInsertId().toInt();
    }
    return -1;
}

bool DatabaseManager::updateCategory(const Category& category) {
    QSqlQuery q(m_db);
    q.prepare("UPDATE categories SET name = ?, color = ?, icon = ?, is_income = ? WHERE id = ?");
    q.addBindValue(category.name.trimmed());
    q.addBindValue(category.color);
    q.addBindValue(category.icon);
    q.addBindValue(category.isIncome ? 1 : 0);
    q.addBindValue(category.id);
    if (q.exec()) {
        emit dataChanged();
        return true;
    }
    return false;
}

bool DatabaseManager::deleteCategory(int id) {
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM categories WHERE id = ?");
    q.addBindValue(id);
    if (q.exec()) {
        emit dataChanged();
        return true;
    }
    return false;
}

Transaction DatabaseManager::transactionFromQuery(QSqlQuery& q) {
    Transaction t;
    t.id = q.value(0).toInt();
    t.dateTime = QDateTime::fromString(q.value(1).toString(), Qt::ISODate);
    t.type = q.value(2).toString();
    t.categoryId = q.value(3).toInt();
    t.amount = q.value(4).toDouble();
    t.paymentMethod = q.value(5).toString();
    t.description = q.value(6).toString();
    t.notes = q.value(7).toString();
    t.categoryName = q.value(8).toString();
    t.categoryColor = q.value(9).toString();
    t.categoryIcon = q.value(10).toString();
    return t;
}

QList<Transaction> DatabaseManager::getAllTransactions() {
    QList<Transaction> list;
    QString sql =
        "SELECT t.id, t.date_time, t.type, t.category_id, t.amount, t.payment_method, "
        "t.description, t.notes, c.name, c.color, c.icon "
        "FROM transactions t "
        "LEFT JOIN categories c ON t.category_id = c.id "
        "ORDER BY t.date_time DESC, t.id DESC";
    QSqlQuery q(sql, m_db);
    while (q.next()) {
        list.append(transactionFromQuery(q));
    }
    return list;
}

QList<Transaction> DatabaseManager::getRecentTransactions(int limit) {
    QList<Transaction> list;
    QString sql = QString(
        "SELECT t.id, t.date_time, t.type, t.category_id, t.amount, t.payment_method, "
        "t.description, t.notes, c.name, c.color, c.icon "
        "FROM transactions t "
        "LEFT JOIN categories c ON t.category_id = c.id "
        "ORDER BY t.date_time DESC, t.id DESC "
        "LIMIT %1").arg(limit);
    QSqlQuery q(sql, m_db);
    while (q.next()) {
        list.append(transactionFromQuery(q));
    }
    return list;
}

QList<Transaction> DatabaseManager::getFilteredTransactions(const QDateTime& from, const QDateTime& to,
                                                           int categoryId,
                                                           const QString& type,
                                                           const QString& paymentMethod,
                                                           const QString& searchQuery) {
    QList<Transaction> list;
    QString sql =
        "SELECT t.id, t.date_time, t.type, t.category_id, t.amount, t.payment_method, "
        "t.description, t.notes, c.name, c.color, c.icon "
        "FROM transactions t "
        "LEFT JOIN categories c ON t.category_id = c.id "
        "WHERE 1=1 ";

    if (from.isValid()) {
        sql += " AND t.date_time >= :from";
    }
    if (to.isValid()) {
        sql += " AND t.date_time <= :to";
    }
    if (categoryId > 0) {
        sql += " AND t.category_id = :catId";
    }
    if (!type.isEmpty() && type != "All") {
        sql += " AND t.type = :type";
    }
    if (!paymentMethod.isEmpty() && paymentMethod != "All") {
        sql += " AND t.payment_method = :payMethod";
    }
    if (!searchQuery.trimmed().isEmpty()) {
        sql += " AND (t.description LIKE :search OR t.notes LIKE :search)";
    }

    sql += " ORDER BY t.date_time DESC, t.id DESC";

    QSqlQuery q(m_db);
    q.prepare(sql);
    if (from.isValid()) q.bindValue(":from", from.toString(Qt::ISODate));
    if (to.isValid()) q.bindValue(":to", to.toString(Qt::ISODate));
    if (categoryId > 0) q.bindValue(":catId", categoryId);
    if (!type.isEmpty() && type != "All") q.bindValue(":type", type);
    if (!paymentMethod.isEmpty() && paymentMethod != "All") q.bindValue(":payMethod", paymentMethod);
    if (!searchQuery.trimmed().isEmpty()) q.bindValue(":search", QString("%%1%").arg(searchQuery.trimmed()));

    if (q.exec()) {
        while (q.next()) {
            list.append(transactionFromQuery(q));
        }
    }
    return list;
}

Transaction DatabaseManager::getTransactionById(int id) {
    Transaction t;
    QString sql =
        "SELECT t.id, t.date_time, t.type, t.category_id, t.amount, t.payment_method, "
        "t.description, t.notes, c.name, c.color, c.icon "
        "FROM transactions t "
        "LEFT JOIN categories c ON t.category_id = c.id "
        "WHERE t.id = :id";
    QSqlQuery q(m_db);
    q.prepare(sql);
    q.bindValue(":id", id);
    if (q.exec() && q.next()) {
        t = transactionFromQuery(q);
    }
    return t;
}

int DatabaseManager::addTransaction(const Transaction& tx) {
    QSqlQuery q(m_db);
    q.prepare(
        "INSERT INTO transactions (date_time, type, category_id, amount, payment_method, description, notes) "
        "VALUES (?, ?, ?, ?, ?, ?, ?)"
    );
    q.addBindValue(tx.dateTime.toString(Qt::ISODate));
    q.addBindValue(tx.type);
    q.addBindValue(tx.categoryId);
    q.addBindValue(tx.amount);
    q.addBindValue(tx.paymentMethod);
    q.addBindValue(tx.description.trimmed());
    q.addBindValue(tx.notes.trimmed());

    if (q.exec()) {
        emit dataChanged();
        return q.lastInsertId().toInt();
    } else {
        qWarning() << "Failed to add transaction:" << q.lastError().text();
    }
    return -1;
}

bool DatabaseManager::updateTransaction(const Transaction& tx) {
    QSqlQuery q(m_db);
    q.prepare(
        "UPDATE transactions SET date_time = ?, type = ?, category_id = ?, amount = ?, "
        "payment_method = ?, description = ?, notes = ? WHERE id = ?"
    );
    q.addBindValue(tx.dateTime.toString(Qt::ISODate));
    q.addBindValue(tx.type);
    q.addBindValue(tx.categoryId);
    q.addBindValue(tx.amount);
    q.addBindValue(tx.paymentMethod);
    q.addBindValue(tx.description.trimmed());
    q.addBindValue(tx.notes.trimmed());
    q.addBindValue(tx.id);

    if (q.exec()) {
        emit dataChanged();
        return true;
    }
    return false;
}

bool DatabaseManager::deleteTransaction(int id) {
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM transactions WHERE id = ?");
    q.addBindValue(id);
    if (q.exec()) {
        emit dataChanged();
        return true;
    }
    return false;
}

QList<Budget> DatabaseManager::getBudgets(int month, int year) {
    QList<Budget> list;
    QString sql =
        "SELECT b.id, b.category_id, b.monthly_limit, b.month, b.year, c.name, c.color, c.icon "
        "FROM budgets b "
        "JOIN categories c ON b.category_id = c.id "
        "WHERE b.month = ? AND b.year = ? "
        "ORDER BY c.name ASC";
    QSqlQuery q(m_db);
    q.prepare(sql);
    q.addBindValue(month);
    q.addBindValue(year);

    if (q.exec()) {
        while (q.next()) {
            Budget b;
            b.id = q.value(0).toInt();
            b.categoryId = q.value(1).toInt();
            b.monthlyLimit = q.value(2).toDouble();
            b.month = q.value(3).toInt();
            b.year = q.value(4).toInt();
            b.categoryName = q.value(5).toString();
            b.categoryColor = q.value(6).toString();
            b.categoryIcon = q.value(7).toString();

            // Calculate spent amount for this category in this month/year
            QDateTime startOfMonth(QDate(year, month, 1).startOfDay());
            QDateTime endOfMonth(QDate(year, month, QDate(year, month, 1).daysInMonth()).endOfDay());

            QSqlQuery spentQ(m_db);
            spentQ.prepare(
                "SELECT COALESCE(SUM(amount), 0) FROM transactions "
                "WHERE category_id = ? AND type = 'Expense' "
                "AND date_time >= ? AND date_time <= ?"
            );
            spentQ.addBindValue(b.categoryId);
            spentQ.addBindValue(startOfMonth.toString(Qt::ISODate));
            spentQ.addBindValue(endOfMonth.toString(Qt::ISODate));
            if (spentQ.exec() && spentQ.next()) {
                b.spentAmount = spentQ.value(0).toDouble();
            }

            list.append(b);
        }
    }
    return list;
}

bool DatabaseManager::setBudget(int categoryId, double limit, int month, int year) {
    QSqlQuery q(m_db);
    q.prepare(
        "INSERT INTO budgets (category_id, monthly_limit, month, year) "
        "VALUES (?, ?, ?, ?) "
        "ON CONFLICT(category_id, month, year) DO UPDATE SET monthly_limit = excluded.monthly_limit"
    );
    q.addBindValue(categoryId);
    q.addBindValue(limit);
    q.addBindValue(month);
    q.addBindValue(year);

    if (q.exec()) {
        emit dataChanged();
        return true;
    }
    return false;
}

bool DatabaseManager::deleteBudget(int id) {
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM budgets WHERE id = ?");
    q.addBindValue(id);
    if (q.exec()) {
        emit dataChanged();
        return true;
    }
    return false;
}

void DatabaseManager::getFinancialTotals(const QDateTime& from, const QDateTime& to,
                                        double& totalIncome, double& totalExpense) {
    totalIncome = 0.0;
    totalExpense = 0.0;

    QString sql =
        "SELECT type, COALESCE(SUM(amount), 0) FROM transactions "
        "WHERE 1=1 ";
    if (from.isValid()) sql += " AND date_time >= :from";
    if (to.isValid()) sql += " AND date_time <= :to";
    sql += " GROUP BY type";

    QSqlQuery q(m_db);
    q.prepare(sql);
    if (from.isValid()) q.bindValue(":from", from.toString(Qt::ISODate));
    if (to.isValid()) q.bindValue(":to", to.toString(Qt::ISODate));

    if (q.exec()) {
        while (q.next()) {
            QString type = q.value(0).toString();
            double sum = q.value(1).toDouble();
            if (type == "Income") {
                totalIncome = sum;
            } else {
                totalExpense = sum;
            }
        }
    }
}

QList<CategoryStat> DatabaseManager::getCategoryExpenses(const QDateTime& from, const QDateTime& to) {
    QList<CategoryStat> list;
    double totalExpense = 0.0;
    double dummyIncome = 0.0;
    getFinancialTotals(from, to, dummyIncome, totalExpense);

    QString sql =
        "SELECT c.name, c.color, c.icon, COALESCE(SUM(t.amount), 0) as total "
        "FROM transactions t "
        "JOIN categories c ON t.category_id = c.id "
        "WHERE t.type = 'Expense' ";
    if (from.isValid()) sql += " AND t.date_time >= :from";
    if (to.isValid()) sql += " AND t.date_time <= :to";
    sql += " GROUP BY c.id ORDER BY total DESC";

    QSqlQuery q(m_db);
    q.prepare(sql);
    if (from.isValid()) q.bindValue(":from", from.toString(Qt::ISODate));
    if (to.isValid()) q.bindValue(":to", to.toString(Qt::ISODate));

    if (q.exec()) {
        while (q.next()) {
            CategoryStat cs;
            cs.name = q.value(0).toString();
            cs.color = q.value(1).toString();
            cs.icon = q.value(2).toString();
            cs.totalAmount = q.value(3).toDouble();
            cs.percentage = (totalExpense > 0.0) ? (cs.totalAmount / totalExpense * 100.0) : 0.0;
            list.append(cs);
        }
    }
    return list;
}

QList<MonthlyStat> DatabaseManager::getMonthlyTrends(int monthsCount) {
    QList<MonthlyStat> list;
    QDate currentDate = QDate::currentDate();

    for (int i = monthsCount - 1; i >= 0; --i) {
        QDate dt = currentDate.addMonths(-i);
        int year = dt.year();
        int month = dt.month();

        QDateTime start(QDate(year, month, 1).startOfDay());
        QDateTime end(QDate(year, month, dt.daysInMonth()).endOfDay());

        MonthlyStat stat;
        stat.year = year;
        stat.month = month;
        stat.monthLabel = dt.toString("MMM yyyy");

        getFinancialTotals(start, end, stat.totalIncome, stat.totalExpense);
        list.append(stat);
    }

    return list;
}

QMap<QString, double> DatabaseManager::getPaymentMethodBreakdown(const QDateTime& from, const QDateTime& to) {
    QMap<QString, double> map;
    QString sql =
        "SELECT payment_method, COALESCE(SUM(amount), 0) "
        "FROM transactions "
        "WHERE type = 'Expense' ";
    if (from.isValid()) sql += " AND date_time >= :from";
    if (to.isValid()) sql += " AND date_time <= :to";
    sql += " GROUP BY payment_method ORDER BY SUM(amount) DESC";

    QSqlQuery q(m_db);
    q.prepare(sql);
    if (from.isValid()) q.bindValue(":from", from.toString(Qt::ISODate));
    if (to.isValid()) q.bindValue(":to", to.toString(Qt::ISODate));

    if (q.exec()) {
        while (q.next()) {
            map.insert(q.value(0).toString(), q.value(1).toDouble());
        }
    }
    return map;
}

QString DatabaseManager::getSetting(const QString& key, const QString& defaultValue) {
    QSqlQuery q(m_db);
    q.prepare("SELECT value FROM settings WHERE key = ?");
    q.addBindValue(key);
    if (q.exec() && q.next()) {
        return q.value(0).toString();
    }
    return defaultValue;
}

bool DatabaseManager::setSetting(const QString& key, const QString& value) {
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO settings (key, value) VALUES (?, ?) ON CONFLICT(key) DO UPDATE SET value = excluded.value");
    q.addBindValue(key);
    q.addBindValue(value);
    return q.exec();
}

bool DatabaseManager::clearAllTransactionsAndBudgets() {
    QSqlQuery q(m_db);
    bool ok1 = q.exec("DELETE FROM transactions;");
    bool ok2 = q.exec("DELETE FROM budgets;");
    if (ok1 && ok2) {
        emit dataChanged();
        return true;
    }
    return false;
}

void DatabaseManager::loadSampleData() {
    clearAllTransactionsAndBudgets();

    // Map categories to IDs
    QMap<QString, int> catMap;
    for (const auto& cat : getAllCategories()) {
        catMap.insert(cat.name, cat.id);
    }

    QDate today = QDate::currentDate();
    int curYear = today.year();
    int curMonth = today.month();

    // Set sample budgets for this month
    if (catMap.contains("Food & Dining")) setBudget(catMap["Food & Dining"], 450.0, curMonth, curYear);
    if (catMap.contains("Groceries")) setBudget(catMap["Groceries"], 600.0, curMonth, curYear);
    if (catMap.contains("Housing & Rent")) setBudget(catMap["Housing & Rent"], 1500.0, curMonth, curYear);
    if (catMap.contains("Utilities")) setBudget(catMap["Utilities"], 220.0, curMonth, curYear);
    if (catMap.contains("Transportation")) setBudget(catMap["Transportation"], 250.0, curMonth, curYear);
    if (catMap.contains("Shopping")) setBudget(catMap["Shopping"], 350.0, curMonth, curYear);
    if (catMap.contains("Entertainment")) setBudget(catMap["Entertainment"], 180.0, curMonth, curYear);
    if (catMap.contains("Healthcare")) setBudget(catMap["Healthcare"], 150.0, curMonth, curYear);

    struct SampleItem {
        int daysAgo;
        QString type;
        QString cat;
        double amount;
        QString pay;
        QString desc;
        QString notes;
    };

    QList<SampleItem> samples = {
        // Incomes
        {1, "Income", "Salary", 4500.00, "Bank Transfer", "Monthly Tech Corp Salary", "Direct deposit"},
        {12, "Income", "Freelance", 850.00, "PayPal", "UI/UX Design Contract", "Client payment"},
        {20, "Income", "Investments", 320.50, "Bank Transfer", "Quarterly Dividend Payout", "Index funds"},
        {35, "Income", "Salary", 4500.00, "Bank Transfer", "Previous Month Salary", "Direct deposit"},
        {42, "Income", "Bonus & Gifts", 300.00, "Cash", "Birthday Gift from Family", ""},

        // Fixed expenses
        {2, "Expense", "Housing & Rent", 1450.00, "Bank Transfer", "Apartment Monthly Rent", "Downtown loft"},
        {4, "Expense", "Utilities", 115.40, "Credit Card", "Electric & Power Bill", "City Power"},
        {5, "Expense", "Utilities", 65.00, "Debit Card", "High-Speed Fiber Internet", "Fiber ISP"},

        // Groceries & Food
        {0, "Expense", "Food & Dining", 24.50, "Credit Card", "Lunch at Ramen Bar", "Spicy miso ramen"},
        {1, "Expense", "Groceries", 142.80, "Debit Card", "Supermarket Weekly Run", "Fresh produce & organic milk"},
        {3, "Expense", "Food & Dining", 48.00, "Credit Card", "Dinner with Colleagues", "Italian Bistro"},
        {6, "Expense", "Groceries", 78.20, "Credit Card", "Farmers Market Groceries", "Local organic produce"},
        {8, "Expense", "Food & Dining", 16.75, "Cash", "Artisan Coffee & Pastries", "Morning espresso"},
        {10, "Expense", "Groceries", 112.90, "Debit Card", "Costco Bulk Goods", "Pantry essentials"},

        // Transport
        {2, "Expense", "Transportation", 45.00, "Credit Card", "Gasoline Fuel Refill", "Shell Station"},
        {7, "Expense", "Transportation", 28.50, "Credit Card", "Uber Ride to Airport", "Late night commute"},
        {14, "Expense", "Transportation", 65.00, "Debit Card", "Metro Monthly Transit Pass", ""},

        // Shopping & Entertainment
        {3, "Expense", "Shopping", 129.99, "Credit Card", "Running Shoes", "Nike Pegasus 40"},
        {9, "Expense", "Entertainment", 17.99, "Credit Card", "Streaming Subscriptions", "Netflix & Spotify bundle"},
        {11, "Expense", "Entertainment", 62.00, "Credit Card", "Cinema IMAX Tickets", "2 tickets + popcorn"},
        {15, "Expense", "Shopping", 45.50, "Credit Card", "Books & Stationery", "B&N purchase"},

        // Health & Education
        {5, "Expense", "Healthcare", 40.00, "Debit Card", "Pharmacy Prescription", "Vitamin supplements"},
        {13, "Expense", "Healthcare", 85.00, "Credit Card", "Dental Routine Cleaning", "Dr. Miller clinic"},
        {18, "Expense", "Education", 29.99, "Credit Card", "Online C++ Masterclass", "Udemy course"},

        // Older transactions for trends
        {32, "Expense", "Housing & Rent", 1450.00, "Bank Transfer", "Last Month Rent", ""},
        {34, "Expense", "Groceries", 390.00, "Debit Card", "Monthly Groceries Total", ""},
        {38, "Expense", "Food & Dining", 280.00, "Credit Card", "Dining & Restaurants", ""},
        {45, "Expense", "Transportation", 180.00, "Credit Card", "Gas & Public Transit", ""},
        {50, "Expense", "Shopping", 210.00, "Credit Card", "Autumn Clothing", ""}
    };

    for (const auto& s : samples) {
        if (!catMap.contains(s.cat)) continue;
        Transaction t;
        t.dateTime = QDateTime(today.addDays(-s.daysAgo), QTime(12, 30));
        t.type = s.type;
        t.categoryId = catMap[s.cat];
        t.amount = s.amount;
        t.paymentMethod = s.pay;
        t.description = s.desc;
        t.notes = s.notes;
        addTransaction(t);
    }

    emit dataChanged();
}
