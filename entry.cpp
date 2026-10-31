#include "includes.h"

int __stdcall DllMain( HMODULE self, ulong_t reason, void* reserved )
{
	if ( reason == DLL_PROCESS_ATTACH )
	{
		DisableThreadLibraryCalls( self );

		HANDLE thread = CreateThread( nullptr, 0, Client::init, nullptr, 0, nullptr );
		if ( thread )
			CloseHandle( thread );
	}

	if ( reason == DLL_PROCESS_DETACH )
	{
		FreeLibraryAndExitThread( self, 0 );
		ImGui_ImplDX9_Shutdown();
		ImGui_ImplWin32_Shutdown();

		//if ( oWndProc != nullptr )
		//{
		//	SetWindowLongPtrW( Game_Window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>( oWndProc ) );
		//	oWndProc = nullptr;
		//}
		//
		//SHUTDOWN
	}

	return true;
}