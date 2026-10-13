#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>
#include <QLabel>
#include "../../models/Transaction.h"

class TransactionsView : public QWidget {
    Q_OBJECT
public:
    explicit TransactionsView(QWidget* parent = nullptr);

public slots:
    void refreshData();
    void openAddDialog(const QString& defaultType = "Expense");

private slots:
    void onSearchChanged();
    void onDateFilterChanged(int index);
    void onEditClicked();
    void onDeleteClicked();
    void onExportCsvClicked();
    void onImportCsvClicked();
    void onTableRowDoubleClicked(int row, int column);

private:
    void setupUi();
    void populateCategories();
    void updateTable();

    QLineEdit* m_searchEdit;
    QComboBox* m_datePresetCombo;
    QDateEdit* m_startDateEdit;
    QDateEdit* m_endDateEdit;
    QComboBox* m_categoryCombo;
    QComboBox* m_typeCombo;
    QComboBox* m_paymentCombo;

    QTableWidget* m_table;
    QLabel* m_summaryLabel;

    QList<Transaction> m_currentTransactions;
};
