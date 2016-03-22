#include "win.h"
#include <QString>
#include <QSize>
#include <QMessageBox>
#include <QApplication>
#include "MainWindow.h"
#include "DecoyWindow.h"
#include "../kickerdll/KickerAPI.h"
#include "ShellExecuteDialog.h"
#include <deque>

QString getModuleFileName()
{
	wchar_t tmp[300];
	DWORD n = GetModuleFileNameW(0, tmp, 300);
	return QString::fromUtf16((ushort const *)tmp, n);
}

QString getModuleFileDir()
{
	QString path = getModuleFileName();
	int i = path.lastIndexOf('/');
	int j = path.lastIndexOf('\\');
	if (i < j) i = j;
	if (i < 0) return QString();
	return QString::fromUtf16(path.utf16(), i);
}


QSize getPrimaryScreenSize()
{
	int w = GetSystemMetrics(SM_CXSCREEN);
	int h = GetSystemMetrics(SM_CYSCREEN);
	return QSize(w, h);
}

struct KickerLibrary::Private {
	HMODULE hKickerModule;
	KICKER_API_1 kicker_api;
	DecoyWindow *window;
	std::deque<Request> request_queue;
};

KickerLibrary::KickerLibrary()
{
	priv = new Private();
	priv->hKickerModule = 0;
}

KickerLibrary::~KickerLibrary()
{
	FreeLibrary(priv->hKickerModule);
	delete priv;
}

static void getproc_(FARPROC &fn, HMODULE h, char const *name)
{
	fn = GetProcAddress(h, name);
	if (!fn) throw QString(name);
}

#include <intrin.h>

HMODULE KickerLibrary::load_(char const *path, KICKER_API_1 *api)
{
	HMODULE h;
	h = LoadLibraryA(path);
	if (h) {
		fn_GetKickerAPI1_t getapi;
		getproc_((FARPROC &)getapi, h, "GetKickerAPI1");
		if (getapi(api, sizeof(KICKER_API_1))) {
			return h;
		}
		FreeLibrary(h);
	} else {
//		DWORD e = GetLastError();
//		__nop();
	}
	return 0;
}

bool KickerLibrary::load()
{
	char const *path = "kicker.dll";
	try {
		priv->hKickerModule = load_(path, &priv->kicker_api);
		if (!priv->hKickerModule) throw QString(path);
		return true;
	} catch (QString const &) {
	}
	priv->hKickerModule = 0;
	return false;
}

void KickerLibrary::setup(DecoyWindow *widget)
{
	if (!priv->hKickerModule) return;
	priv->window = widget;
	HWND hwnd = (HWND)widget->winId();
	priv->kicker_api.setup((HWND)hwnd);
}

void KickerLibrary::enable(bool f)
{
	if (!priv->hKickerModule) return;
	if (f) {
		priv->kicker_api.enable();
	} else {
		priv->kicker_api.disable();
	}
}


HWND GetInputTargetWindow()
{
	HWND hw = 0;
	DWORD ap = 0;
	DWORD at = GetWindowThreadProcessId(GetForegroundWindow(), &ap);
	if (AttachThreadInput(GetCurrentThreadId(), at, TRUE)) {
		hw = GetFocus();
		AttachThreadInput(GetCurrentThreadId(), at, FALSE);
	}
	if (!hw) {
		hw = GetForegroundWindow();
	}
	return hw;
}

static void InputVK(DWORD vk, bool press)
{
	INPUT in[1];
	memset(&in, 0, sizeof(in));
	in[0].type = INPUT_KEYBOARD;
	in[0].ki.wVk = (WORD)vk;
	in[0].ki.wScan = 0;
	in[0].ki.dwFlags = 0;
	in[0].ki.time = 0;
	if (!press) {
		in[0].ki.dwFlags |= KEYEVENTF_KEYUP;
	}
	SendInput(1, in, sizeof(INPUT));
}

#define OPEN_MAIN_WINDOW "<ShowMainWindow>"
#define SHELL_EXECUTE "<ShellExecute>"

bool KickerLibrary::GetRequest(int vk, Request *req)
{
	*req = Request();
	if (vk == VK_OEM_2 || vk == '0') { // /?
		req->command = OPEN_MAIN_WINDOW;
		return true;
	} else {
		return the_mainwindow->findShortcutCommand(vk, &req->command);
	}
	return true;
}

void forceActivate(QWidget *w)
{
	DWORD at = GetWindowThreadProcessId(GetForegroundWindow(), 0);
	if (AttachThreadInput(GetCurrentThreadId(), at, TRUE)) {
		w->show();
		w->activateWindow();
		AttachThreadInput(GetCurrentThreadId(), at, FALSE);
	}
}

void KickerLibrary::doShowMainWindow()
{
	forceActivate(the_mainwindow);
}

void KickerLibrary::doShellExecute()
{
	ShellExecuteDialog *d = the_mainwindow->getShellExecuteDialog();
	forceActivate(d);
}

void KickerLibrary::enableCaptureScanCode(bool f)
{
	priv->kicker_api.enable_capture_scan_code(f);
}

typedef BOOL (WINAPI *fn_IsWow64Process_t)(HANDLE hProcess, BOOL *Wow64Process);
typedef BOOL (WINAPI *fn_Wow64DisableWow64FsRedirection_t)(VOID **OldValue);
typedef BOOLEAN (WINAPI *fn_Wow64EnableWow64FsRedirection_t)(BOOLEAN Wow64FsEnableRedirection);
typedef BOOL (WINAPI *fn_Wow64RevertWow64FsRedirection_t)(VOID *OlValue);

void KickerLibrary::execute(QString const &cmd)
{
	ShellExecuteW(0, L"open", (wchar_t const *)cmd.utf16(), 0, 0, SW_SHOWNORMAL);
}

void KickerLibrary::Invoke(Request const &req)
{
	if (req.empty()) return;

	if (req.command[0] == '<') {
		if (req.command == OPEN_MAIN_WINDOW) {
			doShowMainWindow();
		} else if (req.command == SHELL_EXECUTE) {
			doShellExecute();
		}
		return;
	}

	HMODULE hm = GetModuleHandleA("kernel32.dll");
	if (hm) {
		fn_IsWow64Process_t fn_IsWow64Process;
		(FARPROC &)fn_IsWow64Process = GetProcAddress(hm, "IsWow64Process");
		if (fn_IsWow64Process) {
			BOOL is64 = FALSE;
			if (fn_IsWow64Process(GetCurrentProcess(), &is64) && is64)  {
				fn_Wow64DisableWow64FsRedirection_t fn_Wow64DisableWow64FsRedirection;
				fn_Wow64EnableWow64FsRedirection_t fn_Wow64EnableWow64FsRedirection;
				fn_Wow64RevertWow64FsRedirection_t fn_Wow64RevertWow64FsRedirection;
				(FARPROC &)fn_Wow64DisableWow64FsRedirection = GetProcAddress(hm, "Wow64DisableWow64FsRedirection");
				(FARPROC &)fn_Wow64EnableWow64FsRedirection = GetProcAddress(hm, "Wow64EnableWow64FsRedirection");
				(FARPROC &)fn_Wow64RevertWow64FsRedirection = GetProcAddress(hm, "Wow64RevertWow64FsRedirection");
				if (fn_Wow64DisableWow64FsRedirection && fn_Wow64EnableWow64FsRedirection && fn_Wow64RevertWow64FsRedirection) {
					void *oldval = 0;
					Wow64DisableWow64FsRedirection(&oldval);
					execute(req.command);
					Wow64RevertWow64FsRedirection(oldval);
					return;
				}
			}
		}
	}

	execute(req.command);
}

void KickerLibrary::onShellExecute(MSG *msg)
{
	Request req;
	req.command = SHELL_EXECUTE;
	priv->request_queue.push_back(req);
	PostMessage(msg->hwnd, WM_KICKER_PROCESS_QUEUE, 0, 0);
}

void postChar(WPARAM ch)
{
	HWND hw = GetInputTargetWindow();
	::PostMessage(hw, WM_CHAR, ch, 0);
}


WPARAM KickerLibrary::translateKey(WPARAM vk, bool shift) const
{
	WPARAM ch = 0;
	switch (vk) {
	case 'Q':
		ch = !shift ? '?' : '!';
		break;
	case 'W':
//			if (!shift) {
//				InputVK(VK_LWIN, true);
//				InputVK(VK_LWIN, false);
//			} else {
//				InputVK(VK_APPS, true);
//				InputVK(VK_APPS, false);
//			}
		break;
	case 'E':
		ch = !shift ? '=' : '#';
		break;
	case 'R':
		ch = !shift ? '$' : '`';
		break;
	case 'T':
		ch = !shift ? '~' : '^';
		break;
	case 'Y':
		InputVK(VK_HOME, true);
		InputVK(VK_HOME, false);
		break;
	case 'U':
		InputVK(VK_END, true);
		InputVK(VK_END, false);
		break;
	case 'I':
		InputVK(VK_UP, true);
		InputVK(VK_UP, false);
		break;
	case 'O':
//			ch = !shift ? ';' : ':';
		break;
	case 'P':
		ch = !shift ? '+' : '-';
		break;

	case 'A':
		ch = !shift ? '@' : '(';
		break;
	case 'S':
		ch = !shift ? '\'' : ')';
		break;
	case 'D':
		ch = !shift ? '\"' : '_';
		break;
	case 'F':
		InputVK(VK_KANJI, true);
		InputVK(VK_KANJI, false);
		break;
	case 'G':
		InputVK(VK_PRIOR, true);
		InputVK(VK_PRIOR, false);
		break;
	case 'H':
		InputVK(VK_NEXT, true);
		InputVK(VK_NEXT, false);
		break;
	case 'J':
		InputVK(VK_LEFT, true);
		InputVK(VK_LEFT, false);
		break;
	case 'K':
		InputVK(VK_DOWN, true);
		InputVK(VK_DOWN, false);
		break;
	case 'L':
		InputVK(VK_RIGHT, true);
		InputVK(VK_RIGHT, false);
		break;
	case VK_OEM_1:
		ch = !shift ? ';' : ':';
		break;

	case 'Z':
		ch = !shift ? '[' : '{';
		break;
	case 'X':
		ch = !shift ? ']' : '}';
		break;
	case 'C':
		InputVK(VK_NONCONVERT, true);
		InputVK(VK_NONCONVERT, false);
		break;
	case 'V':
		InputVK(VK_CONVERT, true);
		InputVK(VK_CONVERT, false);
		break;
	case 'B':
		ch = !shift ? '\\' : '|';
		break;
	case 'N':
		ch = !shift ? '/' : '%';
		break;
	case 'M':
		ch = !shift ? '*' : '&';
		break;
	case VK_OEM_COMMA:
		ch = !shift ? '?': '!';
		break;
	case VK_OEM_PERIOD:
		ch = !shift ? '/' : '\\';
		break;
	case VK_SPACE:
		break;
	case VK_RETURN:
		break;
	case VK_ESCAPE:
//			if (!shift) {
//				InputVK(VK_LWIN, true);
//				InputVK(VK_LWIN, false);
//			} else {
//				InputVK(VK_APPS, true);
//				InputVK(VK_APPS, false);
//			}
		break;
	}
	return ch;
}

bool KickerLibrary::nativeEventFilter(const QByteArray & /*eventType*/, void *message, long *result)
{
	MSG *msg = (MSG *)message;
	if (msg->message == WM_KICKER_INPUTKEY) {
		if (msg->wParam == VK_RETURN) {
			onShellExecute(msg);
		} else {
			bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
			WPARAM ch = translateKey(msg->wParam, shift);
			if (ch) {
				HWND hw = GetInputTargetWindow();
				::PostMessage(hw, WM_CHAR, ch, 0);
			}
		}
		return true;
	} else if (msg->message == WM_KICKER_INPUTCHAR) {
		WPARAM ch = msg->wParam;
		if (ch) {
			HWND hw = GetInputTargetWindow();
			if (!hw) return false;
			::PostMessage(hw, WM_CHAR, ch, 0);
		}
		return true;
	} else if (msg->message == WM_KICKER_LAYOUT) {
		priv->window->updateLayout();
		return true;
	} else if (msg->message == WM_KICKER_SHORTCUT) {
		Request req;
		if (GetRequest(msg->wParam, &req)) {
			if (!req.empty()) {
				priv->request_queue.push_back(req);
				PostMessage(msg->hwnd, WM_KICKER_PROCESS_QUEUE, 0, 0);
			}
			*result = SHORTCUT_HANDLED;
		} else {
			*result = SHORTCUT_SYSTEMDEFAULT;
		}
		return true;
	} else if (msg->message == WM_KICKER_PROCESS_QUEUE) {
		while (!priv->request_queue.empty()) {
			Request const &r = priv->request_queue.front();
			Invoke(r);
			priv->request_queue.pop_front();
		}
		return true;
	} else if (msg->message == WM_KICKER_SCANCODE) {
		char tmp[100];
		sprintf(tmp, "0x%02X %s", msg->lParam, msg->wParam ? "pressed" : "released");
		the_mainwindow->setStatusText(tmp, strlen(tmp));
	} else if (msg->message == WM_KICKER_STATUSTEXT) {
//		priv->status_text = QString::fromUtf8((char const *)msg->lParam, msg->wParam);
		the_mainwindow->setStatusText((char const *)msg->lParam, msg->wParam);
		return true;
	}
	return false;
}
