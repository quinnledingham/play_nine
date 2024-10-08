
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wingdi.h>
#include <stdint.h>

#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include "d3dx12.h" // Helper Structures and Functions

#include <string>
#include <wrl.h>
using Microsoft::WRL::ComPtr;
#include <shellapi.h>

#include "d3dx12.h"
#include "defines.h"
#include "types.h"
#include "dx12.h"
#include "win32.h"

void print(const char *msg, ...);
void logprint(const char *where, const char *msg, ...);

inline void
win32_print_str(const char *str) {
  OutputDebugStringA((LPCSTR)str);
}

inline void
win32_print_last_error() {
  wchar_t buf[256];
  FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                 NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
                 buf, (sizeof(buf) / sizeof(wchar_t)), NULL);
}

#include "dx12.cpp"
#include "golf.cpp"

// win32

bool8 global_running = true;

internal void
win32_process_pending_messages(HWND *hwnd) {
  MSG msg;
  while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
    switch(msg.message) {
      case WM_QUIT: {
        global_running = false;
      } break;

      default: {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
      } break;
    }
  }
}

LRESULT CALLBACK
win32_main_window_callback(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
  LRESULT result = 0;
  switch(message) {
    case WM_CLOSE: {
      global_running = false;
    } break;

    case WM_ACTIVATEAPP: {
      win32_print_str("WM_ACTIVATEAPP\n");
    } break;

    case WM_DESTROY: {

    } break;

    case WM_PAINT:
    case WM_ERASEBKGND:
    default: {
      result = DefWindowProc(window, message, w_param, l_param);
    }
  }

  return result;
}

int CALLBACK
WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR command_line, int show_code) {
  WNDCLASS window_class = {};
  window_class.hInstance = instance;
  window_class.lpfnWndProc = win32_main_window_callback;
  window_class.lpszClassName = "GolfWindowClass";

  if (!RegisterClassA(&window_class)) {
    win32_print_str("(win32) failed to register window class\n");
    win32_print_last_error();
    return 1;
  }
  
  HWND window_handle = CreateWindowExA(0, window_class.lpszClassName, "Golf", 
    WS_OVERLAPPEDWINDOW | WS_VISIBLE, // style
    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, // x, y, width, height
    0, 0, instance, 0);
  if (!window_handle) {
    win32_print_last_error();
    return 1;
  }
  
  RECT client_rect;
  GetClientRect(window_handle, &client_rect);
  gfx.window_dim.width = client_rect.right - client_rect.left;
  gfx.window_dim.height = client_rect.bottom - client_rect.top;
  gfx.aspect_ratio = (float32)gfx.window_dim.width / (float32)gfx.window_dim.height;

  dx_init(&gfx.dx12, window_handle, gfx.window_dim);

  App app = {};

  event_handler(&app, APP_INIT);
  
  while(global_running) {
    win32_process_pending_messages(&window_handle);


    event_handler(&app, APP_UPDATE);
  }

  dx_destroy(&gfx.dx12);
  
  return 0;
}
