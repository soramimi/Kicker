#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QMenu>
#include "DecoyWindow.h"
#include "win.h"

#include <QLabel>

#include "main.h"

namespace Ui {
class MainWindow;
}

class ShellExecuteDialog;

class MainWindow : public QMainWindow
{
	Q_OBJECT
private:
	struct Private;
	Private *priv;
	QSystemTrayIcon *tray_icon;
	QMenu *tray_icon_menu;
	DecoyWindow *the_decoywindow;
	ShellExecuteDialog *shell_execute_dialog;
	bool quit_accept;
	QString status_text;
	void setQuitMode(bool f);
	void updateShortcutList();
public:
	explicit MainWindow(MyApplication *a);
	~MainWindow();

	virtual void closeEvent(QCloseEvent *);
	virtual void keyPressEvent(QKeyEvent *);

	DecoyWindow *getDecoyWindow() const
	{
		return the_decoywindow;
	}

	ShellExecuteDialog *getShellExecuteDialog()
	{
		return shell_execute_dialog;
	}

	QString shortcutItemsFilePath() const;
	void saveShortcutDefines();
	void loadShortcutDefines();

	bool findShortcutCommand(int vk, QString *command);

	void setStatusText(QString const &text);
	void setStatusText(char const *ptr, int len);


signals:
	void updateStatusText();
private slots:

	void doUpdateStatusText();


	void on_pushButton_close_clicked();



	void on_action_exit_triggered();

	void on_action_open_triggered();

	void on_radioButton_hide_clicked();

	void on_radioButton_quit_clicked();

	void on_pushButton_browse_clicked();


	void on_pushButton_clicked();

	void on_pushButton_clear_clicked();

	void on_radioButton_do_nothing_clicked();

	void on_radioButton_delegate_to_the_default_shell_clicked();

	void on_checkBox_show_scan_code_toggled(bool checked);

private:
	Ui::MainWindow *ui;
};

extern MainWindow *the_mainwindow;

#endif // MAINWINDOW_H
