#pragma once
#include <unordered_map>
#include "imgui/imgui.h"
#include <string>
#include <functional>
#include "color.h"

/*
GUI (Jee-Yew-Eye)
* Noun
	Acronym of Grimace User Interface.
	An interface for issuing commands to a Logical Device for Grimace utilizing a pointing device that manipulates and activates Grimace. 
*/

enum CfgManageType_t : char {
	CFG_SAVE,
	CFG_LOAD,
	CFG_CREATE,
	CFG_DELETE
};

enum bindmode_t : char {
	ALWAYS = 0,
	HELD, 
	HELDOFF,
	TOGGLE
};

struct bind_t {
	int key, mode;
	bind_t() {
		key = 0;
		mode = bindmode_t::HELD;
	}

	bind_t(int k, int m) {
		key = k;
		mode = m;
	}
};

enum WeaponConfig_t : int {
	CONFIG_DEFAULT = 0,
	CONFIG_AUTO,
	CONFIG_SSG08,
	CONFIG_AWP,
	CONFIG_HEAVYPISTOL,
	CONFIG_PISTOL,
	CONFIG_MAX
};

struct FlagArrangement_t {
	// I Don't Fucking Care to do it RN
};

class GRUI {
public:
	bool m_is_open = true;
	int m_selected_aimweapon;
	int m_aimweapon_copy;
public: // custom imgui elements / modifications to elements
	ImVec2 size{ 650, 500 };
	const char* title = "DWSUGHDS";
	void Render();
	void RageTab(int iColumn = 0);
	void VisualsTab(int iColumn = 0);
	void MiscTab(int iColumn = 0);
	void SkinsTab(int iColumn = 0);
	void PlayersTab(int iColumn = 0);
public:
	bool Checkbox(const char* pTitle, bool* pItem, bool bCog = false, std::function<void()> fnLambda = nullptr);
	bool Colorpicker(const char* pTitle, Color* pItem);
	bool ChildWrap(const char* pName);
	bool Selectable(const char* label, bool selected, ImGuiSelectableFlags flags = 0, const ImVec2& size_arg = { 0, 0 });
	bool StandaloneCog(const char* name, std::function<void()> fnLambda = nullptr);
	bool MultiCombo(const char* szLabel, std::vector<std::pair<bool*, std::string>> items);
	bool Combo(const char* label, int* current_item, const char* items_separated_by_zeros, int popup_max_height_in_items = -1);
	bool Slider(const char* label, float* v, float v_min, float v_max, const char* format = "%.f", ImGuiSliderFlags flags = ImGuiSliderFlags_NoInput | ImGuiSliderFlags_AlwaysClamp);
	bool Slider(const char* label, int* v, int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = ImGuiSliderFlags_NoInput | ImGuiSliderFlags_AlwaysClamp);
public: // config system
	void ManageConfig(const char* pFileName, CfgManageType_t nType);
	void InitializeConfigSystem();
	void PopulateConfigs();
	std::string szDirectory;
	std::vector<std::string> vecConfigFiles;
	int iSelectedConfig = -1;
	int iSelectedSkinWeapon = 0;
	const char* szGlowTypes = "blur\0blur outline\0model\0edge\0pulse\0";
	const char* szFontsig = "verdana\0verdana bold\0tahoma bold\0small\0";
	bool bruh;
public:
	ImFont* cog;
};

class GVariables {
public:
	friend class GRUI;
	std::unordered_map<std::string, bool> m_bools;
	std::unordered_map<std::string, int> m_ints;
	std::unordered_map<std::string, float> m_floats;
	std::unordered_map<std::string, Color> m_colors;
	std::unordered_map<std::string, bind_t> m_binds;
	std::unordered_map<std::string, std::string> m_strings;
private:
	void InitRagebot();
	void InitVisuals();
	void InitMisc();
	void InitSkins();
public:
	void Init() {
		m_floats["menu_fade_speed"] = 6.66f;
		m_colors["menu_accent_color"] = Color(255, 255, 255); // Color(84, 170, 219)
		m_bools["menu_shade"] = false;
		m_bools["watermark"] = false;
		InitRagebot();
		InitVisuals();
		InitMisc();
		InitSkins();
	}

	bool get_keystate(bind_t skey);
	std::string GetAimbotWeaponPrefix(WeaponConfig_t);
};

extern GRUI g_GUI;

#define MAKECHILD(name) g_GUI.ChildWrap(name);
#define ENDCHILD ImGui::EndChild();

extern GVariables g_Variables;