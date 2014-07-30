#include <Qt\qapplication.h>
#include "Window_NB.h"

int main()
{
	int x=0;
	QApplication app(x, 0);

	Window_NB window;
	return app.exec();
}