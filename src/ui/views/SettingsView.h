#pragma once
#include <QWidget>
#include <QComboBox>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>

class SettingsView : public QWidget {
    Q_OBJECT
public:
    explicit SettingsView(QWidget* parent = nullptr);

public slots:
    void refreshData();

private slots:
    void onCurrencyChanged(const QString& symbol);
    void onExportJsonBackup();
    void onImportJsonBackup();
    void onLoadSampleData();
    void onClearAllData();
    void onAddCategory();
    void onDeleteCategory();

private:
    void setupUi();
    void updateCategoriesTable();

    QComboBox* m_currencyCombo;
    QLabel* m_dbLocationLabel;
    QTableWidget* m_categoriesTable;
};
