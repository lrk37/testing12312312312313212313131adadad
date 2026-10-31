#include "includes.h"

HRESULT __stdcall Hooks::Reset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentParameters)
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	auto result = g_hooks.m_device.GetOldMethod<decltype(&Reset)>(16)(pDevice, pPresentParameters);

	if (result >= 0)
		ImGui_ImplDX9_CreateDeviceObjects();

	return result;
}