#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QTextEdit>
#include <QPushButton>
#include "../../models/Transaction.h"

class TransactionDialog : public QDialog {
    Q_OBJECT
public:
    explicit TransactionDialog(QWidget* parent = nullptr, const Transaction& tx = Transaction(), bool isEdit = false);

    Transaction getTransaction() const;

private slots:
    void onTypeChanged(const QString& type);
    void validateAndAccept();

private:
    void populateCategories(const QString& type);

    Transaction m_transaction;
    bool m_isEdit;

    QPushButton* m_expenseBtn;
    QPushButton* m_incomeBtn;
    QString m_selectedType;

    QDoubleSpinBox* m_amountSpin;
    QComboBox* m_categoryCombo;
    QDateTimeEdit* m_dateTimeEdit;
    QComboBox* m_paymentCombo;
    QLineEdit* m_descEdit;
    QTextEdit* m_notesEdit;
};
