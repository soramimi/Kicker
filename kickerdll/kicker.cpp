
#pragma warning(disable:4996)

#define DLLEXPORT __declspec(dllexport)
#include "KickerAPI.h"

#include <stdio.h>
#include <windows.h>
#include <set>
#include "resource.h"

#include "common.h"

HINSTANCE _hInstance = 0;

#pragma data_seg("jp_soramimi_kicker_shared_data")

enum {
	BIT_F1  = 0x00000001,
	BIT_F2  = 0x00000002,
	BIT_F3  = 0x00000004,
	BIT_F4  = 0x00000008,
	BIT_F5  = 0x00000010,
	BIT_F6  = 0x00000020,
	BIT_F7  = 0x00000040,
	BIT_F8  = 0x00000080,
	BIT_F9  = 0x00000100,
	BIT_F10 = 0x00000200,
	BIT_F11 = 0x00000400,
	BIT_F12 = 0x00000800,
};

HWND s_transparent_hwnd = 0;
HHOOK s_hMyKeyboardHookLL = 0;
HHOOK s_hMyMouseHook = 0;
bool s_hidden = false;
int s_mouse_point_x = 0;
int s_mouse_point_y = 0;
int s_hidden_point_x = 0;
int s_hidden_point_y = 0;
bool s_special = false;
unsigned short s_fkeys = 0;
bool s_capslock = false;
bool s_capture_scan_code = false;

#pragma data_seg()

#include <sys/stat.h>

void debug_print(char const *ptr)
{
	HDC hdc = GetWindowDC(0);
	TextOutA(hdc, 0, 0, ptr, (int)strlen(ptr));
	ReleaseDC(0, hdc);
}

void debug_counter()
{
	static unsigned int _ = 0;
	char tmp[100];
	sprintf(tmp, "[%u] ", ++_);
	debug_print(tmp);
}

void show_cursor()
{
	if (s_hidden) {
		SetCursorPos(s_mouse_point_x, s_mouse_point_y);
		s_hidden = false;
	}
}

void hide_cursor()
{
	if (!s_hidden && !GetCapture()) {
		SendMessage(s_transparent_hwnd, WM_KICKER_LAYOUT, 0, 0);

		POINT pt;
		GetCursorPos(&pt);
		s_mouse_point_x = pt.x;
		s_mouse_point_y = pt.y;

		RECT rc;
		GetWindowRect(s_transparent_hwnd, &rc);
		s_hidden_point_x = rc.left;
		s_hidden_point_y = rc.top;

		SetCursorPos(s_hidden_point_x, s_hidden_point_y);
		s_hidden = true;
	}
}

static void InputVK(DWORD vk, bool press)
{
#if 1
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
#else
	DWORD flags = 0;
	if (!press) flags |= KEYEVENTF_KEYUP;
	keybd_event((BYTE)vk, 0, flags, 0);
#endif
}

static unsigned short GetTargetThreadLanguageId()
{
	DWORD tid = GetWindowThreadProcessId(GetForegroundWindow(), 0);
	HKL hkl = GetKeyboardLayout(tid);
	return (unsigned short)hkl;
}

static bool isJP()
{
	return PRIMARYLANGID(GetTargetThreadLanguageId()) == LANG_JAPANESE;
}

static bool isEN()
{
	return PRIMARYLANGID(GetTargetThreadLanguageId()) == LANG_ENGLISH;
}

static inline bool isPressed(int vk)
{
	return (GetAsyncKeyState(vk) & 0x8000) != 0;
}

bool isCapsLock(int vk)
{
	return vk == 0xf0;
}

bool isVirtualF11(int vk)
{
	return vk == VK_OEM_MINUS;
}

bool isVirtualF12(int vk)
{
	if (vk == VK_OEM_7) {
		if (isJP()) return true;
	} else if (vk == VK_OEM_PLUS) {
		if (isEN()) return true;
	}
	return false;
}

bool isVirtualColon(int vk)
{
	if (vk == VK_OEM_PLUS) {
		if (isJP()) return true;
	} else if (vk == VK_OEM_1) {
		if (isEN()) return true;
	}
	return false;
}

HWND GetInputTargetWindow()
{
	DWORD ap = 0;
	DWORD at = GetWindowThreadProcessId(GetForegroundWindow(), &ap);
	if (AttachThreadInput(GetCurrentThreadId(), at, TRUE)) {
		HWND hw = GetFocus();
		AttachThreadInput(GetCurrentThreadId(), at, FALSE);
		return hw;
	} else {
		return 0;
	}
}


LRESULT CALLBACK MyKeyboardHookLL(int code, WPARAM wParam, LPARAM lParam)
{
	if (code == HC_ACTION && lParam) {
		KBDLLHOOKSTRUCT *data = (KBDLLHOOKSTRUCT *)lParam;
		bool pressed = (wParam == WM_KEYDOWN) || (wParam == WM_SYSKEYDOWN);
		bool released = (wParam == WM_KEYUP) || (wParam == WM_SYSKEYUP);
		DWORD vk = data->vkCode;

		bool shift_pressed = isPressed(VK_SHIFT);
		bool ctrl_pressed = isPressed(VK_CONTROL);
		bool alt_pressed = isPressed(VK_MENU);

		if (s_capture_scan_code) {
			SendMessage(s_transparent_hwnd, WM_KICKER_SCANCODE, pressed ? 1 : 0, vk);
		}

		if (0) { // debug
			int i = GetKeyboardType(0);
			char tmp[100];
			sprintf(tmp, "VK=0x%02X %s", vk & 0xff, pressed ? "Pressed" : "Released");
			SendMessage(s_transparent_hwnd, WM_KICKER_STATUSTEXT, strlen(tmp), (LPARAM)tmp);
			//debug_print(tmp);
		}

		if (vk == VK_OEM_2 && pressed && ctrl_pressed && alt_pressed) {
			SendMessage(s_transparent_hwnd, WM_KICKER_SHORTCUT, vk, 0);
			return 1;
		}

		if (isCapsLock(vk) && pressed && !shift_pressed && !ctrl_pressed && !alt_pressed) {
			s_special = true;
			return 1;
		}

		if (vk == VK_LEFT || vk == VK_RIGHT || vk == VK_UP || vk == VK_DOWN) { // hide cursor
			hide_cursor();
		} else if ((vk >= 'A' && vk <= 'Z') || vk == VK_OEM_COMMA || vk == VK_OEM_PERIOD || vk == VK_SPACE || vk == VK_RETURN || vk == VK_TAB || vk == VK_ESCAPE || vk == VK_BACK || isVirtualColon(vk)) {
			if (pressed) {
				if (s_special) {
					s_special = false;
					if (isVirtualColon(vk)) {
						vk = VK_OEM_1;
					}
					PostMessage(s_transparent_hwnd, WM_KICKER_INPUTKEY, vk, 0);
					return 1;
				}
			}
		} else if ((vk >= '0' && vk <= '9') || isVirtualF11(vk) || isVirtualF12(vk)) { // virtual function keys
			if (pressed) {
				if (s_special) {
					s_special = false;
					DWORD fk = 0;
					switch (vk) {
					case '1':  fk = VK_F1;  s_fkeys |= BIT_F1;  break;
					case '2':  fk = VK_F2;  s_fkeys |= BIT_F2;  break;
					case '3':  fk = VK_F3;  s_fkeys |= BIT_F3;  break;
					case '4':  fk = VK_F4;  s_fkeys |= BIT_F4;  break;
					case '5':  fk = VK_F5;  s_fkeys |= BIT_F5;  break;
					case '6':  fk = VK_F6;  s_fkeys |= BIT_F6;  break;
					case '7':  fk = VK_F7;  s_fkeys |= BIT_F7;  break;
					case '8':  fk = VK_F8;  s_fkeys |= BIT_F8;  break;
					case '9':  fk = VK_F9;  s_fkeys |= BIT_F9;  break;
					case '0':  fk = VK_F10; s_fkeys |= BIT_F10; break;
					default:
						if (isVirtualF11(fk)) { fk = VK_F11; s_fkeys |= BIT_F11; break; }
						if (isVirtualF12(fk)) { fk = VK_F12; s_fkeys |= BIT_F12; break; }
						break;
					}
					if (fk != 0) {
						InputVK(fk, pressed);
						return 1;
					}
				}
			}
		} else {
			if (released) {
				switch (vk) {
				case VK_SHIFT:
				case VK_LSHIFT:
				case VK_RSHIFT:
				case VK_MENU:
				case VK_LMENU:
				case VK_RMENU:
				case VK_CONTROL:
				case VK_LCONTROL:
				case VK_RCONTROL:
				case '0':
					break;
				default:
					s_special = false;
					break;
				}
			}
		}
		if (pressed) {
			if (ctrl_pressed && alt_pressed) { // Ctrl+Alt shutcut
				LRESULT r = SendMessage(s_transparent_hwnd, WM_KICKER_SHORTCUT, vk, 0);
				if (r == SHORTCUT_HANDLED) {
					return 1;
				}
			}
		} else {
			if (s_fkeys & BIT_F1)  { InputVK(VK_F1, false); }
			if (s_fkeys & BIT_F2)  { InputVK(VK_F2, false); }
			if (s_fkeys & BIT_F3)  { InputVK(VK_F3, false); }
			if (s_fkeys & BIT_F4)  { InputVK(VK_F4, false); }
			if (s_fkeys & BIT_F5)  { InputVK(VK_F5, false); }
			if (s_fkeys & BIT_F6)  { InputVK(VK_F6, false); }
			if (s_fkeys & BIT_F7)  { InputVK(VK_F7, false); }
			if (s_fkeys & BIT_F8)  { InputVK(VK_F8, false); }
			if (s_fkeys & BIT_F9)  { InputVK(VK_F9, false); }
			if (s_fkeys & BIT_F10) { InputVK(VK_F10, false); }
			if (s_fkeys & BIT_F11) { InputVK(VK_F11, false); }
			if (s_fkeys & BIT_F12) { InputVK(VK_F12, false); }
			s_fkeys = 0;
		}
	}
	return CallNextHookEx(s_hMyKeyboardHookLL, code, wParam, lParam);
}

LRESULT CALLBACK MyMouseHook(int code, WPARAM wParam, LPARAM lParam)
{
	MOUSEHOOKSTRUCT *m = (MOUSEHOOKSTRUCT *)lParam;
	if (code >= 0) {
		if (s_hidden_point_x != m->pt.x || s_hidden_point_y != m->pt.y) {
			show_cursor();
		}
	}

	return CallNextHookEx(s_hMyMouseHook, code, wParam, lParam);
}

void WINAPI kicker_setup(HWND hwnd)
{
	s_transparent_hwnd = hwnd;
}

void WINAPI kicker_enable()
{
	if (!s_hMyMouseHook)      s_hMyMouseHook = SetWindowsHookEx(WH_MOUSE, MyMouseHook, _hInstance, 0);
	if (!s_hMyKeyboardHookLL) s_hMyKeyboardHookLL = SetWindowsHookEx(WH_KEYBOARD_LL, MyKeyboardHookLL, _hInstance, 0);
}

void WINAPI kicker_disable()
{
	if (s_hMyMouseHook)    UnhookWindowsHookEx(s_hMyMouseHook);
	if (s_hMyKeyboardHookLL) UnhookWindowsHookEx(s_hMyKeyboardHookLL);
}

void WINAPI kicker_show()
{
	show_cursor();
}

void WINAPI kicker_hide()
{
	hide_cursor();
}

void WINAPI kicker_enable_capture_scan_code(bool f)
{
	s_capture_scan_code = f;
}

extern "C" DLLEXPORT bool WINAPI GetKickerAPI1(KICKER_API_1 *api, int size)
{
	if (!api || size != sizeof(KICKER_API_1)) {
		return false;
	}
	api->setup = kicker_setup;
	api->enable = kicker_enable;
	api->disable = kicker_disable;
	api->show = kicker_show;
	api->hide = kicker_hide;
	api->enable_capture_scan_code = kicker_enable_capture_scan_code;
	return true;
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	_hInstance = (HINSTANCE)hModule;

	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

