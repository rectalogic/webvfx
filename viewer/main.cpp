 #include <QApplication>
 #include "viewer.h"

 int main(int argc, char *argv[])
 {
    QApplication app(argc, argv);

    app.setOrganizationDomain("webvfx.org");
    app.setOrganizationName("WebVFX");
    app.setApplicationName("Viewer");

    Viewer viewer;
    viewer.show();
    return app.exec();
 }

