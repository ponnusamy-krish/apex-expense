#include <QApplication>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QStyleFactory>
#include <QPalette>
#include <QComboBox>
#include "database/DatabaseManager.h"
#include "utils/CurrencyFormatter.h"
#include "ui/MainWindow.h"

int main(int argc, char *argv[]) {
    // Ensure Qt plugins are discovered from local toolchain prefix and system
    QCoreApplication::addLibraryPath("/home/mount/.local/qt5/usr/lib/x86_64-linux-gnu/qt5/plugins");
    QCoreApplication::addLibraryPath("/usr/lib/x86_64-linux-gnu/qt5/plugins");
    QCoreApplication::addLibraryPath(QDir::currentPath() + "/plugins");

    // Enable High DPI scaling
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication app(argc, argv);
    app.setApplicationName("ApexExpense");
    app.setApplicationDisplayName("ApexExpense — Financial Tracker");
    app.setOrganizationName("LuminaTech");
    app.setApplicationVersion("1.0.0");

    // Enforce Fusion style and dark palette so native popups/dropdowns are never in light mode
    app.setStyle(QStyleFactory::create("Fusion"));
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor("#0F172A"));
    darkPalette.setColor(QPalette::WindowText, QColor("#F8FAFC"));
    darkPalette.setColor(QPalette::Base, QColor("#1E293B"));
    darkPalette.setColor(QPalette::AlternateBase, QColor("#0F172A"));
    darkPalette.setColor(QPalette::ToolTipBase, QColor("#1E293B"));
    darkPalette.setColor(QPalette::ToolTipText, QColor("#F8FAFC"));
    darkPalette.setColor(QPalette::Text, QColor("#F8FAFC"));
    darkPalette.setColor(QPalette::Button, QColor("#1E293B"));
    darkPalette.setColor(QPalette::ButtonText, QColor("#F8FAFC"));
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor("#6366F1"));
    darkPalette.setColor(QPalette::Highlight, QColor("#6366F1"));
    darkPalette.setColor(QPalette::HighlightedText, Qt::white);
    app.setPalette(darkPalette);

    // Load stylesheet (try embedded resource first, then local filesystem)
    QFile qssFile(":/styles/modern_dark.qss");
    if (!qssFile.exists()) {
        qssFile.setFileName(QDir::currentPath() + "/resources/styles/modern_dark.qss");
    }
    if (qssFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        app.setStyleSheet(qssFile.readAll());
        qssFile.close();
    }

    // Initialize Database
    DatabaseManager& db = DatabaseManager::instance();
    if (!db.initialize()) {
        qCritical() << "Fatal: Could not initialize database!";
        return 1;
    }

    // Restore saved currency
    QString savedCurrency = db.getSetting("currency", "$");
    CurrencyFormatter::instance().setCurrencySymbol(savedCurrency);

    // If new database with 0 transactions, seed sample data for immediate stunning showcase
    if (db.getAllTransactions().isEmpty()) {
        db.loadSampleData();
    }

    MainWindow mainWindow;
    mainWindow.show();

    // Support --snapshot <filepath> [--tab <index>] [--open-combo] for verification and visual preview
    int targetTab = 0;
    bool openCombo = false;
    for (int i = 1; i < argc; ++i) {
        if (QString(argv[i]) == "--tab" && i + 1 < argc) {
            targetTab = QString(argv[i + 1]).toInt();
        }
        if (QString(argv[i]) == "--open-combo") {
            openCombo = true;
        }
    }
    if (targetTab > 0) {
        mainWindow.switchView(targetTab);
    }

    for (int i = 1; i < argc; ++i) {
        if (QString(argv[i]) == "--snapshot" && i + 1 < argc) {
            QString outPath = argv[i + 1];
            mainWindow.resize(1280, 800);
            app.processEvents();
            if (openCombo) {
                QComboBox* combo = mainWindow.findChild<QComboBox*>("currencyCombo");
                if (combo) {
                    combo->showPopup();
                    app.processEvents();
                }
            }
            QPixmap pixmap = mainWindow.grab();
            pixmap.save(outPath);
            qDebug() << "Snapshot saved to:" << outPath;
            return 0;
        }
    }

    return app.exec();
}
