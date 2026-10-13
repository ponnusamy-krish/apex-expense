#pragma once
#include <QString>
#include <QDateTime>

struct Transaction {
    int id = -1;
    QDateTime dateTime = QDateTime::currentDateTime();
    QString type = "Expense"; // "Expense" or "Income"
    int categoryId = 1;
    QString categoryName;
    QString categoryColor = "#6366F1";
    QString categoryIcon = "📁";
    double amount = 0.0;
    QString paymentMethod = "Credit Card";
    QString description;
    QString notes;
};
