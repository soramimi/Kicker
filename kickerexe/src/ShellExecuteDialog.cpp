#include <windows.h>
#include <shlobj.h>
#include "ShellExecuteDialog.h"
#include "ui_ShellExecuteDialog.h"
#include "win.h"
#include <QDesktopWidget>

ShellExecuteDialog::ShellExecuteDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::ShellExecuteDialog)
{
	ui->setupUi(this);
	Qt::WindowFlags flags = windowFlags();
	flags &= ~Qt::WindowContextHelpButtonHint;
	setWindowFlags(flags);

	QRect screen = QApplication::desktop()->screenGeometry();
	move(screen.center() - rect().center());

	bool isadmin = IsUserAnAdmin();
	ui->checkBox_run_as_admin->setChecked(isadmin);

	ui->lineEdit->setFocus();
	ui->lineEdit->selectAll();
}

ShellExecuteDialog::~ShellExecuteDialog()
{
	delete ui;
}

QString ShellExecuteDialog::text() const
{
	return ui->lineEdit->text();
}

void ShellExecuteDialog::accept()
{
	QString command = ui->lineEdit->text();
	hide();
	wchar_t const *verb = L"open";
	if (ui->checkBox_run_as_admin->isChecked()) {
		verb = L"runas";
	}
	KickerLibrary::execute(command);
//	ShellExecute(0, verb, (wchar_t const *)command.utf16(), 0, 0, SW_SHOW);
}

void ShellExecuteDialog::reject()
{
	hide();
}

#include <QFileDialog>

void ShellExecuteDialog::on_pushButton_browse_clicked()
{
	QString path;
	path = QFileDialog::getOpenFileName(this, tr("Browse"), path, tr("Programs") + " (*.exe;*.pif;*.com;*.bat;*.cmd);;" + tr("All files") + " (*.*)");
	if (!path.isEmpty()) {
		ui->lineEdit->setText(path);
	}
}
