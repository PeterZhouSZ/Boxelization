#include "boxelization.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	Boxelization w;
	w.show();
	return a.exec();
}
