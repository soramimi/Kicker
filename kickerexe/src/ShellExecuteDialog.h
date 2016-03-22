#ifndef SHELLEXECUTEDIALOG_H
#define SHELLEXECUTEDIALOG_H

#include <QDialog>

namespace Ui {
class ShellExecuteDialog;
}

class ShellExecuteDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ShellExecuteDialog(QWidget *parent = 0);
	~ShellExecuteDialog();

	QString text() const;

	virtual void accept();
	virtual void reject();


private slots:
	void on_pushButton_browse_clicked();

private:
	Ui::ShellExecuteDialog *ui;
};

#endif // SHELLEXECUTEDIALOG_H
