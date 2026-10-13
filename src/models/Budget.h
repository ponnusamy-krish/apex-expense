#pragma once
#include <QString>

struct Budget {
    int id = -1;
    int categoryId = 1;
    QString categoryName;
    QString categoryColor = "#6366F1";
    QString categoryIcon = "📁";
    double monthlyLimit = 0.0;
    int month = 1; // 1 - 12
    int year = 2026;
    double spentAmount = 0.0; // Dynamic property computed from transactions
};
