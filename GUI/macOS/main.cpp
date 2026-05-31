#include <QApplication>
#include "LWMainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("LWZip");
    app.setApplicationVersion("0.1.0");
    app.setOrganizationName("LWZip");

    LWMainWindow window;
    window.show();

    // Open archive from command line argument
    if (argc > 1)
        window.openFile(QString::fromUtf8(argv[1]));

    return app.exec();
}
