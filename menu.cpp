#include "includes.h"

GRUI g_GUI;
#include <map>
#include "Sound.h"
#include <urlmon.h>
#include <mmsystem.h>
#include <functional>

#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "Winmm.lib")
#pragma comment( lib, "Iphlpapi.lib" )

std::vector<const char*> vecTabs = { "rage", "visuals", "miscellaneous", "skins", "players" };

std::map<int, std::vector<const char*>> vecSubTabs = {
    {0, {"aimbot", "anti-aim"}},
    {1, {"players", "world"}},
    {2, {"main", "configuration"}},
    {3, {"main##2"}},
    {4, {"main##3"}},
    {5, {"main##2"}},
};

int curr_tab, last_curr_tab, curr_subtab, last_curr_subtab;
int btnclicks;
std::map<int, int> saved_subtab;

float CalcAlpha(float n, float max = 1.f) {
    n = (n > 0.0f) * n + !(n > 0.0f) * 0.0f;
    return (n < max) * n + !(n < max) * max;
}

#define TAB_CHECK(x) \
if (curr_tab != x) \
return;\

void GRUI::Render() {
    if ( !g_csgo.m_done )
        return;

    ImGui::GetStyle().Alpha = m_is_open ? CalcAlpha(ImGui::GetStyle().Alpha + g_Variables.m_floats["menu_fade_speed"] * ImGui::GetIO().DeltaTime) : CalcAlpha(ImGui::GetStyle().Alpha - g_Variables.m_floats["menu_fade_speed"] * ImGui::GetIO().DeltaTime);

    if (ImGui::GetStyle().Alpha <= 0.f) {
        ImGui::SetNextWindowPos(size);
        return;
    }

    ImGui::SetNextWindowSize(size);
    Color col = g_Variables.m_colors["menu_accent_color"];
    if (ImGui::Begin("penis", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar)) {
#pragma region base_framework
        ImGui::GetStyle().Colors[ImGuiCol_CheckMark] = ImVec4(col.r() / 255.f, col.g() / 255.f, col.b() / 255.f, col.a() / 255.f);
        ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, { 0.5f, 0.5f });
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.f, 0.f });
        if (ImGui::BeginChild("0", { 630 * 0.2, 50 }, ImGuiChildFlags_FrameStyle)) { // title
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(col.r() / 255.f, col.g() / 255.f, col.b() / 255.f, col.a() / 255.f));
            if (ImGui::Button("Neverlose.cc", { 630 * 0.2, 50 })) {
                btnclicks++;
                if (btnclicks > 1) {
                    ImGui::SetNextWindowSize({ 400, 150 });
                    ImGui::OpenPopup("GlobalOptions");
                    btnclicks = 0;
                }
            }
            ImGui::PopStyleColor();

            if (ImGui::BeginPopup("GlobalOptions")) {
                //g_GUI.Checkbox("Background tint", &g_Variables.background_shade);
                Slider("fade speed", &g_Variables.m_floats["menu_fade_speed"], 1.f, 10.f, "%.2f");
                Checkbox("menu shade", &g_Variables.m_bools["menu_shade"]);
                Checkbox("watermark", &g_Variables.m_bools["watermark"]);
                Colorpicker("accent color", &g_Variables.m_colors["menu_accent_color"]);
                ImGui::EndPopup();
            }
            ImGui::EndChild();
        }
        ImGui::PopStyleVar(2);

        ImGui::SameLine(0, 4.f);

        ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, { 0.5f, 0.5f });
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.f, 0.f });
        if (ImGui::BeginChild("Subtabs", { 630 * 0.8, 50 }, ImGuiChildFlags_FrameStyle, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar)) {
            for (auto& subtab : vecSubTabs) {
                if (curr_tab != subtab.first)
                    continue;

                for (int i = 0; i < subtab.second.size(); i++) {
                    std::string text = std::to_string(i);
                    if (curr_subtab == i) {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(col.r() / 255.f, col.g() / 255.f, col.b() / 255.f, col.a() / 255.f));
                        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]);
                        last_curr_subtab = i;
                    }

                    if (ImGui::Button(subtab.second[i], { static_cast<float>(504 / subtab.second.size()), 50 }) && curr_subtab != i) {
                        curr_subtab = i;
                        saved_subtab[curr_tab] = curr_subtab;
                    }

                    if (i < 3)
                        ImGui::SameLine(0, 0.f);

                    if (curr_subtab == i && last_curr_subtab == i)
                        ImGui::PopStyleColor(2); // stupid thing that happens but anyways if we change tabs then this will get called twice(button changes the tab etc), so we make sure our last tab is also the same to prevent this.
                }
            }
            ImGui::EndChild();
        }
        ImGui::PopStyleVar(2);

        if (ImGui::BeginChild("Tabs", { 630 * 0.2, 430 }, ImGuiChildFlags_FrameStyle, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar)) {
            for (int i = 0; i < vecTabs.size(); i++) {
                if (curr_tab == i) {
                    last_curr_tab = i;
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(col.r() / 255.f, col.g() / 255.f, col.b() / 255.f, col.a() / 255.f));
                }

                if (Selectable(vecTabs[i], curr_tab == i, 0, { 0, static_cast<float>(300.f / vecTabs.size()) }) && curr_tab != i) {
                    last_curr_tab = curr_tab;
                    curr_tab = i;
                    curr_subtab = saved_subtab[curr_tab];
                    continue;
                }

                if (curr_tab == i)
                    ImGui::PopStyleColor();
            }
            ImGui::EndChild();
        }
#pragma endregion

        // create child box to create other 4 child boxes.

        ImGui::SetCursorPos(ImVec2(138, 62));
        // there is a better way to do this alpha thing, instead we can have an overlapping rect (above the child) thats the original color. then, that rect decreases in alpha so that the alpha of every element 
        // gives the illusion of appearing and we dont have to mess with PushStyleColor / PopStyleColor for every elements style

        if (ImGui::BeginChild("Basechild", ImVec2(252, 430), ImGuiChildFlags_FrameStyle, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar)) {
            switch (curr_tab) {
            case 0:
                RageTab(); break;
            case 1:
                VisualsTab(); break;
            case 2:
                MiscTab(); break;
            case 3:
                SkinsTab(); break;
            case 4:
                PlayersTab(); break;
            }
            ImGui::EndChild();
        }

        ImGui::SetCursorPos(ImVec2(390, 62));
        if (ImGui::BeginChild("Basechild2", ImVec2(252, 430), ImGuiChildFlags_FrameStyle, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar)) {
            switch (curr_tab) {
            case 0:
                RageTab(1); break;
            case 1:
                VisualsTab(1); break;
            case 2:
                MiscTab(1); break;
            case 3:
                SkinsTab(1); break;
            case 4:
                PlayersTab(1); break;
            }
            ImGui::EndChild();
        }

        ImGui::End();
    }

    if (g_Variables.m_bools["menu_shade"])
        ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), ImVec2(g_ctx.m_width, g_ctx.m_height), Color(0, 0, 0, ImGui::GetStyle().Alpha * 144.f));
}

void copyaim( int from, int to )
{
    std::string add = g_Variables.GetAimbotWeaponPrefix( (WeaponConfig_t)to );
    std::string add2 = g_Variables.GetAimbotWeaponPrefix( (WeaponConfig_t)from );

    g_Variables.m_bools[ "hitbox_head" + add ] = g_Variables.m_bools[ "hitbox_head" + add2 ];
    g_Variables.m_bools[ "hitbox_chest" + add ] = g_Variables.m_bools[ "hitbox_chest" + add2 ];
    g_Variables.m_bools[ "hitbox_body" + add ] = g_Variables.m_bools[ "hitbox_body" + add2 ];
    g_Variables.m_bools[ "hitbox_arms" + add ] = g_Variables.m_bools[ "hitbox_arms" + add2 ];
    g_Variables.m_bools[ "hitbox_legs" + add ] = g_Variables.m_bools[ "hitbox_legs" + add2 ];
    g_Variables.m_bools[ "hitbox_feet" + add ] = g_Variables.m_bools[ "hitbox_feet" + add2 ];
    g_Variables.m_bools[ "multipoint_head" + add ] = g_Variables.m_bools[ "multipoint_head" + add2 ];
    g_Variables.m_bools[ "multipoint_chest" + add ] = g_Variables.m_bools[ "multipoint_chest" + add2 ];
    g_Variables.m_bools[ "multipoint_body" + add ] = g_Variables.m_bools[ "multipoint_body" + add2 ];
    g_Variables.m_bools[ "multipoint_arms" + add ] = g_Variables.m_bools[ "multipoint_arms" + add2 ];
    g_Variables.m_bools[ "multipoint_legs" + add ] = g_Variables.m_bools[ "multipoint_legs" + add2 ];
    g_Variables.m_bools[ "multipoint_feet" + add ] = g_Variables.m_bools[ "multipoint_feet" + add2 ];
    g_Variables.m_floats[ "multipoint_head_scale" + add ] = g_Variables.m_floats[ "multipoint_head_scale" + add2 ];
    g_Variables.m_floats[ "multipoint_scale" + add ] = g_Variables.m_floats[ "multipoint_scale" + add2 ];
    g_Variables.m_bools[ "multipoint_dynamic_scale" + add ] = g_Variables.m_bools[ "multipoint_dynamic_scale" + add2 ];
    g_Variables.m_floats[ "hitchance" + add ] = g_Variables.m_floats[ "hitchance" + add2 ];
    g_Variables.m_ints[ "damage_safety_threshold" + add ] = g_Variables.m_ints[ "damage_safety_threshold" + add2 ];
    g_Variables.m_ints[ "avoid_head_after" + add ] = g_Variables.m_ints[ "avoid_head_after" + add2 ];
    g_Variables.m_floats[ "minimum_damage" + add ] = g_Variables.m_floats[ "minimum_damage" + add2 ];
    g_Variables.m_floats[ "minimum_damage_override" + add ] = g_Variables.m_floats[ "minimum_damage_override" + add2 ];
    g_Variables.m_floats[ "prefer_overlap" + add ] = g_Variables.m_floats[ "prefer_overlap" + add2 ];
    g_Variables.m_floats[ "prefer_hitchance" + add ] = g_Variables.m_floats[ "prefer_hitchance" + add2 ];
    g_Variables.m_bools[ "conditional_standing" + add ] = g_Variables.m_bools[ "conditional_standing" + add2 ];
    g_Variables.m_bools[ "conditional_unsafe_moving" + add ] = g_Variables.m_bools[ "conditional_unsafe_moving" + add2 ];
    g_Variables.m_bools[ "conditional_in_air" + add ] = g_Variables.m_bools[ "conditional_in_air" + add2 ];
    g_Variables.m_bools[ "conditional_already_missed" + add ] = g_Variables.m_bools[ "conditional_already_missed" + add ];
    g_Variables.m_floats[ "conditional_prefer_overlap" + add ] = g_Variables.m_floats[ "conditional_prefer_overlap" + add2 ];
    g_Variables.m_floats[ "conditional_prefer_hitchance" + add ] = g_Variables.m_floats[ "conditional_prefer_hitchance" + add2 ];
    g_Variables.m_bools[ "delay_shot" + add ] = g_Variables.m_bools[ "delay_shot" + add2 ];
    g_Variables.m_bools[ "delay_shot_disable_vulnerable" + add ] = g_Variables.m_bools[ "delay_shot_disable_vulnerable" + add2 ];
    g_Variables.m_bools[ "delay_shot_damage" + add ] = g_Variables.m_bools[ "delay_shot_damage" + add2 ];
    g_Variables.m_bools[ "delay_shot_safety" + add ] = g_Variables.m_bools[ "delay_shot_safety" + add2 ];
    g_Variables.m_floats[ "delay_shot_strictness" + add ] = g_Variables.m_floats[ "delay_shot_strictness" + add2 ];
}

void GRUI::RageTab(int iColumn) {
    switch (curr_subtab) {
    case 0:
        if (iColumn == 0) {
            MAKECHILD("main") {
                static std::function<void()> enabled_cog = [&]()
                    {
                        Checkbox("silent-aim", &g_Variables.m_bools["aimbot_silent"]);
                        Checkbox("zeus bot", &g_Variables.m_bools["aimbot_zeus"]);
                        Checkbox( "knife bot", &g_Variables.m_bools[ "aimbot_knife" ] );
                    };

                Checkbox("enable", &g_Variables.m_bools["aimbot_enabled"], true, enabled_cog);

                static std::function<void()> autostop_cog = [&]()
                    {
                        MultiCombo( "extras", {
                            {&g_Variables.m_bools[ "aimbot_early_stop" ], "early" },
                            {&g_Variables.m_bools[ "aimbot_jump_stop" ], "jump-scout" }
                            } );
                    };

                Checkbox( "auto-stop", &g_Variables.m_bools[ "aimbot_autostop" ], true, autostop_cog );

                Checkbox( "auto-fire", &g_Variables.m_bools[ "aimbot_autofire" ] );
                Checkbox( "headshot only", &g_Variables.m_bools[ "aimbot_headshot_only" ] );

                static std::function<void()> weapon_cog = [&]()
                    {
                        Combo("type", &m_selected_aimweapon, "default\0auto\0scout\0awp\0heavy pistol\0pistol\0");
                        if (ImGui::Button("copy")) {
                            m_aimweapon_copy = m_selected_aimweapon;
                        }
                        ImGui::SameLine(); 
                        if (ImGui::Button("paste")) {
                            // set info for weapon-specific cfg vars...
                            copyaim(m_aimweapon_copy, m_selected_aimweapon);
                        }
                    };

                ImGui::Text("current weapon"); ImGui::SameLine(); StandaloneCog("weaoib##bruh", weapon_cog);
            } ENDCHILD

            MAKECHILD("other") {
                MultiCombo("resolve anti-aim", {
                    {&g_Variables.m_bools["aimbot_resolve_fake"], "fake-angles"},
                    {&g_Variables.m_bools["aimbot_resolve_desync"], "desync"},
                    });

                Checkbox( "draw aimbox matrix", &g_Variables.m_bools[ "draw_aimbot_matrix" ] );
                ImGui::SameLine(); Colorpicker( "##aimbot_matrix.color", &g_Variables.m_colors[ "aimbot_matrix_color" ] );

                std::function<void()> optimizations_cog = [&]()
                    {
                        Slider( "intensity reduction", &g_Variables.m_ints[ "aimbot_delay_shot_intensity" ], 0, 8 );
                    };

                MultiCombo( "optimizations", {
                    {&g_Variables.m_bools["aimbot_reduce_delay_shot_intensity" ], "reduce delay shot intensity" }
                    } );

                if ( g_Variables.m_bools[ "aimbot_reduce_delay_shot_intensity" ] )
                {
                    ImGui::SameLine(); 
                    StandaloneCog( "optimi##bruh", optimizations_cog );
                }
            }ENDCHILD
        }

        if ( iColumn == 1 )
        {
            std::string add = g_Variables.GetAimbotWeaponPrefix( (WeaponConfig_t)m_selected_aimweapon );
            std::string current_wpncfg = "weapon" + add;
            MAKECHILD( current_wpncfg.c_str() )
            {
                MultiCombo( "hitboxes", {
                    {&g_Variables.m_bools[ "hitbox_head" + add ], "head"},
                    {&g_Variables.m_bools[ "hitbox_chest" + add ], "chest"},
                    {&g_Variables.m_bools[ "hitbox_body" + add ], "body"},
                    {&g_Variables.m_bools[ "hitbox_arms" + add ], "arms"},
                    {&g_Variables.m_bools[ "hitbox_legs" + add ], "legs"},
                    {&g_Variables.m_bools[ "hitbox_feet" + add ], "feet"},
                    } );

                std::function<void()> multipoint_cog = [&]()
                    {
                        Slider( "head scale", &g_Variables.m_floats[ "multipoint_head_scale" + add ], 1.f, 100.f, "%.f%%" );;
                        Slider( "point scale", &g_Variables.m_floats[ "multipoint_scale" + add ], 1.f, 100.f, "%.f%%" );;
                        Checkbox( "dynamic scale", &g_Variables.m_bools[ "multipoint_dynamic_scale" + add ] );
                    };

                MultiCombo( "multipoint", {
                    {&g_Variables.m_bools[ "multipoint_head" + add ], "head"},
                    {&g_Variables.m_bools[ "multipoint_chest" + add ], "chest"},
                    {&g_Variables.m_bools[ "multipoint_body" + add ], "body"},
                    {&g_Variables.m_bools[ "multipoint_arms" + add ], "arms"},
                    {&g_Variables.m_bools[ "multipoint_legs" + add ], "legs"},
                    {&g_Variables.m_bools[ "multipoint_feet" + add ], "feet"},
                    } ); ImGui::SameLine(); StandaloneCog( "mpoint##bruh", multipoint_cog );

                Slider( "hitchance", &g_Variables.m_floats[ "hitchance" + add ], 1.f, 100.f, "%.f%%" );
                Slider( "min. damage", &g_Variables.m_floats[ "minimum_damage" + add ], 1.f, 100.f, "%.f" );

                if ( g_Variables.m_binds[ "aimbot_mindamageoverride" ].key > 0 )
                    Slider( "min. damage ovr", &g_Variables.m_floats[ "minimum_damage_override" + add ], 1.f, 100.f, "%.f" );

                std::function<void()> delay_shot_cog = [&]()
                    {
                        Checkbox( "disable if vulnerable", &g_Variables.m_bools[ "delay_shot_disable_vulnerable" + add ] );

                        MultiCombo( "conditions", {
                            {&g_Variables.m_bools[ "delay_shot_damage" + add ], "damage with safety" },
                            {&g_Variables.m_bools[ "delay_shot_safety" + add ], "safety with damage" }
                            } );

                        Slider( "strictness", &g_Variables.m_floats[ "delay_shot_strictness" + add ], 10.f, 90.f, "%.f%%" );
                    };

                Checkbox( "delay shot", &g_Variables.m_bools[ "delay_shot" + add ], true, delay_shot_cog );

                std::function<void()> hitscan_cog = [&]()
                    {
                        ImGui::Text( "prefer overlap" );
                        Slider( "##1", &g_Variables.m_floats[ "prefer_overlap" + add ], 0.f, 100.f, "%.f%%" );
                        if ( ImGui::IsItemHovered() )
                        {
                            ImGui::BeginTooltip();
                            ImGui::Text( "preference for fake-angle safety" );
                            ImGui::EndTooltip();
                        }

                        ImGui::Text( "prefer hitchance" );
                        Slider( "##2", &g_Variables.m_floats[ "prefer_hitchance" + add ], 0.f, 100.f, "%.f%%" );
                        if ( ImGui::IsItemHovered() )
                        {
                            ImGui::BeginTooltip();
                            ImGui::Text( "preference for spread safety" );
                            ImGui::EndTooltip();
                        }

                        ImGui::Text( "conditional hitscan" );
                        MultiCombo( "##3", {
                            {&g_Variables.m_bools[ "conditional_standing" + add ], "standing" },
                            {&g_Variables.m_bools[ "conditional_unsafe_moving" + add ], "unsafe moving" },
                            {&g_Variables.m_bools[ "conditional_in_air" + add ], "in air" },
                            {&g_Variables.m_bools[ "conditional_already_missed" + add ], "already missed" }
                            } );

                        if ( g_Variables.m_bools[ "conditional_standing" + add ]
                            || g_Variables.m_bools[ "conditional_unsafe_moving" + add ]
                            || g_Variables.m_bools[ "conditional_in_air" + add ]
                            || g_Variables.m_bools[ "conditional_already_missed" + add ] )
                        {
                            ImGui::Text( "conditional prefer overlap" );
                            Slider( "##4", &g_Variables.m_floats[ "conditional_prefer_overlap" + add ], 0.f, 100.f, "%.f%%" );

                            ImGui::Text( "conditional prefer hitchance" );
                            Slider( "##5", &g_Variables.m_floats[ "conditional_prefer_hitchance" + add ], 0.f, 100.f, "%.f%%" );
                        }

                        Slider( "avoid head", &g_Variables.m_ints[ "avoid_head_after" + add ], 0, 20, g_Variables.m_ints[ "avoid_head_after" + add ] > 0 ? "after %d miss(es)" : "disabled" );
                    };

                ImGui::Text( "hitscan" ); ImGui::SameLine(); StandaloneCog( "hiscan##bruh", hitscan_cog );

            }ENDCHILD

            MAKECHILD( "keybinds" )
            {
                ImGui::Text( "force baim" ); ImGui::SameLine(); ImGui::Keybind( "##00001", &g_Variables.m_binds[ "aimbot_forcebaim" ].key, &g_Variables.m_binds[ "aimbot_forcebaim" ].mode );
                ImGui::Text( "min. damage override" ); ImGui::SameLine(); ImGui::Keybind( "##00002", &g_Variables.m_binds[ "aimbot_mindamageoverride" ].key, &g_Variables.m_binds[ "aimbot_mindamageoverride" ].mode );
                ImGui::Text( "resolver override" ); ImGui::SameLine(); ImGui::Keybind( "##00003", &g_Variables.m_ints[ "aimbot_resolveroverride" ], nullptr, false );
                Checkbox( "increase fire rate", &g_Variables.m_bools[ "increase_fire_rate" ] );
            }ENDCHILD
        }
        break;
    case 1:
        if ( iColumn == 0 )
        {
            MAKECHILD( "anti-aim" )
            {
                std::function<void()> aa_enable_cog = [&]()
                    {
                        Checkbox( "round end", &g_Variables.m_bools[ "antiaim_enabled_round_end" ] );
                        Checkbox( "freezetime", &g_Variables.m_bools[ "antiaim_enabled_freezetime" ] );
                        Checkbox( "in use", &g_Variables.m_bools[ "antiaim_enabled_use" ] );
                    };

                Checkbox( "enabled", &g_Variables.m_bools[ "antiaim_enabled" ], true, aa_enable_cog );

                std::function<void()> yaw_cog = [&]()
                    {
                        Checkbox( "face targets", &g_Variables.m_bools[ "antiaim_face_targets" ] );
                        Checkbox( "backwards on duck", &g_Variables.m_bools[ "antiaim_backwards_duck" ] );
                        Checkbox( "adaptive in air", &g_Variables.m_bools[ "antiaim_adaptive_air" ] );
                    };

                Combo( "pitch", &g_Variables.m_ints[ "antiaim_pitch" ], "up\0down\0zero\0" );
                Combo( "yaw base", &g_Variables.m_ints[ "antiaim_yaw" ], "freestand\0backwards\0" );
                ImGui::SameLine(); StandaloneCog("taw##bruh", yaw_cog );

                std::function<void()> adaptive_lby_cog = [&]()
                    {
                        Checkbox( "extend lastmoving", &g_Variables.m_bools[ "antiaim_extend_lastmoving" ] );

                        if ( g_Variables.m_bools[ "antiaim_extend_lastmoving" ] )
                        {
                            Combo( "lby side", &g_Variables.m_ints[ "antiaim_lby_orientation" ], "peek\0hide\0" );
                            Combo( "lby angle", &g_Variables.m_ints[ "antiaim_lby_angle" ], "safest\0furthest\0" );
                        }

                        Combo( "anim type", &g_Variables.m_ints[ "antiaim_lby_anim" ], "force\0supress\0" );

                        if ( g_Variables.m_ints[ "antiaim_lby_anim" ] == 0 )
                            Combo( "real angle preference", &g_Variables.m_ints[ "antiaim_real_angle" ], "default\0closest\0" );

                        auto& bro = g_Variables.m_ints[ "antiaim_lby_lag" ];

                        Slider( "lag limit", &bro, 1, g_csgo.sv_maxusrcmdprocessticks->GetInt() - 1, bro == 1 ? "default" : "%d" );
                    };

                Checkbox( "adaptive lby", &g_Variables.m_bools[ "antiaim_adaptive_lby" ], true, adaptive_lby_cog );

                std::function<void()> twist_cog = [&]()
                    {
                        Checkbox( "disable fake-lag", &g_Variables.m_bools[ "antiaim_twist_disable_lag" ] );
                        Checkbox( "tilt yaw towards freestand", &g_Variables.m_bools[ "antiaim_twist_tilt_freestand" ] );
                        Slider( "minimum desync", &g_Variables.m_floats[ "antiaim_twist_minimum_desync" ], 10.f, 45.f );
                        Slider( "lock threshold", &g_Variables.m_floats[ "antiaim_twist_lock_threshold" ], 0.f, 30.f );
                    };

                Checkbox( "twist", &g_Variables.m_bools[ "antiaim_twist" ], true, twist_cog );

                Checkbox( "transition flick", &g_Variables.m_bools[ "antiaim_transition_flick" ] );
            } ENDCHILD
        }

        if ( iColumn == 1 )
        {
            MAKECHILD( "fake-lag" )
            {
                std::function<void()> fakelag_cog = [&]()
                    {
                        MultiCombo( "conditions", {
                            {&g_Variables.m_bools[ "fakelag_move" ], "moving" },
                            {&g_Variables.m_bools[ "fakelag_air" ], "in air" },
                            {&g_Variables.m_bools[ "fakelag_duck" ], "ducking" },
                            } );
                    };

                Checkbox( "enabled", &g_Variables.m_bools[ "fakelag_enabled" ], true, fakelag_cog );

                Slider( "lag limit", &g_Variables.m_ints[ "fakelag_limit" ], 1, g_csgo.sv_maxusrcmdprocessticks->GetInt() - 1 );

                Checkbox( "adaptive limit", &g_Variables.m_bools[ "fakelag_adaptive_limit" ] );
                Checkbox( "lag on peek", &g_Variables.m_bools[ "fakelag_on_peek" ] );
            } ENDCHILD

            MAKECHILD( "other" )
            {
                static std::function<void()> manual_cog = [&]()
                    {
                        ImGui::Text( "forward" ); ImGui::SameLine(); ImGui::Keybind( "##00009", &g_Variables.m_ints[ "antiaim_manual_forward" ], nullptr, false );
                        ImGui::Text( "left" ); ImGui::SameLine(); ImGui::Keybind( "##0000123123", &g_Variables.m_ints[ "antiaim_manual_left" ], nullptr, false );
                        ImGui::Text( "right" ); ImGui::SameLine(); ImGui::Keybind( "##0000124124124", &g_Variables.m_ints[ "antiaim_manual_right" ], nullptr, false );
                        ImGui::Text( "back" ); ImGui::SameLine(); ImGui::Keybind( "##00235235253", &g_Variables.m_ints[ "antiaim_manual_back" ], nullptr, false );

                        Checkbox( "double-press to set last-moving", &g_Variables.m_bools[ "antiaim_manual_lastmove" ] );
                        ImGui::SameLine(); Colorpicker( "##antiaim_manual_lastmove.color", &g_Variables.m_colors[ "antiaim_manual_lastmove_color" ] );

                        Checkbox( "arrows", &g_Variables.m_bools[ "antiaim_manual_arrows" ] );
                        ImGui::SameLine(); Colorpicker( "##antiaim_manual_arrows.color", &g_Variables.m_colors[ "antiaim_manual_arrows_color" ] );
                        Combo("style", &g_Variables.m_ints["antiaim_manual_arrows_style"], "default\0KibitNapHack\0");
                    };

                Combo( "leg movement", &g_Variables.m_ints[ "antiaim_leg_movement" ], "default\0always slide\0never slide\0" );
                ImGui::Text( "manual anti-aim" ); ImGui::SameLine(); StandaloneCog("aa##bruh", manual_cog );
            }
            ENDCHILD
        }
        break;
    }
}

void GRUI::VisualsTab(int iColumn) {
    switch (curr_subtab) {
    case 0: // Players
        if (iColumn == 0) {
            MAKECHILD("esp") {
                static std::function<void()> enabled_cog = [&]()
                    {
                        Checkbox( "dormant", &g_Variables.m_bools[ "esp_draw_dormant" ] );
                        Checkbox( "teammates", &g_Variables.m_bools[ "esp_draw_teammates" ] );
                        Checkbox( "wide bounding box", &g_Variables.m_bools[ "esp_wide_boxes" ] );
                        Checkbox( "override dormant color", &g_Variables.m_bools[ "esp_override_dormant" ] );
                        ImGui::SameLine(); Colorpicker("col", &g_Variables.m_colors[ "esp_dormant" ]);
                    };

                Checkbox( "enabled", &g_Variables.m_bools[ "esp_enabled" ], true, enabled_cog );

                static std::function<void()> oof_cog = [&]()
                    {
                        ImGui::Text("color");
                        ImGui::SameLine();
                        Colorpicker("##oof_arrows.color", &g_Variables.m_colors["oof_arrows_color"]);
                        Slider( "size", &g_Variables.m_floats[ "oof_arrows_size" ], 10.f, 30.f);
                        Slider( "distance", &g_Variables.m_floats[ "oof_arrows_distance" ], 10.f, 100.f );
                    };
                Checkbox("out of fov arrows", &g_Variables.m_bools["oof_arrows"], true, oof_cog);

                static std::function<void()> cog = [&]()
                { 
                    ImGui::Text("color");
                    ImGui::SameLine();
                    Colorpicker("##box_esp.color", &g_Variables.m_colors["esp_box_color"]);
                };

                Checkbox("box", &g_Variables.m_bools["esp_box"], true, cog);

                static std::function<void()> cog2 = [&]()
                { 
                    ImGui::Text("color");
                    ImGui::SameLine();
                    Colorpicker("##name_esp.color", &g_Variables.m_colors["esp_name_color"]);
                    Combo("font", &g_Variables.m_ints[ "esp_name_font" ], szFontsig );
                };

                Checkbox("name", &g_Variables.m_bools["esp_name"], true, cog2);

                static std::function<void()> cog3 = [&]()
                {
                    ImGui::Text("starting color");
                    ImGui::SameLine(); Colorpicker("##health_esp.color", &g_Variables.m_colors["esp_health_start_color"]);
                    ImGui::Text("ending color");
                    ImGui::SameLine(); Colorpicker("##health_esp.second_color", &g_Variables.m_colors["esp_health_end_color"]);
                    Slider( "background opacity", &g_Variables.m_floats[ "esp_health_bar_alpha" ], 1.f, 100.f, "%.f%%" );
                    Checkbox("draw text", &g_Variables.m_bools["esp_health_text"]);
                };

                Checkbox("health", &g_Variables.m_bools["esp_health"], true, cog3);

                static std::function<void()> cog4 = [&]()
                {
                    ImGui::Text("starting color");
                    ImGui::SameLine(); Colorpicker("##ammo_esp.color", &g_Variables.m_colors["esp_weapon_ammo_start_color"]);
                    ImGui::Text("ending color");
                    ImGui::SameLine(); Colorpicker("##ammo_esp.second_color", &g_Variables.m_colors["esp_weapon_ammo_end_color"]);
                    Checkbox("draw text", &g_Variables.m_bools["esp_weapon_ammo_text"]);
                };

                static std::function<void()> cog5 = [&]()
                {
                    Checkbox( "weapon text", &g_Variables.m_bools[ "esp_weapon_text" ] );
                    ImGui::SameLine(); Colorpicker("##esp_weapon_text.color", &g_Variables.m_colors["esp_weapon_text_color"]);
                    Checkbox( "weapon icons", &g_Variables.m_bools[ "esp_weapon_icon" ] );
                    if ( g_Variables.m_bools[ "esp_weapon_icon" ] )
                    {
                        ImGui::SameLine(); Colorpicker( "##esp_weapon_icon.color", &g_Variables.m_colors[ "esp_weapon_icon_color" ] );
                        ImGui::Text( "icon zeus color" ); ImGui::SameLine(); Colorpicker( "##esp_weapon_zeus.color", &g_Variables.m_colors[ "esp_weapon_zeus_color" ] );
                    }

                    Checkbox("ammo bar", &g_Variables.m_bools["esp_weapon_ammo"]);
                    ImGui::SameLine(); Colorpicker("##esp_weapon_ammostart.color", &g_Variables.m_colors["esp_weapon_ammo_start_color"]);
                    ImGui::SameLine(); Colorpicker("##esp_weapon_ammoend.color", &g_Variables.m_colors["esp_weapon_ammo_end_color"]);
                    if ( g_Variables.m_bools[ "esp_weapon_ammo" ] )
                        Slider("background opacity", &g_Variables.m_floats[ "esp_weapon_ammo_bar_alpha" ], 1.f, 100.f, "%.f%%");

                    Checkbox("ammo text", &g_Variables.m_bools["esp_weapon_ammo_text"]);
                };

                Checkbox("weapon", &g_Variables.m_bools["esp_weapon"], true, cog5);

                static std::function<void()> cog6 = [&]()
                    {
                        ImGui::Text( "color" );
                        ImGui::SameLine(); Colorpicker( "##lby.color", &g_Variables.m_colors[ "esp_lby_update_color" ] );
                        ImGui::Text( "unsafe color" );
                        ImGui::SameLine(); Colorpicker( "##lby_unsafe.color", &g_Variables.m_colors[ "esp_lby_update_unsafe_color" ] );
                        Slider( "background opacity", &g_Variables.m_floats[ "esp_lby_update_bar_alpha" ], 1.f, 100.f, "%.f%%" );
                    };
                Checkbox( "lby-update", &g_Variables.m_bools[ "esp_lby_update" ], true, cog6 );

                static std::function<void()> cog7 = [&]()
                    {
                        MultiCombo( "options", {
                        {&g_Variables.m_bools[ "esp_flags_money" ], "money"},
                        {&g_Variables.m_bools[ "esp_flags_armor" ], "armor"},
                        {&g_Variables.m_bools[ "esp_flags_ping" ], "ping"},
                        {&g_Variables.m_bools[ "esp_flags_defusing" ], "defusing"},
                        {&g_Variables.m_bools[ "esp_flags_c4" ], "c4"},
                        {&g_Variables.m_bools[ "esp_flags_scoped" ], "scoped"},
                        {&g_Variables.m_bools[ "esp_flags_flashed" ], "flashed"},
                        {&g_Variables.m_bools[ "esp_flags_reload" ], "reload"},
                        {&g_Variables.m_bools[ "esp_flags_exploit" ], "exploit"},
                        {&g_Variables.m_bools[ "esp_flags_lagcomp" ], "lagcomp"},
                        {&g_Variables.m_bools[ "esp_flags_resolver_confidence" ], "resolver confidence"},
                        {&g_Variables.m_bools[ "esp_flags_desync" ], "twist correction"},
                        {&g_Variables.m_bools[ "esp_flags_distortion" ], "distortion" }
                            } );

                        if ( g_Variables.m_bools[ "esp_flags_ping" ] )
                        {
                            Checkbox( "brighten ping", &g_Variables.m_bools[ "brighten_ping" ] );
                            Checkbox( "capitalize ping", &g_Variables.m_bools[ "capitalize_ping" ] );
                        }
                    };
                Checkbox("flags", &g_Variables.m_bools["esp_flags"], true, cog7);

                static std::function<void()> cog8 = [&]()
                    {
                        ImGui::Text( "color" );
                        ImGui::SameLine(); Colorpicker( "##esp_skeleton.color", &g_Variables.m_colors[ "esp_skeleton_color" ] );
                    };
                Checkbox("skeleton", &g_Variables.m_bools["esp_skeleton"], true, cog8);

            } ENDCHILD

            MAKECHILD("other") {
                static std::function<void()> WRLD_options = [&]() { // Juice WRLd
                    Checkbox("anti-aliased", &g_Variables.m_bools["hitmarker_antialias_world"]);
                    Slider("length", &g_Variables.m_ints["hitmarker_world_length"], 1, 20, "%dpx");
                    ImGui::Text("default"); ImGui::SameLine(); Colorpicker("##r43tregxfd", &g_Variables.m_colors["hitmarker_world_default"]);
                    ImGui::Text("headshot"); ImGui::SameLine(); Colorpicker("##r43243tregxfd", &g_Variables.m_colors["hitmarker_world_headshot"]);
                    ImGui::Text("lethal"); ImGui::SameLine(); Colorpicker("##r43tr2431egxfd", &g_Variables.m_colors["hitmarker_world_lethal"]);
                };

                static std::function<void()> screen_options = [&]() {
                    Checkbox("anti-aliased", &g_Variables.m_bools["hitmarker_antialias_screen"]);
                    Slider("length", &g_Variables.m_ints["hitmarker_screen_length"], 1, 20, "%dpx");

                    ImGui::Text("default"); ImGui::SameLine(); Colorpicker("##r43tregxfd", &g_Variables.m_colors["hitmarker_screen_default"]);
                    ImGui::Text("headshot"); ImGui::SameLine(); Colorpicker("##r43243tregxfd", &g_Variables.m_colors["hitmarker_screen_headshot"]);
                    ImGui::Text("lethal"); ImGui::SameLine(); Colorpicker("##r43tr2431egxfd", &g_Variables.m_colors["hitmarker_screen_lethal"]);
                };

                static std::function<void()> dmg_options = [&]() {
                    ImGui::Text("default"); ImGui::SameLine(); Colorpicker("##r43tregxfd", &g_Variables.m_colors["hitmarker_damage_default"]);
                    ImGui::Text("headshot"); ImGui::SameLine(); Colorpicker("##r43243tregxfd", &g_Variables.m_colors["hitmarker_damage_headshot"]);
                    ImGui::Text("lethal"); ImGui::SameLine(); Colorpicker("##r43tr2431egxfd", &g_Variables.m_colors["hitmarker_damage_lethal"]);
                    };

                static std::function<void()> hmarker_cog = [&]()
                    {
                        MultiCombo("hitmarkers", {
                            {&g_Variables.m_bools["hitmarker_sound"], "sound"},
                            {&g_Variables.m_bools["hitmarker_screen"], "screen"},
                            {&g_Variables.m_bools["hitmarker_world"], "world"},
                            {&g_Variables.m_bools["hitmarker_damage"], "damage"},
                        });

                        if (g_Variables.m_bools["hitmarker_screen"]) {
                            ImGui::Text("screen hitmarker"); ImGui::SameLine(); StandaloneCog("mtner342wjkgtr4wi##bruh", screen_options);
                        }

                        if (g_Variables.m_bools["hitmarker_world"]) {
                            ImGui::Text("world hitmarker"); ImGui::SameLine(); StandaloneCog("mtnerwjkgtr4wi##bruh", WRLD_options);
                        }

                        if (g_Variables.m_bools["hitmarker_damage"]) {
                            ImGui::Text("damage"); ImGui::SameLine(); StandaloneCog("543223##bruh", dmg_options);
                        }

                    };
                ImGui::Text("hitmarkers"); ImGui::SameLine(); StandaloneCog("himargb##bruh", hmarker_cog);

                static std::function<void()> localsekeleont = [&]()
                    {
                        ImGui::Text( "xyren retard 666" );
                        ImGui::Text( "shitty" );
                        ImGui::Text( "Bro" );
                        ImGui::Text( "this" );
                        ImGui::Text( "is" );
                        ImGui::Text( "the worst" );
                        ImGui::Text( "ever" );
                        ImGui::Text( "text" );
                        ImGui::Text( "text" );
                        ImGui::Text( "ladder" );
                        ImGui::Text( "text" );
                        ImGui::Text( "text" );
                        Colorpicker( "color", &g_Variables.m_colors[ "esp_local_skeleton_color" ] );
                    };
                Checkbox("local skeletrix", &g_Variables.m_bools[ "esp_local_skeleton" ], true, localsekeleont);
            } ENDCHILD
        }

        if (iColumn == 1) {
            MAKECHILD("models") {
                static std::function<void()> glow = [&]()
                    {
                        ImGui::Text( "enemy color" );
                        ImGui::SameLine(); Colorpicker( "##glow_enemy.color", &g_Variables.m_colors[ "glow_enemy_color" ] );
                        ImGui::SameLine(); Combo("##style", &g_Variables.m_ints["glow_enemy_style"], szGlowTypes);

                        ImGui::Text( "friendly color" );
                        ImGui::SameLine(); Colorpicker( "##glow_friendly.color", &g_Variables.m_colors[ "glow_friendly_color" ] );
                        ImGui::SameLine(); Combo("##style2", &g_Variables.m_ints["glow_friendly_style"], szGlowTypes);

                        ImGui::Text( "local color" );
                        ImGui::SameLine(); Colorpicker( "##glow_local.color", &g_Variables.m_colors[ "glow_local_color" ] );
                        ImGui::SameLine(); Combo("##style3", &g_Variables.m_ints["glow_local_style"], szGlowTypes);

                    };

                MultiCombo( "glow", {
                    {&g_Variables.m_bools[ "glow_enemy" ], "enemy"},
                    {&g_Variables.m_bools[ "glow_friendly" ], "friendly"},
                    {&g_Variables.m_bools[ "glow_local" ], "local" }
                    } );
                StandaloneCog("glow##bruh", glow );

                static std::function<void()> chams_enemy = [&]()
                    {
                        ImGui::Text( "base color" );
                        ImGui::SameLine(); Colorpicker( "##chams_enemy.color", &g_Variables.m_colors[ "chams_enemy_color" ] );

                        Checkbox( "occluded", &g_Variables.m_bools[ "chams_enemy_occluded" ] );
                        ImGui::SameLine(); Colorpicker( "##chams_enemy_occluded.color", &g_Variables.m_colors[ "chams_enemy_occluded_color" ] );

                        Checkbox( "history", &g_Variables.m_bools[ "chams_enemy_history" ] );
                        ImGui::SameLine(); Colorpicker( "##chams_enemy_history.color", &g_Variables.m_colors[ "chams_enemy_history_color" ] );
                        Checkbox("ragdoll", &g_Variables.m_bools["chams_enemy_ragdoll"]);
                        ImGui::SameLine(); Colorpicker("##chams_enemy_ragdoll.color", &g_Variables.m_colors["chams_enemy_ragdoll_color"]);
                        Checkbox("occluded ragdoll", &g_Variables.m_bools["chams_enemy_ragdoll_occluded"]);
                        ImGui::SameLine(); Colorpicker("##chams_enemy_ragdoll_occluded.color", &g_Variables.m_colors["chams_enemy_ragdoll_occluded_color"]);
                        Combo("material", &g_Variables.m_ints["chams_enemy_material"], "textured\0flat\0metallic\0");
                        if (g_Variables.m_ints["chams_enemy_material"] == 2) {
                            ImGui::SameLine(); Colorpicker("##chams_enemy_shine.color", &g_Variables.m_colors["chams_enemy_shine_color"]);
                            Checkbox("phong", &g_Variables.m_bools["chams_enemy_alpha_phong"]);
                            if (g_Variables.m_bools["chams_enemy_alpha_phong"]) {
                                ImGui::SameLine();
                                Slider("##phong", &g_Variables.m_ints["chams_enemy_phong_amount"], 15, 255, "%d");
                            }
                        }
                    };

                Checkbox( "enemy chams", &g_Variables.m_bools[ "chams_enemy" ], true, chams_enemy );

                static std::function<void()> chams_friendly = [&]()
                    {
                        ImGui::Text( "base color" );
                        ImGui::SameLine(); Colorpicker( "##chams_friendly.color", &g_Variables.m_colors[ "chams_friendly_color" ] );

                        Checkbox( "occluded", &g_Variables.m_bools[ "chams_friendly_occluded" ] );
                        ImGui::SameLine(); Colorpicker( "##chams_friendly_occluded.color", &g_Variables.m_colors[ "chams_friendly_occluded_color" ] );

                        Combo( "material", &g_Variables.m_ints[ "chams_friendly_material" ], "textured\0flat\0metallic\0" );

                        if (g_Variables.m_ints["chams_friendly_material"] == 2) {
                            ImGui::SameLine(); Colorpicker("##chams_friendly_shine.color", &g_Variables.m_colors["chams_friendly_shine_color"]);
                            Checkbox("phong", &g_Variables.m_bools["chams_friendly_alpha_phong"]);
                            if (g_Variables.m_bools["chams_friendly_alpha_phong"]) {
                                ImGui::SameLine();
                                Slider("##phong", &g_Variables.m_ints["chams_friendly_phong_amount"], 15, 255, "%d");
                            }
                        }
                    };

                Checkbox( "friendly chams", &g_Variables.m_bools[ "chams_friendly" ], true, chams_friendly );

                static std::function<void()> chams_local = [&]()
                    {
                        ImGui::Text( "color" );
                        ImGui::SameLine(); Colorpicker( "##chams_local.color", &g_Variables.m_colors[ "chams_local_color" ] );
                        Checkbox("override gloves", &g_Variables.m_bools[ "local_override_gloves" ] );
                        Combo( "material", &g_Variables.m_ints[ "chams_local_material" ], "textured\0flat\0metallic\0" );
                        if (g_Variables.m_ints["chams_local_material"] == 2) {
                            ImGui::SameLine(); Colorpicker("##chams_local_shine_color.color", &g_Variables.m_colors["chams_local_shine_color"]);
                            Checkbox("phong", &g_Variables.m_bools["chams_local_alpha_phong"]);
                            if (g_Variables.m_bools["chams_local_alpha_phong"]) {
                                ImGui::SameLine();
                                Slider("##phong", &g_Variables.m_ints["chams_local_phong_amount"], 15, 255, "%d");
                            }
                        }
                        ImGui::Separator();
                        MultiCombo("local blend", {
                            {&g_Variables.m_bools["local_blend_scope"], "scoped"},
                            {&g_Variables.m_bools["local_blend_grenade"], "holding grenade"},
                            {&g_Variables.m_bools["disable_chams_on_blend"], "disable chams"},
                        });

                        if (g_Variables.m_bools["local_blend_scope"] || g_Variables.m_bools["local_blend_grenade"]) {
                            Slider("##amountlocalblend", &g_Variables.m_floats["local_blend_amt"], 0.f, 100.f, "%.f%%");
                        }

                        ImGui::Separator();
                        Checkbox( "lby", &g_Variables.m_bools[ "chams_local_lby" ] );
                        ImGui::SameLine(); Colorpicker( "##chams_local_lby_color.color", &g_Variables.m_colors[ "chams_local_lby_color" ] );
                    };

                Checkbox( "local chams", &g_Variables.m_bools[ "chams_local" ], true, chams_local );

                Slider( "cham brightness", &g_Variables.m_floats[ "global_cham_tint" ], 80.f, 100.f, "%.f%%" );

            } ENDCHILD
        }
        break;
    case 1: // World
        if (iColumn == 0) {
            MAKECHILD("projectiles") {
                static std::function<void()> cog1 = [&]()
                    {
                        ImGui::Text( "color" ); 
                        ImGui::SameLine(); Colorpicker( "##local_nade_prediction.color", &g_Variables.m_colors[ "local_nade_prediction_color" ] );
                    };
                Checkbox( "local grenade prediction", &g_Variables.m_bools[ "local_nade_prediction" ], true, cog1 );

                static std::function<void()> cog2 = [&]()
                    {
                        ImGui::Text( "color" );
                        ImGui::SameLine(); Colorpicker( "##draw_grenades.color", &g_Variables.m_colors[ "draw_grenades_color" ] );
                    };
                Checkbox( "draw grenades", &g_Variables.m_bools[ "draw_grenades" ], true, cog2 );

                static std::function<void()> cog3 = [&]()
                    {
                        /*
                        m_floats[ "beam_lifetime" ] = 3.f;
	                    m_floats[ "beam_thickness" ] = 3.f;
	                    m_ints[ "beam_type" ] = 0;
                        */
                        Checkbox( "local", &g_Variables.m_bools[ "local_beams" ] );
                        ImGui::SameLine(); Colorpicker( "##local_beams.color", &g_Variables.m_colors[ "local_beam_color" ] );
                        Checkbox( "enemy", &g_Variables.m_bools[ "enemy_beams" ] );
                        ImGui::SameLine(); Colorpicker( "##enemy_beams.color", &g_Variables.m_colors[ "enemy_beam_color" ] );
                        Checkbox( "friendly", &g_Variables.m_bools[ "friendly_beams" ] );
                        ImGui::SameLine(); Colorpicker( "##friendly_beams.color", &g_Variables.m_colors[ "friendly_beam_color" ] );

                        Slider( "lifetime", &g_Variables.m_floats[ "beam_lifetime" ], 1.f, 10.f, "%.f seconds" );
                        Slider( "thickness", &g_Variables.m_floats[ "beam_thickness" ], 1.f, 10.f, "%.fu");
                        Combo("type", &g_Variables.m_ints[ "beam_type" ], "laser\0physbeam\0white\0");
                    };
                Checkbox( "bullet beams", &g_Variables.m_bools[ "bullet_beams" ], true, cog3 );

                static std::function<void()> impacts_cog = [&]()
                    {
                        g_GUI.Colorpicker("client impact color", &g_Variables.m_colors["client_impact_color"]);
                        g_GUI.Colorpicker("server impact color", &g_Variables.m_colors["server_impact_color"]);
                        g_GUI.Slider("size", &g_Variables.m_floats["impacts_size"], 1.f, 100.f, "%.f%%");
                        g_GUI.Slider("lifetime", &g_Variables.m_floats["impacts_time"], 1.f, 5.f, "%.f seconds");
                    };
                Checkbox("draw local impacts", &g_Variables.m_bools["draw_impacts"], true, impacts_cog);
            } ENDCHILD

            MAKECHILD("weapons") {
                /*
                m_bools["glow_weapons"] = false;
	m_colors["glow_weapons_color"] = Color(255, 255, 255);
                */
                static std::function<void()> cog1 = [&]()
                    {
                        Checkbox( "text", &g_Variables.m_bools[ "dropped_weapon_text" ] );
                        ImGui::SameLine(); Colorpicker( "##dropped_weapon_text.color", &g_Variables.m_colors[ "dropped_weapon_text_color" ] );
                        Checkbox( "icon", &g_Variables.m_bools[ "dropped_weapon_icon" ] );
                        ImGui::SameLine(); Colorpicker( "##dropped_weapon_icon.color", &g_Variables.m_colors[ "dropped_weapon_icon_color" ] );

                        Checkbox("glow", &g_Variables.m_bools["glow_weapons"]);
                        ImGui::SameLine(); Colorpicker("##glow_weapons_color.color", &g_Variables.m_colors["glow_weapons_color"]);
                        ImGui::SameLine(); Combo("##style", &g_Variables.m_ints["glow_weapons_style"], szGlowTypes);
                    };

                Checkbox( "dropped weapons", &g_Variables.m_bools[ "dropped_weapons" ], true, cog1 );

                static std::function<void()> cog2 = [&]()
                    {
                        Checkbox( "world", &g_Variables.m_bools[ "planted_bomb_world" ] );
                        Checkbox( "screen", &g_Variables.m_bools[ "planted_bomb_screen" ] );
                    };

                Checkbox( "planted bomb", &g_Variables.m_bools[ "planted_bomb" ], true, cog2 );
            } ENDCHILD
        }

        if (iColumn == 1) {
            MAKECHILD("modulation") {
                static std::function<void()> cog1 = [&]()
                    {
                        ImGui::Text( "world" );
                        ImGui::SameLine(); if (Colorpicker("##world.color", &g_Variables.m_colors["world_color"])) {
                            g_ctx.m_change_world = true;
                        }

                        ImGui::Text("props"); 
                        ImGui::SameLine(); if (Colorpicker("##prop.color", &g_Variables.m_colors["prop_color"])) {
                            g_ctx.m_change_world = true;
                        }

                        ImGui::Text("sky");
                        ImGui::SameLine(); if (Colorpicker("##sky.color", &g_Variables.m_colors["sky_color"])) {
                            g_ctx.m_change_world = true;
                        }

                        ImGui::Text("fire"); ImGui::SameLine();
                        Colorpicker("##fire.color", &g_Variables.m_colors["fire_color"]);
                    };

                if ( Checkbox( "world modulation", &g_Variables.m_bools[ "world_modulation" ], true, cog1 ) )
                {
                    g_ctx.m_change_world = true;
                }

                static std::function<void()> cog3 = [&]()
                    {
                        ImGui::Text( "color" );
                        ImGui::SameLine(); if (Colorpicker("##ambient_light.color", &g_Variables.m_colors["ambient_light_color"])) {
                            g_ctx.m_change_world = true;
                        }
                    };
                Checkbox( "ambient lighting", &g_Variables.m_bools[ "ambient_light" ], true, cog3 );

                Checkbox( "fullbright", &g_Variables.m_bools[ "world_fullbright" ] );

                static std::function<void()> skybox_cog = [&]()
                    {
                        g_GUI.Combo("skybox", &g_Variables.m_ints["skybox_change_name"],
                            "tibet\0embassy\0italy\0daylight\0cloudy\0night\0night 2\0night flat\0day hd\0day\0rural\0vertigo hd\0vertigo blue\0vertigo\0vietnam\0dust\0jungle\0nuke\0office\0custom\0");

                        if (g_Variables.m_ints["skybox_change_name"] == 19)
                            ImGui::InputText("##cuystom", &g_Variables.m_strings["skybox_custom"]);
                    };
                Checkbox("skybox changer", &g_Variables.m_bools["skybox_changer"], true, skybox_cog);

                static std::function<void()> fog_cog = [&]()
                {
                    if (g_GUI.Slider("fog start", &g_Variables.m_floats["fog_start"], 1.f, 2500.f)) 
                        g_ctx.m_change_world = true;
                    if (g_GUI.Slider("fog end", &g_Variables.m_floats["fog_end"], 1.f, 2500.f))
                        g_ctx.m_change_world = true;
                    if (g_GUI.Slider("fog density", &g_Variables.m_floats["fog_density"], 1.f, 100.f, "%.f%%"))
                        g_ctx.m_change_world = true;
                    if (g_GUI.Colorpicker("fog color", &g_Variables.m_colors["fog_color"]))
                        g_ctx.m_change_world = true;
                };

                if (g_Variables.m_bools["remove_fog"])
                    ImGui::BeginDisabled();

                if (Checkbox("fog modulation", &g_Variables.m_bools["fog_modulation"], true, fog_cog))
                    g_ctx.m_change_world = true;

                if (g_Variables.m_bools["remove_fog"])
                    ImGui::EndDisabled();

                MultiCombo( "removals", {
                    {&g_Variables.m_bools[ "remove_kick_recoil" ], "kick recoil" },
                    {&g_Variables.m_bools[ "remove_smooth_recoil" ], "smooth recoil" },
                    {&g_Variables.m_bools[ "remove_smoke_grenades" ], "smoke grenades" },
                    {&g_Variables.m_bools[ "remove_fog" ], "fog" },
                    {&g_Variables.m_bools[ "remove_flashbangs" ], "flashbangs" },
                    {&g_Variables.m_bools[ "remove_scope" ], "scope" },
                    {&g_Variables.m_bools[ "remove_post_processing" ], "post processing" },
                    {&g_Variables.m_bools[ "remove_scope_processing" ], "scope post processing" },
                    {&g_Variables.m_bools[ "remove_teammates" ], "teammates" },
                    {&g_Variables.m_bools[ "remove_landing_bob" ], "landing bob" },
                    {&g_Variables.m_bools["remove_landing_punch"], "landing punch" }, // m_bools[ "remove_sleeves" ]
                    {&g_Variables.m_bools["remove_sleeves"], "sleeves" },
                    } );
            } ENDCHILD

            MAKECHILD("view") {
                static std::function<void()> thirdperson_cog = [&]()
                    {
                        Slider("distance", &g_Variables.m_floats["thirdperson_distance"], 30.f, 200.f);
                        ImGui::Text("bind"); ImGui::SameLine(); ImGui::Keybind("thirdperson", &g_Variables.m_binds["thirdperson"].key, &g_Variables.m_binds["thirdperson"].mode);
                    };

                ImGui::Text("thirdperson"); ImGui::SameLine(); StandaloneCog("3p##bruh", thirdperson_cog);

                static std::function<void()> cog1 = [&]()
                    {
                        Slider( "base fov", &g_Variables.m_floats[ "view_fov" ], 10.f, 160.f, "%.f" );
                        Slider( "scope zoom", &g_Variables.m_floats[ "fov_scope_percent" ], 0.f, 100.f, "%.f%%" );
                        Slider( "viewmodel fov", &g_Variables.m_floats[ "viewmodel_fov" ], 10.f, 160.f, "%.f" );
                    };

                static std::function<void()> cog2 = [&]()
                    {
                        ImGui::Text( "color" );
                        ImGui::SameLine(); Colorpicker( "##spread_crosshair.color", &g_Variables.m_colors[ "spread_crosshair_color" ] );
                    };

                Checkbox( "fov changer", &g_Variables.m_bools[ "fov_changer" ], true, cog1 );
                Checkbox( "force viewmodel in-scope", &g_Variables.m_bools[ "viewmodel_in_scope" ] );
                Checkbox( "spread crosshair", &g_Variables.m_bools[ "spread_crosshair" ], true, cog2 );

                Checkbox( "force crosshair", &g_Variables.m_bools[ "force_crosshair" ] );

                static std::function<void()> cog3 = [&]()
                    {
                        Combo("style", &g_Variables.m_ints["penetration_crosshair_style"], "default\0cross\0rect\0outlined\0game\0");
                        g_GUI.Colorpicker("penetrable", &g_Variables.m_colors["pen_active"]);
                        g_GUI.Colorpicker("inpenetrable", &g_Variables.m_colors["pen_inactive"]);
                        g_GUI.Colorpicker("hit player", &g_Variables.m_colors["pen_player"]);

                        if ( g_Variables.m_ints[ "penetration_crosshair_style" ] == 4 )
                            Combo("while scoped", &g_Variables.m_ints[ "skip_scope_lines_pen" ], "disable scope lines\0disable crosshair");
                    };

                Checkbox( "penetration crosshair", &g_Variables.m_bools[ "penetration_crosshair" ], true, cog3 );
                Checkbox( "spectators list", &g_Variables.m_bools[ "spectator_list" ] );

                MultiCombo( "indicators", {
                    {&g_Variables.m_bools[ "lby_indicator" ], "lby" },
                    {&g_Variables.m_bools[ "lc_indicator" ], "lagcomp" },
                    {&g_Variables.m_bools[ "ping_indicator" ], "spike" },
                    {&g_Variables.m_bools[ "dmg_indicator" ], "min-damage" },
                    {&g_Variables.m_bools[ "baim_indicator" ], "force-baim" },
                    } );
            } ENDCHILD
        }

        break;
    }
}

void GRUI::MiscTab(int iColumn) {
    switch (curr_subtab) {
    case 0:
        iSelectedConfig = -1;
        if (iColumn == 0) {
            MAKECHILD("movement") {
                static std::function<void()> autostrafe_cog = [&]()
                {
                    g_GUI.Slider("strafe speed", &g_Variables.m_floats["autostrafe_speed"], 1.f, 100.f, "%.f%%");
                };

                static std::function<void()> autopeek_cog = [&]()
                    {
                        ImGui::Text("bind"); ImGui::SameLine(); ImGui::Keybind("0000112", &g_Variables.m_binds["autopeek"].key, &g_Variables.m_binds["autopeek"].mode);
                        MultiCombo("quick-switch", {
                        {&g_Variables.m_bools["quick_switch_default"], "default"},
                        {&g_Variables.m_bools["quick_switch_auto"], "auto sniper"},
                        {&g_Variables.m_bools["quick_switch_scout"], "scout"},
                        {&g_Variables.m_bools["quick_switch_awp"], "awp"},
                        {&g_Variables.m_bools["quick_switch_heavypistol"], "heavy pistol"},
                        {&g_Variables.m_bools["quick_switch_pistol"], "pistol"},
                        {&g_Variables.m_bools["quick_switch_zeus"], "lazer beam shit"}
                        });
                    };


                static std::function<void()> fakewalk_cog = [&]()
                    {
                        ImGui::Text( "bind" ); ImGui::SameLine(); ImGui::Keybind( "0000666", &g_Variables.m_binds[ "fakewalk" ].key, &g_Variables.m_binds[ "fakewalk" ].mode );
                        Slider( "max shot delay", &g_Variables.m_ints[ "fakewalk_max_delay" ], 0, 8, g_Variables.m_ints[ "fakewalk_max_delay" ] == 0 ? "disabled" : "%d ticks" );
                    };

                Checkbox( "quick-switch", &g_Variables.m_bools[ "quickswitch" ] );
                Checkbox("auto-jump", &g_Variables.m_bools["autojump"]);
                Checkbox("duck in-air", &g_Variables.m_bools["air_duck"]);
                Checkbox("auto-strafe", &g_Variables.m_bools["autostrafe"], true, autostrafe_cog);

                ImGui::Text("auto-peek");ImGui::SameLine(); StandaloneCog("AP##bruh", autopeek_cog);
                ImGui::Text( "fake-walk" ); ImGui::SameLine(); StandaloneCog( "FW##bruh", fakewalk_cog );
                ImGui::Text( "edge-jump" );ImGui::SameLine(); ImGui::Keybind( "edge-jump", &g_Variables.m_binds[ "edge_jump" ].key, &g_Variables.m_binds[ "edge_jump" ].mode );
            } ENDCHILD

            MAKECHILD("other") {
                static std::function<void()> buybot_cog = [&]()
                    {
                        g_GUI.Combo("primary", &g_Variables.m_ints["buybot_primary"], "awp\0auto-sniper\0scout\0");
                        g_GUI.Combo("secondary", &g_Variables.m_ints["buybot_secondary"], "none\0deagle / r8\0");
                        g_GUI.MultiCombo("extra", { 
                            {&g_Variables.m_bools["buybot_grenades"], "nades"}, 
                            {&g_Variables.m_bools["buybot_armor"], "armor"},
                            {&g_Variables.m_bools["buybot_taser"], "taser"},
                            {&g_Variables.m_bools["buybot_ignorepistol"], "ignore on pistol round"},
                          });
                    };

                static std::function<void()> fakelatency_cog = [&]()
                    {
                        g_GUI.Slider("default amount", &g_Variables.m_floats["fakelatency_default"], 10.f, 1000.f);
                        ImGui::Text("spike");ImGui::SameLine(); ImGui::Keybind("latency_spike", &g_Variables.m_binds["fakelatency_spike"].key, &g_Variables.m_binds["fakelatency_spike"].mode);
                        g_GUI.Slider("spike amount", &g_Variables.m_floats["fakelatency_spike"], 10.f, 1000.f);
                    };

                Checkbox("buy-bot", &g_Variables.m_bools["buybot"], true, buybot_cog);
                Checkbox("fake-latency", &g_Variables.m_bools["fakelatency"], true, fakelatency_cog);
                Checkbox("unlock inventory in-game", &g_Variables.m_bools["unlock_inventory"]);
                Checkbox("preserve killfeed", &g_Variables.m_bools["preserve_killfeed"]);   
                Checkbox("ragdoll force", &g_Variables.m_bools["ragdoll_force"]);
                MultiCombo("log events",  // XOR( "matchmaking" ), XOR( "damage" ), XOR( "harmed" ), XOR( "purchases" ), XOR( "bomb" ), XOR( "defuse" )
                    {
                        {&g_Variables.m_bools["log_damage"], "damage"},
                        {&g_Variables.m_bools["log_harmed"], "harmed"},
                        {&g_Variables.m_bools["log_purchases"], "purchases"},
                        {&g_Variables.m_bools["log_bomb"], "bomb"},
                        {&g_Variables.m_bools["log_defuse"], "defuse"},
                    });
                ImGui::Text("last-tick defuse");ImGui::SameLine(); ImGui::Keybind("last_tick", &g_Variables.m_binds["last_tick_defuse"].key, &g_Variables.m_binds["last_tick_defuse"].mode);
                
            } ENDCHILD
        }
        if (iColumn == 1) {
            MAKECHILD("identity") {
                Checkbox("clantag spammer", &g_Variables.m_bools["clantag_spammer"]);

                Combo( "spammington", &g_Variables.m_ints[ "chat_spam_type" ], "off\0normal\0" );

                Combo( "race changer", &g_Variables.m_ints[ "race_changer" ], "default\0black\0mixed\0tan\0white\0" );
                if ( ImGui::IsItemHovered() )
                {
                    ImGui::BeginTooltip();
                    ImGui::Text( "if you are racist" );
                    ImGui::EndTooltip();
                }

                if ( ImGui::Button( "brohioshitfart good bye server xddddddddd" ) )
                {
                    ImGui::OpenPopup("BroSure");
                }

                if ( ImGui::BeginPopup( "BroSure" ) )
                {
                    if ( ImGui::Button( "Are you sure Bro this shit barely works this button might as well be the OnetapLlama Rage quit button" ) )
                    {
                        bruh = true;
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }
            } ENDCHILD

            MAKECHILD("more") {
                ImGui::InputText("autoexec", &g_Variables.m_strings["autoexec"]);
                static bool onlyinohio = false;
                MultiCombo( "local model",
                    {
                        {&g_Variables.m_bools[ "remove_model_sway" ], "remove model sway"},
                        {&g_Variables.m_bools[ "force_local_footplant" ], "force foot plant"}, // ohio
                        {&g_Variables.m_bools[ "smooth_local_anims" ], "smooth animations"}
                    } );

                Checkbox("center damage logs", &g_Variables.m_bools["center_damage_logs"]);
                Checkbox("increase hit effects", &g_Variables.m_bools["increase_hit_effects"]);
            } ENDCHILD
        }

        break;
    case 1:
        PopulateConfigs();

        if (iColumn == 0) {
            for (int i = 0; i < vecConfigFiles.size(); i++) {
                auto& file = vecConfigFiles[i];
                static int iLastConfig;
                if (iSelectedConfig == i) {
                    iLastConfig = i;
                    Color col = g_Variables.m_colors["menu_accent_color"];
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(col.r() / 255.f, col.g() / 255.f, col.b() / 255.f, col.a() / 255.f));
                }

                if (Selectable(file.c_str(), iSelectedConfig == i, 0)) {
                    iLastConfig = iSelectedConfig;
                    iSelectedConfig = i;
                }

                if (iSelectedConfig == i && iLastConfig == i)
                    ImGui::PopStyleColor();
            }
        }
        if (iColumn == 1) {
            if (iSelectedConfig != -1) {
                auto& file = vecConfigFiles.at(iSelectedConfig);
                ImGui::Text("%s", file.c_str());
                ImGui::Separator();

                if (ImGui::Button("load")) {
                    ManageConfig(file.c_str(), CfgManageType_t::CFG_LOAD);
                    g_ctx.m_change_world = true;
                }

                if (ImGui::Button("save")) {
                    ImGui::OpenPopup("save_conf");
                }

                if (ImGui::BeginPopup("save_conf")) {
                    if (ImGui::Button("cancel"))
                        ImGui::CloseCurrentPopup();

                    if (ImGui::Button("confirm")) {
                        ManageConfig(file.c_str(), CfgManageType_t::CFG_SAVE);
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }

                if (ImGui::Button("create")) {
                    ImGui::OpenPopup("Create");
                }

                if (ImGui::Button("delete")) {
                    iSelectedConfig = -1;
                    ManageConfig(file.c_str(), CfgManageType_t::CFG_DELETE);
                    return;
                }
            }
            else if (vecConfigFiles.empty()) {
                ImGui::Text("No selected config.");
                ImGui::Separator();
                if (ImGui::Button("Create config"))
                    ImGui::OpenPopup("Create");
            }

            if (ImGui::BeginPopup("Create")) {
                // textbox, create config based on it.
                static char text[200];
                if (ImGui::InputTextWithHint("Config name", "Press enter to create", text, IM_ARRAYSIZE(text), ImGuiInputTextFlags_EnterReturnsTrue)) {
                    ManageConfig(text, CfgManageType_t::CFG_CREATE);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }
    }
}

bool DoesTheKnifeIndexMatchTheMenu(int loopindex) {
    switch (loopindex) {
    case KNIFE_BAYONET:
        return g_Variables.m_ints["knife_index"] == 1; 
    case KNIFE_BOWIE:
        return g_Variables.m_ints["knife_index"] == 2; 
    case KNIFE_BUTTERFLY:
        return g_Variables.m_ints["knife_index"] == 3; 
    case KNIFE_FALCHION:
        return g_Variables.m_ints["knife_index"] == 4; 
    case KNIFE_FLIP:
        return g_Variables.m_ints["knife_index"] == 5; 
    case KNIFE_GUT:
        return g_Variables.m_ints["knife_index"] == 6; 
    case KNIFE_HUNTSMAN:
        return g_Variables.m_ints["knife_index"] == 7; 
    case KNIFE_KARAMBIT:
        return g_Variables.m_ints["knife_index"] == 8; 
    case KNIFE_M9_BAYONET:
        return g_Variables.m_ints["knife_index"] == 9; 
    case KNIFE_SHADOW_DAGGERS:
        return g_Variables.m_ints["knife_index"] == 10;
    default:
        return false;
    }
    

    return false;
}

void GRUI::SkinsTab(int iColumn) {
    if (iSelectedSkinWeapon < 0 || !g_skins.m_finished )
        return;

    switch (curr_subtab) {
    case 0:
        if (iColumn == 0) {
            MAKECHILD("main") {
                static char filter[19];
                int amtintfilter = 0;
                ImGui::InputTextWithHint("##GrimaceOhio", "filter weapons", filter, 19);

                for (int i = 0; i < 19; i++) {
                    if (filter[i] == '\0')
                        break;

                    amtintfilter++;
                }

                for (int i = 0; i < g_skins.m_weapon_name_map.size(); i++) {
                    bool should_continue = false;

                    for (int j = 0; j < amtintfilter; j++) {
                        if (std::tolower(g_skins.m_weapon_name_map[i].name[j]) != std::tolower(filter[j])) {
                            should_continue = true;
                            break;
                        }
                    }

                    if (should_continue)
                        continue;

                    int pop = 0;
                    if (i > 33 && i < 48) {
                        pop++;
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 0.4));
                    }

                    if (DoesTheKnifeIndexMatchTheMenu(g_skins.m_weapon_name_map[i].definition_index)) {
                        pop++;
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 0, 1));
                    }

                    bool active = i == iSelectedSkinWeapon;
                    if (active) {
                        Color col = g_Variables.m_colors["menu_accent_color"];
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(col.r() / 255.f, col.g() / 255.f, col.b() / 255.f, col.a() / 255.f));
                        pop++;
                    }

                    if (Selectable(g_skins.m_weapon_name_map[i].name, active)) {
                        iSelectedSkinWeapon = i; 
                        // get weapon idx: g_skins.m_weapon_name_map[i].definition_index xd xd
                    }

                    if (pop > 0) {
                        ImGui::PopStyleColor(pop);
                    }
                }
            } ENDCHILD

            MAKECHILD("models") {
                Combo("knife", &g_Variables.m_ints["knife_index"],
                    "none\0bayonet\0bowie\0butterfly\0falchion\0flip\0gut\0huntsman\0karambit\0m9 bayonet\0shadow daggers");

                Combo("gloves", &g_Variables.m_ints["glove_index"], "none\0bloodhound\0sport\0driver\0wraps\0moto\0specialist\0hydra\0");
            } ENDCHILD
        }

        if (iColumn == 1) {
            MAKECHILD(g_skins.m_weapon_name_map[iSelectedSkinWeapon].name) {
                static char filter[19];
                int amtintfilter = 0;
                ImGui::InputTextWithHint("##GrimaceOhio", "filter skin ids", filter, 19);

                for (int i = 0; i < 19; i++) {
                    if (filter[i] == '\0')
                        break;

                    amtintfilter++;
                }

                if (iSelectedSkinWeapon == 48) { // glove is 48 cuz yea
                    for (int i = 0; i < g_skins.saved_gloves.size(); i++) {
                        bool should_continue = false;
                        if (g_skins.saved_gloves[i][0] == '\0')
                            continue;

                        for (int j = 0; j < amtintfilter; j++) {
                            if (std::tolower(g_skins.saved_gloves[i][j]) != std::tolower(filter[j])) {
                                should_continue = true;
                                break;
                            }
                        }

                        if (should_continue)
                            continue;

                        std::string buffer = tfm::format("%s##%i", g_skins.saved_gloves[i], i);

                        bool active = g_skins.m_weapon_name_map[iSelectedSkinWeapon].set_paintkit == i;
                        if (active) {
                            Color col = g_Variables.m_colors["menu_accent_color"];
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(col.r() / 255.f, col.g() / 255.f, col.b() / 255.f, col.a() / 255.f));
                        }

                        if (Selectable(buffer.c_str(), g_skins.m_weapon_name_map[iSelectedSkinWeapon].set_paintkit == i)) {
                            g_skins.m_weapon_name_map[iSelectedSkinWeapon].set_paintkit = i;
                        }

                        if (active)
                            ImGui::PopStyleColor();
                    }
                }
                else {
                    for (int i = 1; i < g_skins.saved_kits.size(); i++) {
                        bool should_continue = false;
                        if (g_skins.saved_kits[i].name[0] == '\0')
                            continue;

                        for (int j = 0; j < amtintfilter; j++) {
                            if (std::tolower(g_skins.saved_kits[i].name[j]) != std::tolower(filter[j])) {
                                should_continue = true;
                                break;
                            }
                        }

                        if (should_continue)
                            continue;

                        std::string buffer = tfm::format("%s##%i", g_skins.saved_kits[i].name, i);

                        bool active = g_skins.m_weapon_name_map[iSelectedSkinWeapon].set_paintkit == i;
                        if (active) {
                            Color col = g_Variables.m_colors["menu_accent_color"];
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(col.r() / 255.f, col.g() / 255.f, col.b() / 255.f, col.a() / 255.f));
                        }

                        if (Selectable(buffer.c_str(), active)) {
                            g_skins.m_weapon_name_map[iSelectedSkinWeapon].set_paintkit = i;
                            g_skins.m_update = true;
                        }

                        if (active)
                            ImGui::PopStyleColor();
                    }
                }
            } ENDCHILD

            MAKECHILD("modulation") {
                if (iSelectedSkinWeapon < 48) {
                    if (Colorpicker("color1", &g_skins.saved_kits[g_skins.m_weapon_name_map[iSelectedSkinWeapon].set_paintkit].custom_col[0]))
                        g_skins.m_update = true;
                    ImGui::SameLine(); if (Colorpicker("color2", &g_skins.saved_kits[g_skins.m_weapon_name_map[iSelectedSkinWeapon].set_paintkit].custom_col[1]))
                        g_skins.m_update = true;
                    ImGui::SameLine(); if (Colorpicker("color3", &g_skins.saved_kits[g_skins.m_weapon_name_map[iSelectedSkinWeapon].set_paintkit].custom_col[2]))
                        g_skins.m_update = true;
                    ImGui::SameLine(); if (Colorpicker("color4", &g_skins.saved_kits[g_skins.m_weapon_name_map[iSelectedSkinWeapon].set_paintkit].custom_col[3]))
                        g_skins.m_update = true;

                    if (ImGui::Button("get original skin colors")) {
                        g_skins.saved_kits[g_skins.m_weapon_name_map[iSelectedSkinWeapon].set_paintkit].custom_col[0] = g_skins.saved_kits[g_skins.m_weapon_name_map[iSelectedSkinWeapon].set_paintkit].saved_col[0];
                        g_skins.saved_kits[g_skins.m_weapon_name_map[iSelectedSkinWeapon].set_paintkit].custom_col[1] = g_skins.saved_kits[g_skins.m_weapon_name_map[iSelectedSkinWeapon].set_paintkit].saved_col[1];
                        g_skins.saved_kits[g_skins.m_weapon_name_map[iSelectedSkinWeapon].set_paintkit].custom_col[2] = g_skins.saved_kits[g_skins.m_weapon_name_map[iSelectedSkinWeapon].set_paintkit].saved_col[2];
                        g_skins.saved_kits[g_skins.m_weapon_name_map[iSelectedSkinWeapon].set_paintkit].custom_col[3] = g_skins.saved_kits[g_skins.m_weapon_name_map[iSelectedSkinWeapon].set_paintkit].saved_col[3];
                    }
                }
            } ENDCHILD
        }
        break;
    }
}

void GRUI::PlayersTab(int iColumn) {
    static int iSelectedPlayer;

    if (iColumn == 0) {
        if ( !g_csgo.m_engine->IsInGame() || !g_ctx.m_local )
            return;

        //static bool show_teammates;
        //g_GUI.Checkbox("Show teammates", &show_teammates);

        for ( int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i )
        {
            Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( i );
            if ( !player || !player->enemy( g_ctx.m_local ) )
                continue;

            player_info_t player_info;
            if ( !g_csgo.m_engine->GetPlayerInfo( i, &player_info ) )
                continue;

            AimPlayer* data = &g_aimbot.m_players[ i - 1 ];
            if ( !data )
                continue;

            if ( data->m_ignore )
                ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0, 1, 0, 1 ) );

            if ( Selectable( player_info.m_name, iSelectedPlayer == i, ImGuiSelectableFlags_AllowDoubleClick ) )
            {
                iSelectedPlayer = i;
            }

            if ( data->m_ignore )
                ImGui::PopStyleColor( 1 );
        }
    }

    if (iColumn == 1) {
        if (iSelectedPlayer > 0) {
            player_info_t player_info;
            if ( !g_csgo.m_engine->GetPlayerInfo( iSelectedPlayer, &player_info ) )
                return;

            AimPlayer* data = &g_aimbot.m_players[ iSelectedPlayer - 1 ];
            if ( !data )
                return;
        
            ImGui::Text("%s", player_info.m_name);
            ImGui::Separator();
        
            Checkbox("ignore on aimbot", &data->m_ignore);
            //Checkbox("Highlight on ESP", &player->bHighlight); ImGui::SameLine(); Colorpicker("##yep", &player->bHighlightColor);
        }
    }
}