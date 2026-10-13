#pragma once
#include <QString>

struct Category {
    int id = -1;
    QString name;
    QString color = "#6366F1";
    QString icon = "📁";
    bool isIncome = false;
};
