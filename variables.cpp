#include "menu.h"
#include "wnd_input.h"

GVariables g_Variables;

std::string GVariables::GetAimbotWeaponPrefix(WeaponConfig_t w) {
	if (w > WeaponConfig_t::CONFIG_MAX)
		return "";

	switch (w) {
	case CONFIG_AUTO:
		return "_auto";
	case CONFIG_SSG08:
		return "_scout";
	case CONFIG_AWP:
		return "_awp";
	case CONFIG_HEAVYPISTOL:
		return "_heavypistol";
	case CONFIG_PISTOL:
		return "_pistol";
	default:
		return "_default";
	}
}

void GVariables::InitRagebot()
{
#pragma region aimbot
	m_bools[ "aimbot_enabled" ] = false;
	m_bools[ "aimbot_autofire" ] = false;
	m_bools[ "aimbot_autostop" ] = false;
	m_bools[ "aimbot_early_stop" ] = false;
	m_bools[ "aimbot_jump_stop" ] = false;
	m_bools[ "aimbot_zeus" ] = false;
	m_bools[ "aimbot_knife" ] = false;
	m_bools[ "aimbot_silent" ] = false;
	m_bools[ "aimbot_resolve_fake" ] = false;
	m_bools[ "aimbot_resolve_desync" ] = false;
	m_bools[ "aimbot_headshot_only" ] = false;
	m_bools[ "draw_aimbox_matrix" ] = false;
	m_colors[ "aimbot_matrix_color" ] = Color( 255, 255, 255, 255 );

	m_bools[ "aimbot_reduce_delay_shot_intensity" ] = false;
	m_ints[ "aimbot_delay_shot_intensity" ] = 3;

	m_binds[ "aimbot_forcebaim" ] = bind_t();
	m_binds[ "aimbot_mindamageoverride" ] = bind_t();

	m_ints[ "aimbot_resolveroverride" ] = 0;

	m_bools[ "incease_fire_rate" ] = false;

	for ( int i = 0; i < WeaponConfig_t::CONFIG_MAX; i++ )
	{
		std::string add = GetAimbotWeaponPrefix( (WeaponConfig_t)i );
		/*
		 XOR( "head" ), XOR( "chest" ), XOR( "body" ), XOR( "arms" ), XOR( "legs" ), XOR( "feet" )
		*/
		m_bools[ "hitbox_head" + add ] = false;
		m_bools[ "hitbox_chest" + add ] = false;
		m_bools[ "hitbox_body" + add ] = false;
		m_bools[ "hitbox_arms" + add ] = false;
		m_bools[ "hitbox_legs" + add ] = false;
		m_bools[ "hitbox_feet" + add ] = false;

		m_bools[ "multipoint_head" + add ] = false;
		m_bools[ "multipoint_chest" + add ] = false;
		m_bools[ "multipoint_body" + add ] = false;
		m_bools[ "multipoint_arms" + add ] = false;
		m_bools[ "multipoint_legs" + add ] = false;
		m_bools[ "multipoint_feet" + add ] = false;

		m_floats[ "multipoint_head_scale" + add ] = 50.f;
		m_floats[ "multipoint_scale" + add ] = 50.f;
		m_bools[ "multipoint_dynamic_scale" + add ] = true;

		m_floats[ "hitchance" + add ] = 50.f;

		m_bools[ "delay_shot" + add ] = false;
		m_bools[ "delay_shot_disable_vulnerable" + add ] = true;
		m_bools[ "delay_shot_damage" + add ] = false;
		m_bools[ "delay_shot_safety" + add ] = false;
		m_floats[ "delay_shot_strictness" + add ] = 75.f;

		m_floats[ "prefer_overlap" + add ] = 40.f;
		m_floats[ "prefer_hitchance" + add ] = 80.f;

		m_bools[ "conditional_standing" + add ] = false;
		m_bools[ "conditional_unsafe_moving" + add ] = false;
		m_bools[ "conditional_in_air" + add ] = false;
		m_bools[ "conditional_already_missed" + add ] = false;

		m_floats[ "conditional_prefer_overlap" + add ] = 40.f;
		m_floats[ "conditional_prefer_hitchance" + add ] = 80.f;

		m_ints[ "avoid_head_after" + add ] = 0;

		m_floats[ "minimum_damage" + add ] = 0;
		m_floats[ "minimum_damage_override" + add ] = 0;

		m_bools["quick_switch" + add] = false;
	}
	m_bools[ "quick_switch_zeus" ] = false;
#pragma endregion

#pragma region antiaim
	m_bools[ "antiaim_enabled" ] = false;
	m_bools[ "antiaim_enabled_round_end" ] = false;
	m_bools[ "antiaim_enabled_freezetime" ] = false;
	m_bools[ "antiaim_enabled_use" ] = false;

	m_ints[ "antiaim_pitch" ] = 0;
	m_ints[ "antiaim_yaw" ] = 0;
	m_bools[ "antiaim_face_targets" ] = false;
	m_bools[ "antiaim_backwards_duck" ] = false;
	m_bools[ "antiaim_adaptive_air" ] = false;

	m_bools[ "antiaim_adaptive_lby" ] = false;
	m_bools[ "antiaim_extend_lastmoving" ] = false;
	m_ints[ "antiaim_lby_orientation" ] = 0;
	m_ints[ "antiaim_lby_angle" ] = 0;
	m_ints[ "antiaim_lby_anim" ] = 0;
	m_ints[ "antiaim_real_angle" ] = 0;
	m_ints[ "antiaim_lby_lag" ] = 6;

	m_bools[ "antiaim_twist" ] = false;
	m_bools[ "antiaim_twist_disable_lag" ] = false;
	m_bools[ "antiaim_twist_tilt_freestand" ] = false;
	m_floats[ "antiaim_twist_minimum_desync" ] = 20.f;
	m_floats[ "antiaim_twist_lock_threshold" ] = 10.f;

	m_bools[ "antiaim_transition_flick" ] = false;

	m_ints[ "antiaim_manual_forward" ] = 0;
	m_ints[ "antiaim_manual_left" ] = 0;
	m_ints[ "antiaim_manual_right" ] = 0;
	m_ints[ "antiaim_manual_back" ] = 0;

	m_bools[ "antiaim_manual_lastmove" ] = false;
	m_colors[ "antiaim_manual_lastmove_color" ] = Color( 255, 0, 0 );

	m_bools[ "antiaim_manual_arrows" ] = false;
	m_colors[ "antiaim_manual_arrows_color" ] = Color( 255, 255, 255 );
	m_ints["antiaim_manual_arrows_style"] = 0;

	m_ints[ "antiaim_leg_movement" ] = 0;
#pragma endregion

#pragma region fakelag
	m_bools[ "fakelag_enabled" ] = false;

	m_bools[ "fakelag_move" ] = false;
	m_bools[ "fakelag_air" ] = false;
	m_bools[ "fakelag_duck" ] = false;

	m_ints[ "fakelag_limit" ] = 1;
	m_bools[ "fakelag_adaptive_limit" ] = false;

	m_bools[ "fakelag_on_peek" ] = false;
#pragma endregion
}

void GVariables::InitVisuals()
{
#pragma region esp
	m_bools[ "esp_enabled" ] = false;
	m_bools[ "esp_draw_dormant" ] = false;
	m_bools[ "esp_wide_boxes" ] = false;
	m_bools[ "esp_draw_teammates" ] = false;
	m_bools[ "esp_override_dormant" ] = false;
	m_colors[ "esp_dormant" ] = Color( 130, 130, 130 );

	m_bools[ "oof_arrows" ] = false;
	m_colors[ "oof_arrows_color" ] = false;
	m_floats[ "oof_arrows_size" ] = 14.f;
	m_floats[ "oof_arrows_distance" ] = 20.f;

	m_bools[ "esp_box" ] = false;
	m_colors[ "esp_box_color" ] = Color( 255, 255, 255, 200 );

	m_bools[ "esp_name" ] = false;
	m_ints[ "esp_name_font" ] = 0;
	m_colors[ "esp_name_color" ] = Color( 255, 255, 255, 200 );

	m_bools[ "esp_health" ] = false;
	m_colors[ "esp_health_start_color" ] = Color( 120, 225, 80 );
	m_colors[ "esp_health_end_color" ] = Color( 255, 50, 80 );
	m_bools[ "esp_health_text" ] = false;
	m_floats[ "esp_health_bar_alpha" ] = 78.f;

	m_bools[ "esp_weapon" ] = false;

	m_bools[ "esp_weapon_text" ] = false;
	m_colors[ "esp_weapon_text_color" ] = Color( 255, 255, 255, 200 );

	m_bools[ "esp_weapon_icon" ] = false;
	m_colors[ "esp_weapon_icon_color" ] = Color( 255, 255, 255, 200 );
	m_colors[ "esp_weapon_zeus_color" ] = Color( 255, 222, 0, 200 );

	m_bools[ "esp_weapon_ammo" ] = false;
	m_colors[ "esp_weapon_ammo_start_color" ] = Color( 60, 120, 180 );
	m_colors[ "esp_weapon_ammo_end_color" ] = Color( 60, 120, 180 );
	m_bools[ "esp_weapon_ammo_text" ] = false;
	m_floats[ "esp_weapon_ammo_bar_alpha" ] = 78.f;

	m_bools[ "esp_lby_update" ] = false;
	m_colors[ "esp_lby_update_color" ] = Color( 60, 120, 180 );
	m_colors[ "esp_lby_update_unsafe_color" ] = Color( 255, 0, 0 );
	m_floats[ "esp_lby_update_bar_alpha" ] = 78.f;

	m_bools[ "esp_flags" ] = false;
	m_bools[ "esp_flags_money" ] = false;
	m_bools[ "esp_flags_armor" ] = false;
	m_bools[ "esp_flags_ping" ] = false;
	m_bools[ "esp_flags_defusing" ] = false;
	m_bools[ "esp_flags_c4" ] = false;
	m_bools[ "esp_flags_scoped" ] = false;
	m_bools[ "esp_flags_flashed" ] = false;
	m_bools[ "esp_flags_reload" ] = false;
	m_bools[ "esp_flags_exploit" ] = false;
	m_bools[ "esp_flags_lagcomp" ] = false;
	m_bools[ "esp_flags_resolver_confidence" ] = false;
	m_bools[ "esp_flags_desync" ] = false;
	m_bools[ "esp_flags_distortion" ] = false;

	m_bools[ "brighten_ping" ] = false;
	m_bools[ "capitalize_ping" ] = false;

	m_bools[ "esp_skeleton" ] = false;
	m_colors[ "esp_skeleton_color" ] = Color( 255, 255, 255, 200 );

	m_bools[ "hitmarker_sound" ] = false;
	m_bools[ "hitmarker_screen" ] = false;
	m_bools[ "hitmarker_world" ] = false;
	m_bools[ "hitmarker_damage" ] = false;

	m_bools[ "hitmarker_antialias_world" ] = false;
	m_bools[ "hitmarker_antialias_screen" ] = false;

	m_ints[ "hitmarker_screen_length" ] = 8;
	m_ints[ "hitmarker_world_length" ] = 8;

	m_colors[ "hitmarker_world_default" ] = m_colors[ "hitmarker_screen_default" ] = m_colors[ "hitmarker_damage_default" ] = Color( 255, 255, 255, 200 );
	m_colors[ "hitmarker_world_headshot" ] = m_colors[ "hitmarker_screen_headshot" ] = m_colors[ "hitmarker_damage_headshot" ] = Color( 255, 0, 0, 200 );
	m_colors[ "hitmarker_world_lethal" ] = m_colors[ "hitmarker_screen_lethal" ] = m_colors[ "hitmarker_damage_lethal" ] = Color( 255, 0, 0, 200 );

	m_bools[ "esp_local_skeleton" ] = false;
	m_colors[ "esp_local_skeleton_color" ] = Color( 255, 255, 255, 200 );
#pragma endregion

#pragma region models
	m_bools[ "glow_enemy" ] = false;
	m_ints[ "glow_enemy_style" ] = 0;
	m_colors[ "glow_enemy_color" ] = Color( 180, 60, 120 );

	m_bools[ "glow_friendly" ] = false;
	m_ints[ "glow_friendly_style" ] = 0;
	m_colors[ "glow_friendly_color" ] = Color( 255, 255, 255 );

	m_bools[ "glow_local" ] = false;
	m_ints[ "glow_local_style" ] = 0;
	m_colors[ "glow_local_color" ] = Color( 255, 255, 255 );

	m_bools[ "glow_weapons" ] = false;
	m_ints[ "glow_weapons_style" ] = 0;
	m_colors[ "glow_weapons_color" ] = Color( 255, 255, 255 );

	m_bools[ "chams_enemy" ] = false;
	m_bools[ "chams_enemy_alpha_phong" ] = false;
	m_colors[ "chams_enemy_color" ] = Color( 150, 200, 60 );
	m_colors[ "chams_enemy_shine_color" ] = Color( 150, 200, 60 );
	m_ints[ "chams_enemy_material" ] = 0;
	m_ints[ "chams_enemy_phong_amount" ] = 100;

	m_bools[ "chams_enemy_occluded" ] = false;
	m_colors[ "chams_enemy_occluded_color" ] = Color( 60, 120, 180 );

	m_bools[ "chams_enemy_history" ] = false;
	m_colors[ "chams_enemy_history_color" ] = Color( 255, 255, 255 );

	m_bools[ "chams_friendly" ] = false;
	m_colors[ "chams_friendly_color" ] = Color( 255, 255, 255 );
	m_colors[ "chams_friendly_shine_color" ] = Color( 150, 200, 60 );
	m_ints[ "chams_friendly_material" ] = 0;
	m_ints[ "chams_friendly_phong_amount" ] = 100;
	m_bools[ "chams_friendly_alpha_phong" ] = false;

	m_bools[ "chams_friendly_occluded" ] = false;
	m_colors[ "chams_friendly_occluded_color" ] = Color( 255, 255, 255 );

	m_bools[ "chams_local" ] = false;
	m_ints[ "chams_local_material" ] = 0;
	m_colors[ "chams_local_shine_color" ] = Color( 150, 200, 60 );
	m_colors[ "chams_local_color" ] = Color( 255, 255, 255 );
	m_ints[ "chams_local_phong_amount" ] = 100;
	m_bools[ "chams_local_alpha_phong" ] = false;

	m_bools[ "chams_local_lby" ] = false;
	m_colors[ "chams_local_lby_color" ] = Color( 255, 255, 255 );

	m_bools[ "chams_enemy_ragdoll" ] = false;
	m_bools[ "chams_enemy_ragdoll_occluded" ] = false;
	//m_ints["chams_enemy_ragdoll_material"] = 0; // maybe?
	m_colors[ "chams_enemy_ragdoll_color" ] = Color( 255, 255, 255 );
	m_colors[ "chams_enemy_ragdoll_occluded_color" ] = Color( 255, 255, 255 );

	m_bools[ "local_blend_scope" ] = false;
	m_bools[ "local_blend_grenade" ] = false;
	m_bools[ "disable_chams_on_blend" ] = false;
	m_floats[ "local_blend_amt" ] = 20.f;
	m_bools[ "local_override_gloves" ] = false;

	m_floats[ "global_cham_tint" ] = 90.f;
#pragma endregion

#pragma region projectiles
	m_bools[ "local_nade_prediction" ] = false;
	m_colors[ "local_nade_prediction_color" ] = Color( 255, 255, 255 );

	m_bools[ "draw_grenades" ] = false;
	m_colors[ "draw_grenades_color" ] = Color( 255, 255, 255 );

	m_bools[ "bullet_beams" ] = false;
	m_bools[ "local_beams" ] = false;
	m_colors[ "local_beam_color" ] = Color( 255, 255, 255 );
	m_bools[ "enemy_beams" ] = false;
	m_colors[ "enemy_beam_color" ] = Color( 255, 255, 255 );
	m_bools[ "friendly_beams" ] = false;
	m_colors[ "friendly_beam_color" ] = Color( 255, 255, 255 );

	m_floats[ "beam_lifetime" ] = 3.f;
	m_floats[ "beam_thickness" ] = 3.f;
	m_ints[ "beam_type" ] = 0;
#pragma endregion

#pragma region weapons
	m_bools[ "dropped_weapons" ] = false;

	m_bools[ "dropped_weapon_text" ] = false;
	m_colors[ "dropped_weapon_text_color" ] = Color( 255, 255, 255 );

	m_bools[ "dropped_weapon_icon" ] = false;
	m_colors[ "dropped_weapon_icon_color" ] = Color( 255, 255, 255 );

	m_bools[ "planted_bomb" ] = false;
	m_bools[ "planted_bomb_world" ] = false;
	m_bools[ "planted_bomb_screen" ] = false;
#pragma endregion

#pragma region modulation
	m_bools[ "world_modulation" ] = false;
	m_colors[ "world_color" ] = Color( 43, 41, 46 );
	m_colors[ "prop_color" ] = Color( 125, 125, 125 );
	m_colors[ "sky_color" ] = Color( 255, 255, 255 );
	m_colors[ "fire_color" ] = Color( 255, 255, 255 );

	m_bools[ "ambient_light" ] = false;
	m_colors[ "ambient_light_color" ] = Color( 255, 255, 255 );

	m_bools[ "world_fullbright" ] = false;

	m_bools[ "fog_modulation" ] = false;
	m_colors[ "fog_color" ] = Color( 255, 255, 255, 150 );
	m_floats[ "fog_start" ] = 0.f;
	m_floats[ "fog_end" ] = 0.f;
	m_floats[ "fog_density" ] = 0.f;

	m_bools[ "remove_kick_recoil" ] = false;
	m_bools[ "remove_smooth_recoil" ] = false;
	m_bools[ "remove_landing_bob" ] = false;
	m_bools[ "remove_landing_punch" ] = false; // not done yet
	m_bools[ "remove_smoke_grenades" ] = false;
	m_bools[ "remove_fog" ] = false;
	m_bools[ "remove_flashbangs" ] = false;
	m_bools[ "remove_scope" ] = false;
	m_bools[ "remove_post_processing" ] = false;
	m_bools[ "remove_scope_processing" ] = false;
	m_bools[ "remove_teammates" ] = false;
	m_bools[ "remove_sleeves" ] = false;
#pragma endregion

#pragma region view
	m_bools[ "fov_changer" ] = false;
	m_floats[ "view_fov" ] = 90.f;
	m_floats[ "fov_scope_percent" ] = 35.f;
	m_floats[ "viewmodel_fov" ] = 60.f;

	m_bools[ "force_crosshair" ] = false;
	m_bools[ "penetration_crosshair" ] = false;
	m_ints[ "penetration_crosshair_style" ] = 0;
	m_bools[ "spectator_list" ] = false;
	m_bools[ "viewmodel_in_scope" ] = false;

	m_colors[ "pen_active" ] = colors::transparent_green;
	m_colors[ "pen_inactive" ] = colors::transparent_red;
	m_colors[ "pen_player" ] = colors::transparent_yellow;
	m_ints[ "skip_scope_lines_pen" ] = 0;

	m_bools[ "spread_crosshair" ] = false;
	m_colors[ "spread_crosshair_color" ] = Color( 255, 255, 255 );

	m_binds[ "thirdperson" ] = bind_t();
	m_floats[ "thirdperson_distance" ] = 150;

	m_bools[ "lby_indicator" ] = false;
	m_bools[ "lc_indicator" ] = false;
	m_bools[ "ping_indicator" ] = false;
	m_bools[ "dmg_indicator" ] = false;
	m_bools[ "baim_indicator" ] = false;
#pragma endregion
}

void GVariables::InitMisc()
{
	m_bools[ "quickswitch" ] = false;
	m_bools[ "autojump" ] = false;
	m_bools[ "autostrafe" ] = false;
	m_floats[ "autostrafe_speed" ] = 50.f;
	m_bools[ "air_duck" ] = false;

	m_binds[ "fakewalk" ] = bind_t();
	m_ints[ "fakewalk_max_delay" ] = 0;

	m_binds[ "autopeek" ] = bind_t();
	m_binds[ "edge jump" ] = bind_t();

	m_bools[ "ragdoll_force" ] = false;

	m_bools[ "buybot" ] = false;
	m_ints[ "buybot_primary" ] = 0;
	m_ints[ "buybot_secondary" ] = 0;
	m_bools[ "buybot_grenades" ] = false;
	m_bools[ "buybot_armor" ] = false;
	m_bools[ "buybot_taser" ] = false;
	m_bools[ "buybot_ignorepistol" ] = false;

	m_bools[ "fakelatency" ] = false;
	m_binds[ "fakelatency_spike" ] = bind_t();
	m_floats[ "fakelatency_default" ] = 150.f;
	m_floats[ "fakelatency_spike" ] = 1000.f;

	m_bools[ "unlock_inventory" ] = true;
	m_bools[ "preserve_killfeed" ] = false;
	m_bools[ "clantag_spammer" ] = false;
	m_ints[ "chat_spam_type" ] = 0;

	m_bools[ "skybox_changer" ] = false;
	m_ints[ "skybox_change_name" ] = 0;
	m_strings["skybox_custom"] = "";

	m_bools[ "draw_impacts" ] = false;
	m_colors[ "client_impact_color" ] = Color( 255, 0, 0, 125 );
	m_colors[ "server_impact_color" ] = Color( 0, 0, 255, 125);
	m_floats[ "impacts_size" ] = 100.f;
	m_floats["impacts_time"] = 2.f;

	m_bools[ "log_damage" ] = false;
	m_bools[ "log_harmed" ] = false;
	m_bools[ "log_purchases" ] = false;
	m_bools[ "log_bomb" ] = false;
	m_bools[ "log_defuse" ] = false;
	m_binds[ "last_tick_defuse" ] = bind_t();

	m_strings["autoexec"] = "";

	m_bools["remove_model_sway"] = false;
	m_bools["force_local_footplant"] = false;
	m_bools[ "smooth_local_anims" ] = false;

	m_bools["center_damage_logs"] = false;
	m_bools["increase_hit_effects"] = false;

	m_ints[ "race_changer" ] = 0;
}

void GVariables::InitSkins() {
	m_ints["knife_index"] = 0;
	m_ints["glove_index"] = 0;
}

bool GVariables::get_keystate(bind_t bind){
	switch (bind.mode) {
	case bindmode_t::ALWAYS:
		return true;
	case bindmode_t::HELD:
		return g_input.key_states[bind.key].state;
	case bindmode_t::HELDOFF:
		return !g_input.key_states[bind.key].state;
	case bindmode_t::TOGGLE:
		return g_input.key_states[bind.key].toggled_state;
	default:
		return false; // maybe print something so we know we did something wrong?
	}
}