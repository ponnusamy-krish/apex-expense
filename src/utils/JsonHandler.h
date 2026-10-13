#pragma once
#include <QString>

struct JsonImportResult {
    bool success = false;
    int importedTransactions = 0;
    int importedBudgets = 0;
    int importedCategories = 0;
    QString errorMessage;
};

class JsonHandler {
public:
    static bool exportBackup(const QString& filePath);
    static JsonImportResult importBackup(const QString& filePath, bool merge = true);
};
