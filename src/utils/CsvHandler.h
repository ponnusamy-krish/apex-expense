#pragma once
#include <QString>
#include <QList>
#include "../models/Transaction.h"

struct CsvImportResult {
    bool success = false;
    int importedCount = 0;
    int skippedCount = 0;
    QString errorMessage;
};

class CsvHandler {
public:
    static bool exportTransactions(const QString& filePath, const QList<Transaction>& transactions);
    static CsvImportResult importTransactions(const QString& filePath);

private:
    static QString escapeCsvField(const QString& field);
    static QStringList parseCsvLine(const QString& line);
};
