#ifndef TRANSPARENTWINDOW_H
#define TRANSPARENTWINDOW_H

#include <QDialog>

namespace Ui {
class DecoyWindow;
}

class DecoyWindow : public QWidget
{
	Q_OBJECT

public:
	explicit DecoyWindow(QWidget *parent = 0);
	~DecoyWindow();

	void updateLayout();

	virtual void paintEvent(QPaintEvent *);

	virtual void timerEvent(QTimerEvent *);

private:
	Ui::DecoyWindow *ui;
};


#endif // TRANSPARENTWINDOW_H
