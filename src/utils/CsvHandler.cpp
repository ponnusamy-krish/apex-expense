#include "CsvHandler.h"
#include "../database/DatabaseManager.h"
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMap>
#include <QRegularExpression>

QString CsvHandler::escapeCsvField(const QString& field) {
    QString res = field;
    if (res.contains(',') || res.contains('"') || res.contains('\n') || res.contains('\r')) {
        res.replace("\"", "\"\"");
        return QString("\"%1\"").arg(res);
    }
    return res;
}

QStringList CsvHandler::parseCsvLine(const QString& line) {
    QStringList result;
    QString current;
    bool inQuotes = false;

    for (int i = 0; i < line.length(); ++i) {
        QChar c = line[i];
        if (c == '"') {
            if (inQuotes && i + 1 < line.length() && line[i + 1] == '"') {
                current += '"';
                ++i; // skip escaped quote
            } else {
                inQuotes = !inQuotes;
            }
        } else if (c == ',' && !inQuotes) {
            result.append(current.trimmed());
            current.clear();
        } else {
            current += c;
        }
    }
    result.append(current.trimmed());
    return result;
}

bool CsvHandler::exportTransactions(const QString& filePath, const QList<Transaction>& transactions) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    // Write header
    out << "ID,Date,Time,Type,Category,Amount,PaymentMethod,Description,Notes\n";

    for (const auto& t : transactions) {
        out << t.id << ","
            << t.dateTime.toString("yyyy-MM-dd") << ","
            << t.dateTime.toString("HH:mm:ss") << ","
            << escapeCsvField(t.type) << ","
            << escapeCsvField(t.categoryName) << ","
            << QString::number(t.amount, 'f', 2) << ","
            << escapeCsvField(t.paymentMethod) << ","
            << escapeCsvField(t.description) << ","
            << escapeCsvField(t.notes) << "\n";
    }

    file.close();
    return true;
}

CsvImportResult CsvHandler::importTransactions(const QString& filePath) {
    CsvImportResult result;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        result.errorMessage = "Unable to open file for reading.";
        return result;
    }

    QTextStream in(&file);
    if (in.atEnd()) {
        result.errorMessage = "File is empty.";
        return result;
    }

    // Cache categories map (name -> id)
    DatabaseManager& db = DatabaseManager::instance();
    QMap<QString, int> categoryMap;
    for (const auto& c : db.getAllCategories()) {
        categoryMap.insert(c.name.toLower(), c.id);
    }

    // Read header line
    QString headerLine = in.readLine();
    QStringList headers = parseCsvLine(headerLine);

    // Map column names to index
    int colDate = -1, colTime = -1, colType = -1, colCategory = -1;
    int colAmount = -1, colPayment = -1, colDesc = -1, colNotes = -1;

    for (int i = 0; i < headers.size(); ++i) {
        QString h = headers[i].trimmed().toLower();
        if (h == "date") colDate = i;
        else if (h == "time") colTime = i;
        else if (h == "type") colType = i;
        else if (h == "category") colCategory = i;
        else if (h == "amount") colAmount = i;
        else if (h == "paymentmethod" || h == "payment_method" || h == "payment method" || h == "payment") colPayment = i;
        else if (h == "description" || h == "title" || h == "merchant") colDesc = i;
        else if (h == "notes" || h == "note" || h == "memo") colNotes = i;
    }

    if (colAmount == -1) {
        result.errorMessage = "CSV must have at least an 'Amount' column.";
        return result;
    }

    int defaultExpenseCatId = -1;
    int defaultIncomeCatId = -1;
    if (!categoryMap.isEmpty()) {
        defaultExpenseCatId = categoryMap.first();
        defaultIncomeCatId = categoryMap.first();
        if (categoryMap.contains("other expense")) defaultExpenseCatId = categoryMap["other expense"];
        if (categoryMap.contains("other income")) defaultIncomeCatId = categoryMap["other income"];
    }

    int rowNum = 1;
    while (!in.atEnd()) {
        rowNum++;
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList cols = parseCsvLine(line);
        if (cols.size() <= colAmount) {
            result.skippedCount++;
            continue;
        }

        Transaction t;

        // Parse Amount
        bool ok = false;
        QString cleanAmount = cols[colAmount];
        cleanAmount.remove(QRegularExpression("[^0-9.-]"));
        t.amount = qAbs(cleanAmount.toDouble(&ok));
        if (!ok || t.amount <= 0.0) {
            result.skippedCount++;
            continue;
        }

        // Parse Type
        if (colType >= 0 && colType < cols.size() && !cols[colType].isEmpty()) {
            QString typeStr = cols[colType].trimmed();
            if (typeStr.compare("Income", Qt::CaseInsensitive) == 0) {
                t.type = "Income";
            } else {
                t.type = "Expense";
            }
        } else {
            // Default to Expense unless amount is signed or specified
            t.type = "Expense";
        }

        // Parse Date & Time
        QDate date = QDate::currentDate();
        QTime time(12, 0, 0);

        if (colDate >= 0 && colDate < cols.size() && !cols[colDate].isEmpty()) {
            QString dStr = cols[colDate].trimmed();
            QDate parsedDate = QDate::fromString(dStr, "yyyy-MM-dd");
            if (!parsedDate.isValid()) parsedDate = QDate::fromString(dStr, "MM/dd/yyyy");
            if (!parsedDate.isValid()) parsedDate = QDate::fromString(dStr, "dd/MM/yyyy");
            if (!parsedDate.isValid()) parsedDate = QDate::fromString(dStr, Qt::ISODate);
            if (parsedDate.isValid()) date = parsedDate;
        }

        if (colTime >= 0 && colTime < cols.size() && !cols[colTime].isEmpty()) {
            QTime parsedTime = QTime::fromString(cols[colTime].trimmed(), "HH:mm:ss");
            if (!parsedTime.isValid()) parsedTime = QTime::fromString(cols[colTime].trimmed(), "HH:mm");
            if (parsedTime.isValid()) time = parsedTime;
        }

        t.dateTime = QDateTime(date, time);

        // Parse Category
        QString catName = (colCategory >= 0 && colCategory < cols.size()) ? cols[colCategory].trimmed() : "";
        if (!catName.isEmpty()) {
            QString lowerCat = catName.toLower();
            if (categoryMap.contains(lowerCat)) {
                t.categoryId = categoryMap[lowerCat];
            } else {
                // Auto create new category
                Category newCat;
                newCat.name = catName;
                newCat.color = (t.type == "Income") ? "#10B981" : "#6366F1";
                newCat.icon = "🏷️";
                newCat.isIncome = (t.type == "Income");
                int newId = db.addCategory(newCat);
                if (newId > 0) {
                    categoryMap.insert(lowerCat, newId);
                    t.categoryId = newId;
                } else {
                    t.categoryId = (t.type == "Income") ? defaultIncomeCatId : defaultExpenseCatId;
                }
            }
        } else {
            t.categoryId = (t.type == "Income") ? defaultIncomeCatId : defaultExpenseCatId;
        }

        // Parse Payment Method
        if (colPayment >= 0 && colPayment < cols.size() && !cols[colPayment].isEmpty()) {
            t.paymentMethod = cols[colPayment].trimmed();
        } else {
            t.paymentMethod = "Other";
        }

        // Parse Description
        if (colDesc >= 0 && colDesc < cols.size() && !cols[colDesc].isEmpty()) {
            t.description = cols[colDesc].trimmed();
        } else {
            t.description = QString("Imported %1").arg(t.type);
        }

        // Parse Notes
        if (colNotes >= 0 && colNotes < cols.size()) {
            t.notes = cols[colNotes].trimmed();
        }

        if (db.addTransaction(t) > 0) {
            result.importedCount++;
        } else {
            result.skippedCount++;
        }
    }

    file.close();
    result.success = (result.importedCount > 0);
    return result;
}
