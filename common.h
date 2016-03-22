
#ifndef KICKER_COMMON_H__
#define KICKER_COMMON_H__

#include <Windows.h>

#ifndef DLLEXPORT
#define DLLEXPORT __declspec(dllexport)
#endif

#define WM_KICKER_LAYOUT (WM_APP + 100)
#define WM_KICKER_DIALOG (WM_APP + 101)
#define WM_KICKER_SHORTCUT (WM_APP + 102)
#define WM_KICKER_PROCESS_QUEUE (WM_APP + 103)
#define WM_KICKER_INPUTKEY (WM_APP + 104)
#define WM_KICKER_INPUTCHAR (WM_APP + 105)
#define WM_KICKER_SCANCODE (WM_APP + 106)
#define WM_KICKER_STATUSTEXT (WM_APP + 199)

#define VK_CAPSLOCK_ 0xf0

#define SHORTCUT_SYSTEMDEFAULT 0
#define SHORTCUT_HANDLED 1


typedef void (WINAPI *fn_kicker_setup_t)(HWND hwnd);
typedef void (WINAPI *fn_kicker_enable_t)();
typedef void (WINAPI *fn_kicker_disable_t)();
typedef void (WINAPI *fn_kicker_show_t)();
typedef void (WINAPI *fn_kicker_hide_t)();
typedef void (WINAPI *fn_kicker_enable_capture_scan_code)(bool f);


struct KICKER_API_1 {
	fn_kicker_setup_t setup;
	fn_kicker_enable_t enable;
	fn_kicker_disable_t disable;
	fn_kicker_show_t show;
	fn_kicker_hide_t hide;
	fn_kicker_enable_capture_scan_code enable_capture_scan_code;

};

typedef bool (WINAPI *fn_GetKickerAPI1_t)(KICKER_API_1 *api, int size);

extern "C" DLLEXPORT bool WINAPI GetKickerAPI1(KICKER_API_1 *api, int size);

#endif

