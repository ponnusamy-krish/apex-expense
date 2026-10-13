#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include "../../models/Category.h"

class CategoryDialog : public QDialog {
    Q_OBJECT
public:
    explicit CategoryDialog(QWidget* parent = nullptr, const Category& initialCat = Category());
    Category getCategory() const;

private:
    Category m_category;
    QLineEdit* m_nameEdit;
    QComboBox* m_typeCombo;
    QComboBox* m_iconCombo;
    QComboBox* m_colorCombo;
};
