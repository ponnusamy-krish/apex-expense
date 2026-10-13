#include "JsonHandler.h"
#include "../database/DatabaseManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QMap>

bool JsonHandler::exportBackup(const QString& filePath) {
    DatabaseManager& db = DatabaseManager::instance();

    QJsonObject rootObj;
    rootObj["version"] = "1.0";
    rootObj["exportTimestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    rootObj["appName"] = "ApexExpense";

    // Categories
    QJsonArray catArray;
    QList<Category> categories = db.getAllCategories();
    for (const auto& c : categories) {
        QJsonObject obj;
        obj["id"] = c.id;
        obj["name"] = c.name;
        obj["color"] = c.color;
        obj["icon"] = c.icon;
        obj["isIncome"] = c.isIncome;
        catArray.append(obj);
    }
    rootObj["categories"] = catArray;

    // Budgets
    QJsonArray budgetArray;
    QDate today = QDate::currentDate();
    // Export recent budgets for current and past months
    for (int m = 1; m <= 12; ++m) {
        QList<Budget> budgets = db.getBudgets(m, today.year());
        for (const auto& b : budgets) {
            QJsonObject obj;
            obj["categoryId"] = b.categoryId;
            obj["categoryName"] = b.categoryName;
            obj["monthlyLimit"] = b.monthlyLimit;
            obj["month"] = b.month;
            obj["year"] = b.year;
            budgetArray.append(obj);
        }
    }
    rootObj["budgets"] = budgetArray;

    // Transactions
    QJsonArray txArray;
    QList<Transaction> transactions = db.getAllTransactions();
    for (const auto& t : transactions) {
        QJsonObject obj;
        obj["id"] = t.id;
        obj["dateTime"] = t.dateTime.toString(Qt::ISODate);
        obj["type"] = t.type;
        obj["categoryId"] = t.categoryId;
        obj["categoryName"] = t.categoryName;
        obj["amount"] = t.amount;
        obj["paymentMethod"] = t.paymentMethod;
        obj["description"] = t.description;
        obj["notes"] = t.notes;
        txArray.append(obj);
    }
    rootObj["transactions"] = txArray;

    QJsonDocument doc(rootObj);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

JsonImportResult JsonHandler::importBackup(const QString& filePath, bool merge) {
    JsonImportResult result;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        result.errorMessage = "Unable to open backup file.";
        return result;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        result.errorMessage = QString("Invalid JSON format: %1").arg(parseError.errorString());
        return result;
    }

    QJsonObject root = doc.object();
    DatabaseManager& db = DatabaseManager::instance();

    if (!merge) {
        db.clearAllTransactionsAndBudgets();
    }

    // Map existing categories by name
    QMap<QString, int> categoryMap;
    for (const auto& c : db.getAllCategories()) {
        categoryMap.insert(c.name.toLower(), c.id);
    }

    // Import Categories
    if (root.contains("categories") && root["categories"].isArray()) {
        QJsonArray catArray = root["categories"].toArray();
        for (const auto& val : catArray) {
            QJsonObject obj = val.toObject();
            QString name = obj["name"].toString().trimmed();
            if (name.isEmpty()) continue;

            if (!categoryMap.contains(name.toLower())) {
                Category c;
                c.name = name;
                c.color = obj.contains("color") ? obj["color"].toString() : "#6366F1";
                c.icon = obj.contains("icon") ? obj["icon"].toString() : "📁";
                c.isIncome = obj["isIncome"].toBool();
                int id = db.addCategory(c);
                if (id > 0) {
                    categoryMap.insert(name.toLower(), id);
                    result.importedCategories++;
                }
            }
        }
    }

    // Import Budgets
    if (root.contains("budgets") && root["budgets"].isArray()) {
        QJsonArray budgetArray = root["budgets"].toArray();
        for (const auto& val : budgetArray) {
            QJsonObject obj = val.toObject();
            QString catName = obj["categoryName"].toString().trimmed();
            int catId = -1;
            if (categoryMap.contains(catName.toLower())) {
                catId = categoryMap[catName.toLower()];
            } else if (obj.contains("categoryId")) {
                catId = obj["categoryId"].toInt();
            }

            if (catId > 0) {
                double limit = obj["monthlyLimit"].toDouble();
                int month = obj["month"].toInt();
                int year = obj["year"].toInt();
                if (db.setBudget(catId, limit, month, year)) {
                    result.importedBudgets++;
                }
            }
        }
    }

    // Import Transactions
    if (root.contains("transactions") && root["transactions"].isArray()) {
        QJsonArray txArray = root["transactions"].toArray();
        for (const auto& val : txArray) {
            QJsonObject obj = val.toObject();
            Transaction t;
            t.dateTime = QDateTime::fromString(obj["dateTime"].toString(), Qt::ISODate);
            if (!t.dateTime.isValid()) t.dateTime = QDateTime::currentDateTime();

            t.type = obj["type"].toString("Expense");
            t.amount = obj["amount"].toDouble();
            t.paymentMethod = obj["paymentMethod"].toString("Other");
            t.description = obj["description"].toString();
            t.notes = obj["notes"].toString();

            QString catName = obj["categoryName"].toString().trimmed();
            if (categoryMap.contains(catName.toLower())) {
                t.categoryId = categoryMap[catName.toLower()];
            } else if (obj.contains("categoryId")) {
                t.categoryId = obj["categoryId"].toInt();
            } else {
                t.categoryId = 1;
            }

            if (t.amount > 0.0 && db.addTransaction(t) > 0) {
                result.importedTransactions++;
            }
        }
    }

    result.success = true;
    return result;
}
