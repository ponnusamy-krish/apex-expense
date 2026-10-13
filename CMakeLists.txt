cmake_minimum_required(VERSION 3.16)
project(ApexExpense VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Configure prefix path for local Qt5 installation
list(PREPEND CMAKE_PREFIX_PATH
    "/home/mount/.local/qt5/usr/lib/x86_64-linux-gnu/cmake"
    "/home/mount/.local/qt5/usr"
)

# Ensure RPATH includes local libraries for portable execution
set(CMAKE_BUILD_RPATH
    "/home/mount/.local/qt5/usr/lib/x86_64-linux-gnu"
    "/usr/lib/x86_64-linux-gnu"
)
set(CMAKE_INSTALL_RPATH
    "/home/mount/.local/qt5/usr/lib/x86_64-linux-gnu"
    "/usr/lib/x86_64-linux-gnu"
)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

find_package(Qt5 REQUIRED COMPONENTS Widgets Charts Sql Gui Core)

set(SOURCES
    src/main.cpp
    src/database/DatabaseManager.cpp
    src/utils/CurrencyFormatter.cpp
    src/utils/CsvHandler.cpp
    src/utils/JsonHandler.cpp
    src/utils/HtmlReportGenerator.cpp
    src/ui/components/MetricCard.cpp
    src/ui/components/CategoryPill.cpp
    src/ui/components/ToastNotification.cpp
    src/ui/dialogs/TransactionDialog.cpp
    src/ui/dialogs/BudgetDialog.cpp
    src/ui/dialogs/CategoryDialog.cpp
    src/ui/views/DashboardView.cpp
    src/ui/views/TransactionsView.cpp
    src/ui/views/BudgetsView.cpp
    src/ui/views/AnalyticsView.cpp
    src/ui/views/SettingsView.cpp
    src/ui/MainWindow.cpp
)

set(HEADERS
    src/models/Category.h
    src/models/Transaction.h
    src/models/Budget.h
    src/database/DatabaseManager.h
    src/utils/CurrencyFormatter.h
    src/utils/CsvHandler.h
    src/utils/JsonHandler.h
    src/utils/HtmlReportGenerator.h
    src/ui/components/MetricCard.h
    src/ui/components/CategoryPill.h
    src/ui/components/ToastNotification.h
    src/ui/dialogs/TransactionDialog.h
    src/ui/dialogs/BudgetDialog.h
    src/ui/dialogs/CategoryDialog.h
    src/ui/views/DashboardView.h
    src/ui/views/TransactionsView.h
    src/ui/views/BudgetsView.h
    src/ui/views/AnalyticsView.h
    src/ui/views/SettingsView.h
    src/ui/MainWindow.h
)

set(RESOURCES
    resources/app.qrc
)

add_executable(ApexExpense ${SOURCES} ${HEADERS} ${RESOURCES})

target_include_directories(ApexExpense PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(ApexExpense PRIVATE
    Qt5::Widgets
    Qt5::Charts
    Qt5::Sql
    Qt5::Gui
    Qt5::Core
)
