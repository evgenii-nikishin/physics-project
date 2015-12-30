#include <QApplication>
#include <QTranslator>
#include "window.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	// Fixed russian translation
	QTranslator translator;
	if (translator.load("lorentz_ru", ":/resources/translations"))
		app.installTranslator(&translator);

	Window window;
	window.show();
	return app.exec();
}
