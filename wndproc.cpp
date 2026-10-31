#include "includes.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND hWnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam
);

WndInput g_input;

LRESULT WINAPI Hooks::WndProc(HWND wnd, uint32_t msg, WPARAM wp, LPARAM lp)
{
	//if ( getforegroundwindow )

	if ( g_input.m_should_listen )
	{
		g_input.setup( msg, wp );
		g_GUI.m_is_open = g_input.get_key_toggle_state( VK_INSERT );

		switch ( msg )
		{
		case WM_LBUTTONDOWN:
			g_input.key_states[ VK_LBUTTON ].state = true;
			g_input.key_states[ VK_LBUTTON ].toggled_state = !g_input.key_states[ VK_LBUTTON ].toggled_state;
			break;

		case WM_LBUTTONUP:
			g_input.key_states[ VK_LBUTTON ].state = false;
			break;

		case WM_RBUTTONDOWN:
			g_input.key_states[ VK_RBUTTON ].state = true;
			g_input.key_states[ VK_LBUTTON ].toggled_state = !g_input.key_states[ VK_LBUTTON ].toggled_state;
			break;

		case WM_RBUTTONUP:
			g_input.key_states[ VK_RBUTTON ].state = false;
			break;

		case WM_MBUTTONDOWN:
			g_input.key_states[ VK_MBUTTON ].state = true;
			g_input.key_states[ VK_MBUTTON ].toggled_state = !g_input.key_states[ VK_MBUTTON ].toggled_state;
			break;

		case WM_MBUTTONUP:
			g_input.key_states[ VK_MBUTTON ].state = false;
			break;

		case WM_XBUTTONDOWN:
			if ( GET_XBUTTON_WPARAM( wp ) == XBUTTON1 )
			{
				g_input.key_states[ VK_XBUTTON1 ].state = true;
				g_input.key_states[ VK_XBUTTON1 ].toggled_state = !g_input.key_states[ VK_XBUTTON1 ].toggled_state;
			}

			if ( GET_XBUTTON_WPARAM( wp ) == XBUTTON2 )
			{
				g_input.key_states[ VK_XBUTTON2 ].state = true;
				g_input.key_states[ VK_XBUTTON2 ].toggled_state = !g_input.key_states[ VK_XBUTTON2 ].toggled_state;
			}

			break;

		case WM_XBUTTONUP:
			if ( GET_XBUTTON_WPARAM( wp ) == XBUTTON1 )
				g_input.key_states[ VK_XBUTTON1 ].state = false;

			if ( GET_XBUTTON_WPARAM( wp ) == XBUTTON2 )
				g_input.key_states[ VK_XBUTTON2 ].state = false;

			break;
		default:
			break;
		}
	}

	if (g_GUI.m_is_open && ImGui_ImplWin32_WndProcHandler(wnd, msg, wp, lp))
		return true;

	return g_winapi.CallWindowProcA(g_hooks.m_old_wndproc, wnd, msg, wp, lp);
}
