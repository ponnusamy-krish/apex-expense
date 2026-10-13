#include "CurrencyFormatter.h"
#include <QLocale>

CurrencyFormatter& CurrencyFormatter::instance() {
    static CurrencyFormatter inst;
    return inst;
}

CurrencyFormatter::CurrencyFormatter() {
}

void CurrencyFormatter::setCurrencySymbol(const QString& symbol) {
    m_symbol = symbol.trimmed().isEmpty() ? "$" : symbol;
}

QString CurrencyFormatter::currencySymbol() const {
    return m_symbol;
}

QString CurrencyFormatter::format(double amount, bool showSign) const {
    QLocale locale(QLocale::English);
    QString numStr = locale.toString(qAbs(amount), 'f', 2);
    if (showSign) {
        if (amount > 0.0001) {
            return QString("+%1%2").arg(m_symbol, numStr);
        } else if (amount < -0.0001) {
            return QString("-%1%2").arg(m_symbol, numStr);
        }
    }
    if (amount < -0.0001) {
        return QString("-%1%2").arg(m_symbol, numStr);
    }
    return QString("%1%2").arg(m_symbol, numStr);
}

QString CurrencyFormatter::formatSigned(double amount, const QString& type) const {
    QLocale locale(QLocale::English);
    QString numStr = locale.toString(qAbs(amount), 'f', 2);
    if (type.compare("Income", Qt::CaseInsensitive) == 0) {
        return QString("+%1%2").arg(m_symbol, numStr);
    } else {
        return QString("-%1%2").arg(m_symbol, numStr);
    }
}
