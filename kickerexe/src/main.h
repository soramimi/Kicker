#ifndef MAIN_H
#define MAIN_H

#include "Options.h"
#include "win.h"

#include <QApplication>


class MyApplication : public QApplication {
private:
public:
	Options options;
	MyApplication(int argc, char *argv[])
		: QApplication(argc, argv)
	{
	}

	KickerLibrary lib;

	QString application_data_dir;
};


#endif // MAIN_H
