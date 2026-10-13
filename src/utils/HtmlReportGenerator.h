#pragma once
#include <QString>
#include <QDateTime>

class HtmlReportGenerator {
public:
    static bool generateReport(const QString& filePath,
                               const QDateTime& from,
                               const QDateTime& to,
                               const QString& title = "Expense & Financial Report");
};
