#include "includes.h"

void GRUI::InitializeConfigSystem() {
	g_Variables.Init();
	
	szDirectory = "C:/Grimace/Configs";
	
	if (!std::filesystem::exists(szDirectory))
		std::filesystem::create_directories(szDirectory);
}

void GRUI::PopulateConfigs() {
	vecConfigFiles.clear();

	for (const auto& it : std::filesystem::directory_iterator(szDirectory)) {
		std::filesystem::path file_path(it.path().filename().string());

		if (file_path.has_extension())
			continue; // only our cfg files shouldnt have an extension

		vecConfigFiles.push_back(it.path().filename().string());
	}
}

void CreateConfig(const std::string path) {
	std::ofstream ofs(path);
}

void SaveConfig(const std::string path) {
	nlohmann::json cfg = {};

	try {
		for (auto& boo : g_Variables.m_bools) {
			cfg["bools"][boo.first] = boo.second;
		}

		for (auto& boo : g_Variables.m_colors) {
			nlohmann::json sub = { };

			// fill node with all color values
			sub.push_back(boo.second.r());
			sub.push_back(boo.second.g());
			sub.push_back(boo.second.b());
			sub.push_back(boo.second.a());


			cfg["colors"][boo.first] = sub.dump();
		}

		for (auto& boo : g_Variables.m_ints) {
			cfg["ints"][boo.first] = boo.second;
		}

		for (auto& boo : g_Variables.m_floats) {
			cfg["floats"][boo.first] = boo.second;
		}

		for (auto& boo : g_Variables.m_binds) {
			nlohmann::json sub = { };
			sub.push_back(boo.second.key);
			sub.push_back(boo.second.mode);


			cfg["binds"][boo.first] = sub.dump();
		}

		for (auto& boo : g_Variables.m_strings) {
			cfg["strings"][boo.first] = boo.second;
		}

		for (int i = 0; i < g_skins.m_weapon_name_map.size(); i++) {
			auto info = g_skins.m_weapon_name_map[i];
			if (info.set_paintkit > 0)
				cfg["skins"][tfm::format("%s", info.definition_index)] = info.set_paintkit;
		}

		for (int i = 0; i < g_skins.saved_kits.size(); i++) {
			auto info = g_skins.saved_kits[i];
			if (info.custom_col[0].rgba() != info.saved_col[0].rgba() 
				|| info.custom_col[1].rgba() != info.saved_col[1].rgba() 
				|| info.custom_col[2].rgba() != info.saved_col[2].rgba()
				|| info.custom_col[3].rgba() != info.saved_col[3].rgba()) {
				nlohmann::json sub = { };

				// fill node with all color values
				sub.push_back(info.custom_col[0].rgba());
				sub.push_back(info.custom_col[1].rgba());
				sub.push_back(info.custom_col[2].rgba());
				sub.push_back(info.custom_col[3].rgba());

				cfg["paintkits"][tfm::format("%i", i)] = sub.dump();
			}
		}

	}

	catch (const nlohmann::detail::exception&)
	{
		throw std::runtime_error("Config save fail Error.");
		return;
	}

	std::ofstream out(path);

	if (!out.good())
		return;

	try {
		out << cfg.dump(4);
		out.close();
	}

	catch (std::ofstream::failure&)
	{
		throw std::runtime_error("Config save Error.");
		return;
	}

	//cLogger.AddToLogTrack(cGlobals.FormatString("Saved %s", file_name));
	//cLogger.AddToLogTrack(std::format("Saved config {}", file_name));
}

void LoadConfig(const std::string path) {
	std::ifstream in(path);
	nlohmann::json cfg = {};
	try {
		cfg = nlohmann::json::parse(in, nullptr, false);

		if (cfg.is_discarded() || cfg.is_null()) {
			throw std::runtime_error("CFG_Discarded Error.");
			return;
		}

		in.close();
	}

	catch (std::ifstream::failure&) {
		throw std::runtime_error("CFG_Ifstream Error.");
		return;
	}

	try {
		for (auto& boo : g_Variables.m_bools) {
			if (cfg["bools"][boo.first].is_null()) {
				printf_s("missing bools variable: %s\n", boo.first.c_str());
				continue;
			}

			boo.second = cfg["bools"][boo.first];
		}

		for (auto& boo : g_Variables.m_colors) {
			if (cfg["colors"][boo.first].is_null()) {
				printf_s("missing colors variable: %s\n", boo.first.c_str());
				continue;
			}

			const nlohmann::json vector = nlohmann::json::parse(cfg["colors"][boo.first].get<std::string>());
			boo.second = Color(
				vector.at(0).get<std::uint8_t>(), // r
				vector.at(1).get<std::uint8_t>(), // g
				vector.at(2).get<std::uint8_t>(), // b
				vector.at(3).get<std::uint8_t>()  // a
			);
		}

		for (auto& boo : g_Variables.m_binds) {
			if (cfg["binds"][boo.first].is_null()) {
				printf_s("missing binds variable: %s\n", boo.first.c_str());
				continue;
			}

			const nlohmann::json vector = nlohmann::json::parse(cfg["binds"][boo.first].get<std::string>());
			boo.second = bind_t(vector.at(0).get<int>(), // key
				vector.at(1).get<int>() // mode
			);
			nlohmann::json sub = { };
		}

		for (auto& boo : g_Variables.m_ints) {
			if (cfg["ints"][boo.first].is_null()) {
				printf_s("missing ints variable: %s\n", boo.first.c_str());
				continue;
			}

			boo.second = cfg["ints"][boo.first];
		}

		for (auto& boo : g_Variables.m_floats) {
			if (cfg["floats"][boo.first].is_null()) {
				printf_s("missing floats variable: %s\n", boo.first.c_str());
				continue;
			}

			boo.second = cfg["floats"][boo.first];
		}

		for (auto& boo : g_Variables.m_strings) {
			if (cfg["strings"][boo.first].is_null()) {
				printf_s("missing strings variable: %s\n", boo.first.c_str());
				continue;
			}

			boo.second = cfg["strings"][boo.first];
		}

		for (int i = 0; i < g_skins.m_weapon_name_map.size(); i++) {
			auto info = g_skins.m_weapon_name_map[i];
			std::string format = tfm::format("%s", info.definition_index);
			if (cfg["skins"][format].is_null())
				continue;

			g_skins.m_weapon_name_map[i].set_paintkit = cfg["skins"][format];
		}

		for (int i = 0; i < g_skins.saved_kits.size(); i++) {
			auto& info = g_skins.saved_kits[i];
			std::string fmt = tfm::format("%i", i);
			if (cfg["paintkits"][fmt].is_null())
				continue;

			const nlohmann::json vector = nlohmann::json::parse(cfg["paintkits"][fmt].get<std::string>());
			info.custom_col[0] = Color(vector.at(0).get<uint32_t>());
			info.custom_col[1] = Color(vector.at(1).get<uint32_t>());
			info.custom_col[2] = Color(vector.at(2).get<uint32_t>());
			info.custom_col[3] = Color(vector.at(3).get<uint32_t>());
		}
	}

	catch (const nlohmann::detail::exception&) {
		throw std::runtime_error("Config Exception Error.");
		return;
	}
}

void GRUI::ManageConfig(const char* pFileName, CfgManageType_t nType) {
	const auto path = szDirectory + "/" + pFileName;

	switch (nType) {
	case CFG_SAVE:
		SaveConfig(path);
		g_notify.add(tfm::format("saved config %s\n", pFileName));
		break;
	case CFG_LOAD:
		LoadConfig(path);
		g_notify.add(tfm::format("loaded config %s\n", pFileName));
		break;
	case CFG_CREATE:
		CreateConfig(path);
		break;
	case CFG_DELETE:
		std::filesystem::remove(path);
		break;
	}

}