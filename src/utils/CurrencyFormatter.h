#pragma once
#include <QString>

class CurrencyFormatter {
public:
    static CurrencyFormatter& instance();

    void setCurrencySymbol(const QString& symbol);
    QString currencySymbol() const;

    QString format(double amount, bool showSign = false) const;
    QString formatSigned(double amount, const QString& type) const;

private:
    CurrencyFormatter();
    QString m_symbol = "$";
};
