#include <Qt\qapplication.h>
#include "Window_Phi.h"

int main()
{
	int x=0;
	QApplication app(x, 0);

	Window_Phi window;
	return app.exec();
}
