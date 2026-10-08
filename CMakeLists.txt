cmake_minimum_required(VERSION 3.16)
project(QtCalculator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# Calculation logic: plain C++17, no Qt.
add_library(calc_core STATIC
    src/core/CalculatorEngine.cpp
    src/core/CalculatorEngine.h
    src/core/ExpressionEvaluator.cpp
    src/core/ExpressionEvaluator.h
)
target_include_directories(calc_core PUBLIC src/core)

# The GUI application.
add_executable(Calculator WIN32 MACOSX_BUNDLE
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/DisplayWidget.cpp
    src/DisplayWidget.h
    src/CalcButton.cpp
    src/CalcButton.h
    src/Theme.h
)
target_link_libraries(Calculator PRIVATE calc_core Qt${QT_VERSION_MAJOR}::Widgets)

# Logic tests (no GUI): run `ctest` or the core_tests executable.
option(CALC_BUILD_TESTS "Build the calculator logic tests" ON)
if(CALC_BUILD_TESTS)
    enable_testing()
    add_executable(core_tests tests/core_tests.cpp)
    target_link_libraries(core_tests PRIVATE calc_core)
    add_test(NAME core_tests COMMAND core_tests)
endif()