#ifndef WIN_H
#define WIN_H

#include <windows.h>
#include <QSize>
#include <QWidget>
#include <QAbstractNativeEventFilter>
#include "DecoyWindow.h"

QSize getPrimaryScreenSize();

struct KICKER_API_1;

class KickerLibrary : public QAbstractNativeEventFilter {
private:
	struct Private;
	Private *priv;

	struct Request {
		QString command;
		bool empty() const
		{
			return command.isEmpty();
		}
	};

	WPARAM translateKey(WPARAM ch, bool shift) const;

	void onShellExecute(MSG *msg);
	virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);
	static HMODULE load_(char const *path, KICKER_API_1 *api);
	bool GetRequest(int vk, Request *req);
	void Invoke(Request const &req);
	void doShowMainWindow();
	void doShellExecute();
public:
	KickerLibrary();
	~KickerLibrary();
	bool load();
	void setup(DecoyWindow *widget);
	void enable(bool f);
	void enableCaptureScanCode(bool f);
	static void execute(const QString &cmd);
};

#endif // WIN_H
