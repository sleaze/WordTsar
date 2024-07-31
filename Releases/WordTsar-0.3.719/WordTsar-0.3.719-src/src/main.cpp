#include <QApplication>
#include <QSplashScreen>
#include <QPixmap>
#include <QTimer>

#include "src/wordtsar.h"
#include "src/core/include/version.h"

#ifdef DO_TEST
#include "src/test/test.h"
#endif

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QPixmap pixmap(":/gui/images/splash.png") ;
    QSplashScreen splash(pixmap) ;
    splash.setWindowFlag(Qt::WindowStaysOnTopHint, true) ;
    QString vers = FULLVERSION_STRING ;
    vers += " " ;
    vers += STATUS ;
    splash.showMessage(vers, Qt::AlignBottom | Qt::AlignCenter, QColor(152, 114, 14)) ;
    splash.show() ;
    app.processEvents();

    cWordTsar w(argc, argv);

    QTimer::singleShot(5000, &splash, SLOT(close())) ;

    w.show();
    app.processEvents();

    // filename as argument
    if(argc > 1)
    {
        QString arg(argv[1]) ;
        app.processEvents() ;

        w.LoadFile(arg) ;
    }

#ifdef DO_TEST
    cTest test ;
    test.StartTest() ;
#endif

    return app.exec();
}
