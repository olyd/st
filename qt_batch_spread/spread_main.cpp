#include <QApplication>

#include "spread_batch.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	MyDlg *dialog = new MyDlg;
	return dialog->exec();
}
