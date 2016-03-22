#include "main.h"
#include "MainWindow.h"
#include <QMessageBox>
#include <QTranslator>
#include "MySettings.h"

#include "Options.h"

MainWindow *the_mainwindow;
Options *the_options;
QString applicationDataDir()
{
	MyApplication *a = (MyApplication *)qApp;
	return a->application_data_dir;
}


int main(int argc, char *argv[])
{
	MyApplication a(argc, argv);
	the_options = &a.options;

#if defined(Q_OS_WIN)
	QApplication::addLibraryPath(qApp->applicationDirPath());
#endif

#if USE_SPLASH
	QSplashScreen splash(QPixmap(":/image/about.png"));
	splash.show();
	qApp->processEvents();
#endif

	int ret = 0;

	HANDLE mutex = CreateMutexA(0, TRUE, "mutex://soramimi.jp/kicker");
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		ret = 1;
	} else {
		QSettings::setDefaultFormat(QSettings::IniFormat);

		a.setOrganizationName("soramimi.jp");
		a.setApplicationName("Kicker");

		QTranslator translator;
		{
			QString path = "Kicker_ja";
			translator.load(path, a.applicationDirPath());
			a.installTranslator(&translator);
		}

		a.application_data_dir = makeApplicationDataDir();
		if (a.application_data_dir.isEmpty()) {
			QMessageBox::warning(0, qApp->applicationName(), "Preparation of data storage folder failed.");
			ret = 1;
		} else {
			the_options->load();

			MainWindow w(&a);
			the_mainwindow = &w;
			w.setWindowIcon(QIcon(":/image/kicker.png"));
//			w.show();
			if (!a.lib.load()) {
				QMessageBox::warning(&w, qApp->applicationName(), "Could not load the kicker.dll");
				ret = 1;
			} else {
				a.lib.setup(w.getDecoyWindow());
				a.lib.enable(true);
				a.installNativeEventFilter(&a.lib);
//				w.hide();
				a.exec();
				ret = 0;
			}
		}
	}
	ReleaseMutex(mutex);
	CloseHandle(mutex);
	return ret;
}
