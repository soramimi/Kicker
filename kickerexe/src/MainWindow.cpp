#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "MySettings.h"
#include "pathcat.h"
#include "Options.h"

#include <vector>
#include <map>

#include <QTableWidgetItem>
#include <QCloseEvent>
#include "ShellExecuteDialog.h"



struct ShortcutItem {
	char key;
	QString name;
	QString command;
	ShortcutItem()
		: key(0)
	{
	}
};

struct MainWindow::Private {
	std::map<int, ShortcutItem> shortcut_items;
	MyApplication *app;
};

MainWindow::MainWindow(MyApplication *a)
	: QMainWindow(0)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	priv = new Private();
	priv->app = a;
	connect(this, SIGNAL(updateStatusText()), this, SLOT(doUpdateStatusText()));

	tray_icon_menu = new QMenu();
	tray_icon_menu->addAction(ui->action_open);
	tray_icon_menu->addAction(ui->action_exit);
	tray_icon = new QSystemTrayIcon(this);
	tray_icon->setIcon(QIcon(":/image/kicker.png"));
	tray_icon->setContextMenu(tray_icon_menu);
	tray_icon->show();

	the_decoywindow = new DecoyWindow();
	the_decoywindow->show();

	shell_execute_dialog = new ShellExecuteDialog();

	ui->label_status->clear();
	ui->radioButton_hide->click();

//	priv->shortcut_items.resize(26);

	if (the_options->getUndefinedShortcutBehavior() == Options::DelegateToTheDefaultShell) {
		ui->radioButton_delegate_to_the_default_shell->click();
	} else {
		ui->radioButton_do_nothing->click();
	}


	loadShortcutDefines();
}

MainWindow::~MainWindow()
{
	delete priv;
	delete ui;
}

void MainWindow::updateShortcutList()
{
	ui->tableWidget->clear();
	ShortcutItem items[26];
	for (size_t i = 0; i < priv->shortcut_items.size(); i++) {
		ShortcutItem const &sc = priv->shortcut_items[i];
		if (sc.key >= 'A' && sc.key <= 'Z') {
			int row = sc.key - 'A';
			items[row] = sc;
		}
	}
	int rows = 26;
	ui->tableWidget->setRowCount(rows);
	ui->tableWidget->setColumnCount(1);
	for (int row = 0; row < rows; row++) {
		ShortcutItem const &sc = items[row];
		QTableWidgetItem *item;
		item = new QTableWidgetItem();
		item->setText(QChar('A' + row));
		ui->tableWidget->setVerticalHeaderItem(row, item);
		item = new QTableWidgetItem();
		item->setText(sc.name);
		ui->tableWidget->setItem(row, 0, item);
		ui->tableWidget->setRowHeight(row, 20);
	}
	ui->tableWidget->resizeColumnsToContents();
}

void MainWindow::closeEvent(QCloseEvent *e)
{
	if (quit_accept) {
		the_decoywindow->close();
		delete the_decoywindow;
		shell_execute_dialog->close();
		delete shell_execute_dialog;
		e->accept();
	} else {
		e->ignore();
		hide();
	}
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
	if (e->key() == Qt::Key_Escape) {
		close();
		e->accept();
	}
}

void MainWindow::setQuitMode(bool f)
{
	quit_accept = f;
}

void MainWindow::on_pushButton_close_clicked()
{
	if (quit_accept) {
		quit_accept = true;
		close();
	} else {
		hide();
	}
}

void MainWindow::on_action_exit_triggered()
{
	quit_accept = true;
	close();
}

void MainWindow::on_action_open_triggered()
{
	show();
}

void MainWindow::on_radioButton_hide_clicked()
{
	setQuitMode(false);
}

void MainWindow::on_radioButton_quit_clicked()
{
	setQuitMode(true);
}

#include "Options.h"

bool MainWindow::findShortcutCommand(int vk, QString *command)
{
	if (vk >= 'A' && vk <= 'Z') {
		std::map<int, ShortcutItem>::const_iterator it = priv->shortcut_items.find(vk);
		if (it != priv->shortcut_items.end() && it->second.key == vk) {
			*command = it->second.command;
			return true;
		}
		if (the_options->getUndefinedShortcutBehavior() == Options::DelegateToTheDefaultShell) {
			return false; // delegate to the default
		}
		return true; // handled
	}
	return false;
}

void MainWindow::setStatusText(QString const &text)
{
	status_text = text;
	emit updateStatusText();
}

void MainWindow::setStatusText(char const *ptr, int len)
{
	if (ptr && len > 0) {
		setStatusText(QString::fromUtf8(ptr, len));
	} else {
		setStatusText(QString());
	}
}

void MainWindow::doUpdateStatusText()
{
	ui->label_status->setText(status_text);
}

#include <QFileDialog>

QString commandName(QString const &path)
{
	int i = path.lastIndexOf('/');
	int j = path.lastIndexOf('\\');
	if (i < j) {
		i = j;
	}
	if (i < 0) {
		i = 0;
	} else {
		i++;
	}
	j = path.lastIndexOf('.');
	if (j < i) j = path.size();
	return path.mid(i, j - i);
}

void MainWindow::on_pushButton_browse_clicked()
{
	int row = ui->tableWidget->currentRow();
	if (row >= 0 && row < 26) {
		// ok
	} else {\
		return;
	}
	QString path = QFileDialog::getOpenFileName(this, qApp->applicationName(), QString(), tr("Executable files (*.exe)") + ";;" + tr("All files (*.*)"));
	if (path.isEmpty()) {
		return;
	}

	ShortcutItem item;
	item.key = 'A' + row;
	item.name = commandName(path);
	item.command = path;
	priv->shortcut_items['A' + row] = item;

	saveShortcutDefines();
	updateShortcutList();
}

#include <QMessageBox>

void MainWindow::on_pushButton_clear_clicked()
{
	int row = ui->tableWidget->currentRow();
	if (row >= 0 && row < 26) {
		// ok
	} else {\
		return;
	}

	char c = 'A' + row;

	std::map<int, ShortcutItem>::iterator it = 	priv->shortcut_items.find(c);
	if (it == priv->shortcut_items.end()) {
		return;
	}

	char tmp[100];
	sprintf(tmp, "Are you sure to clear 'Ctrl+Alt+%c' ?", c);
	if (QMessageBox::warning(this, qApp->applicationName(), tmp, QMessageBox::Ok | QMessageBox::Cancel) != QMessageBox::Ok) {
		return;
	}

	priv->shortcut_items.erase(it);

	saveShortcutDefines();
	updateShortcutList();
}

#include <QXmlStreamWriter>

bool saveShortcutDefines(QString const &path, std::vector<ShortcutItem> const *items)
{
	QFile file(path);
	if (!file.open(QFile::WriteOnly)) {
		return false;
	}
	QXmlStreamWriter writer(&file);
	writer.setAutoFormatting(true);
	writer.writeStartDocument();
	writer.writeStartElement("items");
	for (std::vector<ShortcutItem>::const_iterator it = items->begin(); it != items->end(); it++) {
		ShortcutItem const &item = *it;
		if (item.key != 0) {
			writer.writeStartElement("item");
			writer.writeAttribute("key", item.key == 0 ? QString("") : QString(QChar(item.key)));
			writer.writeStartElement("name");
			writer.writeCharacters(item.name);
			writer.writeEndElement(); // name
			writer.writeStartElement("command");
			writer.writeCharacters(item.command);
			writer.writeEndElement(); // command
			writer.writeEndElement(); // item
		}
	}
	writer.writeEndElement(); // items
	file.close();
	return true;
}


enum State {
	STATE_UNKNOWN,
	STATE_ITEMS,
	STATE_ITEMS_ITEM,
	STATE_ITEMS_ITEM_NAME,
	STATE_ITEMS_ITEM_COMMAND,
};

bool loadShortcutDefines(QString const &path, std::vector<ShortcutItem> *out)
{
	out->clear();
	QFile file(path);
	if (!file.open(QFile::ReadOnly)) {
		return false;
	}

	ShortcutItem item;

	std::vector<State> state_stack;
	QXmlStreamReader reader(&file);
	reader.setNamespaceProcessing(false);
	while (!reader.atEnd()) {
		State state = STATE_UNKNOWN;
		State laststate = STATE_UNKNOWN;
		if (!state_stack.empty()) {
			laststate = state_stack.back();
		}
		switch (reader.readNext()) {
		case QXmlStreamReader::StartElement:
			if (state_stack.empty()) {
				if (reader.name() == "items") {
					state = STATE_ITEMS;
				}
			} else if (laststate == STATE_ITEMS) {
				QStringRef data;
				QXmlStreamAttributes atts = reader.attributes();
				if (reader.name() == "item") {
					item = ShortcutItem();
					QString key = atts.value("key").toString();
					if (key.size() == 1 && key[0].isLetter()) {
						item.key = key[0].toUpper().toLatin1();
					}
					state = STATE_ITEMS_ITEM;
				}
			} else if (laststate == STATE_ITEMS_ITEM) {
				if (reader.name() == "name") {
					state = STATE_ITEMS_ITEM_NAME;
				} else if (reader.name() == "command") {
					state = STATE_ITEMS_ITEM_COMMAND;
				}
			}
			state_stack.push_back(state);
			break;
		case QXmlStreamReader::Characters:
			if (laststate == STATE_ITEMS_ITEM_NAME) {
				item.name += reader.text();
			} else if (laststate == STATE_ITEMS_ITEM_COMMAND) {
				item.command += reader.text();
			}
			break;
		case QXmlStreamReader::EndElement:
			if (laststate == STATE_ITEMS_ITEM) {
				if (item.key != 0) {
					out->push_back(item);
				}
			}
			state_stack.pop_back();
			break;
		}
	}
	return true;
}

QString MainWindow::shortcutItemsFilePath() const
{
	return pathcat(applicationDataDir(), QString("shortcuts.xml"));
}

void MainWindow::saveShortcutDefines()
{
	QString path = shortcutItemsFilePath();
	std::vector<ShortcutItem> items;
	for (std::map<int, ShortcutItem>::const_iterator it = priv->shortcut_items.begin(); it != priv->shortcut_items.end(); it++) {
		items.push_back(it->second);
	}
	::saveShortcutDefines(path, &items);
}

void MainWindow::loadShortcutDefines()
{
	QString path = shortcutItemsFilePath();
	std::vector<ShortcutItem> items;
	::loadShortcutDefines(path, &items);
	priv->shortcut_items.clear();
	for (std::vector<ShortcutItem>::const_iterator it = items.begin(); it != items.end(); it++) {
		priv->shortcut_items[it->key] = *it;
	}
	updateShortcutList();
}




void MainWindow::on_pushButton_clicked()
{
}


void MainWindow::on_radioButton_do_nothing_clicked()
{
	the_options->setUndefinedShortcutBehavior(Options::DoNothing);
	the_options->save();
}

void MainWindow::on_radioButton_delegate_to_the_default_shell_clicked()
{
	the_options->setUndefinedShortcutBehavior(Options::DelegateToTheDefaultShell);
	the_options->save();
}


void MainWindow::on_checkBox_show_scan_code_toggled(bool checked)
{
	priv->app->lib.enableCaptureScanCode(checked);
	if (!checked) {
		setStatusText(0, 0);
	}
}
