#include "DecoyWindow.h"
#include "ui_DecoyWindow.h"

#include "win.h"
//#include "../KickerLite/common.h"
#include "common.h"

DecoyWindow::DecoyWindow(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::DecoyWindow)
{
//	ui->setupUi(this);
	setWindowFlags(Qt::Dialog | Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus);
	setCursor(Qt::BlankCursor);
	this->setWindowOpacity(1.0 / 16);
	updateLayout();
	startTimer(1000);
}

DecoyWindow::~DecoyWindow()
{
	delete ui;
}

void DecoyWindow::updateLayout()
{
	QSize sz = getPrimaryScreenSize();
	setGeometry(sz.width() - 2, sz.height() - 2, 1, 1);
//	setGeometry(sz.width() - 17, sz.height() - 17, 16, 16);
}

#include <QPainter>

void DecoyWindow::paintEvent(QPaintEvent *)
{
	QPainter pr(this);
//	pr.fillRect(0, 0, width(), height(), Qt::red);
	pr.fillRect(0, 0, width(), height(), QColor(128, 128, 128));
}

#include <windows.h>
#include "MainWindow.h"

void DecoyWindow::timerEvent(QTimerEvent *)
{
	HWND w = GetWindow((HWND)winId(), GW_HWNDPREV);
	if (w != 0) {
		SetWindowPos((HWND)winId(), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}
}
