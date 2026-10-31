#include "includes.h"

bool GRUI::Checkbox(const char* pTitle, bool* pItem, bool bCog, std::function<void()> fnLambda) {
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1.f, 1.f));
	bool result = ImGui::Checkbox(pTitle, pItem);

	if (bCog) {
		ImGui::SameLine();
		ImGui::PushFont(cog);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

		if (ImGui::Button(tfm::format("a##%s", pTitle).c_str())) {
			ImGui::OpenPopup(tfm::format("##aa%s", pTitle).c_str());
		}

		ImGui::PopStyleColor(3);
		ImGui::PopFont();

		if (ImGui::BeginPopup(tfm::format("##aa%s", pTitle).c_str())) {
			fnLambda();
			ImGui::EndPopup();
		}
	}

	ImGui::PopStyleVar(1);

	return result;
}

bool GRUI::StandaloneCog(const char* name, std::function<void()> fnLambda) {
	ImGui::SameLine();
	ImGui::PushFont(cog);

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

	 std::string btn = tfm::format("a##%s", name);
	 std::string popup = tfm::format("#aa%s", name);
	if (ImGui::Button(btn.c_str())) {
		ImGui::OpenPopup(popup.c_str());
	}

	ImGui::PopStyleColor(3);
	ImGui::PopFont();

	if (ImGui::BeginPopup(popup.c_str())) {
		fnLambda();
		ImGui::EndPopup();
	}

	return true;
}

bool GRUI::Colorpicker(const char* pTitle, Color* pItem) {

	float clr[4] = {
			(float)pItem->r() / 255,
			(float)pItem->g() / 255,
			(float)pItem->b() / 255,
			(float)pItem->a() / 255
	};

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1.f, 1.f));

	if (ImGui::ColorEdit4(pTitle, clr)) {
		*pItem = Color(float(clr[0] * 255.f), float(clr[1] * 255.f), float(clr[2] * 255.f), float(clr[3] * 255.f));
		ImGui::PopStyleVar();
		return true;
	}

	ImGui::PopStyleVar();

	return false;
}

bool GRUI::ChildWrap(const char* name) {
	ImGui::Text(name);
	return ImGui::BeginChild(tfm::format("%s##child", name).c_str(), ImVec2(248, 194), ImGuiChildFlags_Border, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
}

bool GRUI::Selectable(const char* label, bool selected, ImGuiSelectableFlags flags, const ImVec2& size_arg) {
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.f, 0.f });
	ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, { 0.5f, 0.5f });

	bool result = ImGui::Selectable(label, selected, flags, size_arg);

	ImGui::PopStyleVar(2);

	return result;
}

bool GRUI::MultiCombo(const char* szLabel, std::vector<std::pair<bool*, std::string>> items) {
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 1.f, 1.2f });
	ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, { 0.5f, 0.5f });

	std::string preview_string = "";

	for (int i = 0; i < items.size(); i++) {
		//preview_string += vecValues.size() > 0 ? items.at(i) + "," : items.at(i);
		if (*items.at(i).first) {
			preview_string += (items.size() > 1 && !(i == items.size() - 1)) ? items.at(i).second + ", " : items.at(i).second;
		}
	}

	if (ImGui::BeginCombo(szLabel, preview_string.c_str(), ImGuiComboFlags_NoArrowButton)) {
		for (int i = 0; i < items.size(); i++) {
			ImGui::Selectable(items.at(i).second.c_str(), items.at(i).first, ImGuiSelectableFlags_DontClosePopups);
		}
		ImGui::EndCombo();
	}

	ImGui::PopStyleVar(2);
	return true;
}

bool GRUI::Combo(const char* label, int* current_item, const char* items_separated_by_zeros, int popup_max_height_in_items) {
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 1.f, 1.2f });
	ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, { 0.5f, 0.5f });

	bool result = ImGui::Combo(label, current_item, items_separated_by_zeros, popup_max_height_in_items);

	ImGui::PopStyleVar(2);

	return result;
}

bool GRUI::Slider(const char* label, float* v, float v_min, float v_max, const char* format, ImGuiSliderFlags flags) {
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 1.f, 1.f });
	ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, { 0.5f, 0.5f });

	bool result = ImGui::SliderFloat(label, v, v_min, v_max, format, flags);

	ImGui::PopStyleVar(2);

	return result;
}

bool GRUI::Slider(const char* label, int* v, int v_min, int v_max, const char* format, ImGuiSliderFlags flags) {
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 1.f, 1.f });
	ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, { 0.5f, 0.5f });

	bool result = ImGui::SliderInt(label, v, v_min, v_max, format, flags);

	ImGui::PopStyleVar(2);

	return result;
}