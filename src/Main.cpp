#include "MainWindow.h"

#include <QApplication>

int main(int argc, char* argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);  // automatic in Qt 6
#endif

    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("Calculator"));
    QApplication::setStyle(QStringLiteral("Fusion"));  // same rendering on every platform

    MainWindow window;
    window.show();
    return app.exec();
}