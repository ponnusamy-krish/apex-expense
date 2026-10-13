#include "HtmlReportGenerator.h"
#include "CurrencyFormatter.h"
#include "../database/DatabaseManager.h"
#include <QFile>
#include <QTextStream>

bool HtmlReportGenerator::generateReport(const QString& filePath,
                                        const QDateTime& from,
                                        const QDateTime& to,
                                        const QString& title) {
    DatabaseManager& db = DatabaseManager::instance();
    CurrencyFormatter& cf = CurrencyFormatter::instance();

    double totalIncome = 0.0, totalExpense = 0.0;
    db.getFinancialTotals(from, to, totalIncome, totalExpense);
    double netBalance = totalIncome - totalExpense;
    double savingsRate = (totalIncome > 0.0) ? ((totalIncome - totalExpense) / totalIncome * 100.0) : 0.0;
    if (savingsRate < 0.0) savingsRate = 0.0;

    QList<CategoryStat> catStats = db.getCategoryExpenses(from, to);
    QList<Transaction> transactions = db.getFilteredTransactions(from, to);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);

    out << "<!DOCTYPE html>\n"
        << "<html lang=\"en\">\n"
        << "<head>\n"
        << "<meta charset=\"UTF-8\">\n"
        << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
        << "<title>" << title << "</title>\n"
        << "<style>\n"
        << "  :root {\n"
        << "    --bg: #0F172A;\n"
        << "    --card-bg: #1E293B;\n"
        << "    --text-primary: #F8FAFC;\n"
        << "    --text-secondary: #94A3B8;\n"
        << "    --accent: #6366F1;\n"
        << "    --income: #10B981;\n"
        << "    --expense: #F43F5E;\n"
        << "    --border: #334155;\n"
        << "  }\n"
        << "  body {\n"
        << "    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif;\n"
        << "    background-color: #0F172A;\n"
        << "    color: #F8FAFC;\n"
        << "    margin: 0;\n"
        << "    padding: 30px;\n"
        << "    line-height: 1.5;\n"
        << "  }\n"
        << "  .container { max-width: 960px; margin: 0 auto; }\n"
        << "  .header { display: flex; justify-content: space-between; align-items: flex-end; border-bottom: 2px solid var(--border); padding-bottom: 20px; margin-bottom: 30px; }\n"
        << "  .title { font-size: 28px; font-weight: 700; color: #FFF; margin: 0; }\n"
        << "  .subtitle { font-size: 14px; color: var(--text-secondary); margin-top: 6px; }\n"
        << "  .metrics-grid { display: grid; grid-template-columns: repeat(4, 1fr); gap: 18px; margin-bottom: 35px; }\n"
        << "  .card { background: var(--card-bg); border: 1px solid var(--border); border-radius: 12px; padding: 20px; }\n"
        << "  .card-label { font-size: 13px; font-weight: 600; text-transform: uppercase; color: var(--text-secondary); letter-spacing: 0.5px; }\n"
        << "  .card-value { font-size: 26px; font-weight: 700; margin-top: 8px; }\n"
        << "  .val-income { color: var(--income); }\n"
        << "  .val-expense { color: var(--expense); }\n"
        << "  .val-net { color: " << (netBalance >= 0 ? "var(--income)" : "var(--expense)") << "; }\n"
        << "  .section-title { font-size: 20px; font-weight: 600; margin: 30px 0 15px 0; color: #FFF; border-left: 4px solid var(--accent); padding-left: 10px; }\n"
        << "  table { width: 100%; border-collapse: collapse; margin-bottom: 30px; background: var(--card-bg); border-radius: 10px; overflow: hidden; }\n"
        << "  th, td { padding: 12px 16px; text-align: left; border-bottom: 1px solid var(--border); font-size: 14px; }\n"
        << "  th { background: #162032; font-weight: 600; color: var(--text-secondary); text-transform: uppercase; font-size: 12px; letter-spacing: 0.5px; }\n"
        << "  tr:last-child td { border-bottom: none; }\n"
        << "  .badge { display: inline-block; padding: 4px 10px; border-radius: 12px; font-size: 12px; font-weight: 600; }\n"
        << "  .badge-income { background: rgba(16, 185, 129, 0.2); color: var(--income); }\n"
        << "  .badge-expense { background: rgba(244, 63, 94, 0.2); color: var(--expense); }\n"
        << "  .cat-pill { display: inline-flex; align-items: center; gap: 6px; padding: 3px 8px; border-radius: 6px; background: #334155; font-size: 13px; }\n"
        << "  .progress-bg { background: #334155; border-radius: 6px; height: 10px; width: 100%; overflow: hidden; }\n"
        << "  .progress-bar { height: 100%; border-radius: 6px; }\n"
        << "  .footer { text-align: center; color: var(--text-secondary); font-size: 12px; margin-top: 40px; border-top: 1px solid var(--border); padding-top: 20px; }\n"
        << "  @media print {\n"
        << "    body { background: #FFF !important; color: #000 !important; padding: 0; }\n"
        << "    .card, table { background: #FFF !important; border: 1px solid #CCC !important; color: #000 !important; }\n"
        << "    th { background: #EEE !important; color: #000 !important; }\n"
        << "    td { color: #000 !important; }\n"
        << "  }\n"
        << "</style>\n"
        << "</head>\n"
        << "<body>\n"
        << "<div class=\"container\">\n"
        << "  <div class=\"header\">\n"
        << "    <div>\n"
        << "      <h1 class=\"title\">" << title << "</h1>\n"
        << "      <div class=\"subtitle\">Period: "
        << (from.isValid() ? from.toString("MMM d, yyyy") : "Beginning")
        << " &mdash; "
        << (to.isValid() ? to.toString("MMM d, yyyy") : "Present")
        << " | Generated: " << QDateTime::currentDateTime().toString("MMM d, yyyy HH:mm") << "</div>\n"
        << "    </div>\n"
        << "  </div>\n"

        // KPI Cards
        << "  <div class=\"metrics-grid\">\n"
        << "    <div class=\"card\">\n"
        << "      <div class=\"card-label\">Total Income</div>\n"
        << "      <div class=\"card-value val-income\">" << cf.format(totalIncome) << "</div>\n"
        << "    </div>\n"
        << "    <div class=\"card\">\n"
        << "      <div class=\"card-label\">Total Expenses</div>\n"
        << "      <div class=\"card-value val-expense\">" << cf.format(totalExpense) << "</div>\n"
        << "    </div>\n"
        << "    <div class=\"card\">\n"
        << "      <div class=\"card-label\">Net Balance</div>\n"
        << "      <div class=\"card-value val-net\">" << cf.format(netBalance) << "</div>\n"
        << "    </div>\n"
        << "    <div class=\"card\">\n"
        << "      <div class=\"card-label\">Savings Rate</div>\n"
        << "      <div class=\"card-value\" style=\"color: #6366F1;\">" << QString::number(savingsRate, 'f', 1) << "%</div>\n"
        << "    </div>\n"
        << "  </div>\n"

        // Category Breakdown
        << "  <div class=\"section-title\">Category Spending Breakdown</div>\n"
        << "  <table>\n"
        << "    <thead>\n"
        << "      <tr>\n"
        << "        <th>Category</th>\n"
        << "        <th>Amount</th>\n"
        << "        <th>Share</th>\n"
        << "        <th style=\"width: 35%;\">Distribution</th>\n"
        << "      </tr>\n"
        << "    </thead>\n"
        << "    <tbody>\n";

    if (catStats.isEmpty()) {
        out << "      <tr><td colspan=\"4\" style=\"text-align: center; color: var(--text-secondary);\">No expense records found for this period.</td></tr>\n";
    } else {
        for (const auto& cs : catStats) {
            out << "      <tr>\n"
                << "        <td><span class=\"cat-pill\">" << cs.icon << " " << cs.name << "</span></td>\n"
                << "        <td><strong>" << cf.format(cs.totalAmount) << "</strong></td>\n"
                << "        <td>" << QString::number(cs.percentage, 'f', 1) << "%</td>\n"
                << "        <td>\n"
                << "          <div class=\"progress-bg\">\n"
                << "            <div class=\"progress-bar\" style=\"width: " << cs.percentage << "%; background: " << cs.color << ";\"></div>\n"
                << "          </div>\n"
                << "        </td>\n"
                << "      </tr>\n";
        }
    }

    out << "    </tbody>\n"
        << "  </table>\n"

        // Transactions Table
        << "  <div class=\"section-title\">Transaction Ledger (" << transactions.size() << " records)</div>\n"
        << "  <table>\n"
        << "    <thead>\n"
        << "      <tr>\n"
        << "        <th>Date</th>\n"
        << "        <th>Type</th>\n"
        << "        <th>Category</th>\n"
        << "        <th>Description</th>\n"
        << "        <th>Payment Method</th>\n"
        << "        <th style=\"text-align: right;\">Amount</th>\n"
        << "      </tr>\n"
        << "    </thead>\n"
        << "    <tbody>\n";

    if (transactions.isEmpty()) {
        out << "      <tr><td colspan=\"6\" style=\"text-align: center; color: var(--text-secondary);\">No transactions found.</td></tr>\n";
    } else {
        for (const auto& t : transactions) {
            bool isInc = (t.type == "Income");
            out << "      <tr>\n"
                << "        <td>" << t.dateTime.toString("yyyy-MM-dd") << "</td>\n"
                << "        <td><span class=\"badge " << (isInc ? "badge-income" : "badge-expense") << "\">" << t.type << "</span></td>\n"
                << "        <td>" << t.categoryIcon << " " << t.categoryName << "</td>\n"
                << "        <td>" << t.description << "</td>\n"
                << "        <td>" << t.paymentMethod << "</td>\n"
                << "        <td style=\"text-align: right; font-weight: 700; color: " << (isInc ? "var(--income)" : "var(--expense)") << ";\">"
                << cf.formatSigned(t.amount, t.type)
                << "</td>\n"
                << "      </tr>\n";
        }
    }

    out << "    </tbody>\n"
        << "  </table>\n"

        << "  <div class=\"footer\">Generated with Lumina ApexExpense Tracker &bull; Confidential Personal Finance</div>\n"
        << "</div>\n"
        << "</body>\n"
        << "</html>\n";

    file.close();
    return true;
}
