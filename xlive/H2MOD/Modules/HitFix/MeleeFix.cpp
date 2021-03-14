#include "MeleeFix.h"
#include "H2MOD/Modules/Config/Config.h"
#include "Util/Hooks/Hook.h"
#include "Blam/Engine/Game/GameTimeGlobals.h"
#include "Blam/Enums/Game/HaloStrings.h"
#include "H2MOD/Modules/Startup/Startup.h"

using Blam::Enums::Game::HaloString;

namespace MeleeFix
{
	void MeleePatch(bool toggle)
	{
		WriteValue<BYTE>(h2mod->GetAddress(0x10B408, 0xFDA38) + 2, toggle ? 5 : 6); // sword
		WriteValue<BYTE>(h2mod->GetAddress(0x10B40B, 0xFDA3B) + 2, toggle ? 2 : 1); // generic weapon
	}

	void MeleeCollisionPatch()
	{
		if (!h2mod->Server) {
			/*
				.text:007C3027 148 E8 C1 73 F8 FF       call    collision_test_vector ; Call Procedure
				.text:007C302C 148 83 C4 18             add     esp, 18h        ; Add
				.text:007C302F 130 84 C0                test    al, al          ; Logical Compare
				.text:007C3031 130 0F 84 4B 01 00 00    jz      loc_7C3182      <=== Remove this jump
			 */
			static byte original_melee_collision_instruction[]{ 0x0F, 0x84, 0x4B, 0x01, 0x00, 0x00 };
			if (H2Config_melee_fix)
			{
				NopFill(h2mod->GetAddress(0x143031, 0), 6);
			}
			else
			{
				WriteBytes(h2mod->GetAddress(0x143031, 0), original_melee_collision_instruction, 6);
			}
		}
	}
	//////////////////////////////////////////////////////
	////////////////////Experimental//////////////////////
	//////////////////////////////////////////////////////

	typedef int(__cdecl p_melee_get_time_to_target)(unsigned __int16 object_index);
	p_melee_get_time_to_target* melee_get_time_to_target;

	typedef void(__cdecl p_melee_damage)(int object_index, signed int melee_type, char unk2, float unk3);
	p_melee_damage* melee_damage;

	typedef void (__cdecl p_send_melee_damage_simulation_event)(int a1, int a2, int arg8);
	p_send_melee_damage_simulation_event* c_send_melee_damage_simulation_event;

	typedef void (__cdecl p_melee_environment_damage)(int a1, int arg4, int arg8);
	p_melee_environment_damage* c_melee_environment_damage;

	typedef void (__cdecl p_sub_88B54F)(int a2, int a3);
	p_sub_88B54F* c_sub_88B54F;
	bool MeleeHit = false;

	void __cdecl melee_environment_damage(int a1, int arg4, int arg8)
	{
		MeleeHit = true;
		c_melee_environment_damage(a1, arg4, arg8);
		//LOG_INFO_GAME("[MeleeFix] Environment Hit {} {} {}", a1, arg4, arg8);
	}

	void __cdecl send_melee_damage_simulation_event(int a1, int a2, int arg8)
	{
		MeleeHit = true;
		c_send_melee_damage_simulation_event(a1, a2, arg8);
		//LOG_INFO_GAME("[MeleeFix] Packet Send {} {} {}", a1, a2, arg8);
	}

	void __cdecl sub_88B54F(int a2, int a3)
	{
		MeleeHit = true;
		c_sub_88B54F(a2, a3);
		//LOG_INFO_GAME("[MeleeFix] unk {} {}", a2, a3);
	}
	

	int sub877948Hook(int a1, int a2, int target_player, char a4)
	{
		int result;
		__asm
		{
			push a4
			push target_player
			push a2
			mov eax, a1
			mov edx, [0x167948]
			add edx, [H2BaseAddr]
			call edx
			add esp, 12
			mov result, eax
		}
		return result;
	}

	void __cdecl simulation_melee_action_update_animation(int object_index)
	{
		s_datum_array* objects = *h2mod->GetAddress<s_datum_array**>(0x4E461C);

		int biped_object = *(DWORD *)&objects->datum[12 * (unsigned __int16)object_index + 8];
		int melee_info_offset = *(__int16 *)(biped_object + 858); //???
		auto biped_melee_info = (melee_info *)(biped_object + melee_info_offset);
		int melee_type = biped_melee_info->melee_type_string_id;

		if (biped_melee_info->melee_animation_update == biped_melee_info->max_animation_range || biped_melee_info->melee_animation_update == 0)
			MeleeHit = false;

		if (melee_type == -1)
		{
			biped_melee_info->melee_flags &= 0xF7FFFFFF;
			biped_melee_info->melee_type_string_id = -1;
			MeleeHit = false;
		}
		else
		{
			bool abort_melee_action = false;
			if (!MeleeHit) 
			{
				float currentFrame = (float)biped_melee_info->melee_animation_update;
				float actionFrame = (float)biped_melee_info->animation_action_index;
				float tolerance = 0.1f;
				float leeway = (float)biped_melee_info->max_animation_range * tolerance / 2;
				LOG_INFO_GAME("[MeleeFix] Frame Data: Current Frame {} Action Frame {} Leeway {}", currentFrame, actionFrame, leeway);
				//Add more opportunity frames to a melee to hit
				if (currentFrame >= actionFrame - leeway && currentFrame <= actionFrame + leeway)
				{
					melee_damage(object_index, melee_type, biped_melee_info->field_30, (float)(unsigned __int8)biped_melee_info->field_31 * 0.0039215689);
					if (MeleeHit) 
					{
						LOG_INFO_GAME("[MeleeFix] Melee Hit!");
					}
					else 
					{
						LOG_INFO_GAME("[MeleeFix] Melee Missed!");
					}
				}
				
			}

			/*if (biped_melee_info->melee_animation_update == biped_melee_info->animation_action_index) 
			{
				melee_damage(object_index, melee_type, biped_melee_info->field_30, (float)(unsigned __int8)biped_melee_info->field_31 * 0.0039215689);
			}*/
			
			if (melee_type == HaloString::HS_MELEE_DASH || melee_type == HaloString::HS_MELEE_DASH_AIRBORNE)
			{
				float melee_max_duration = melee_type == HaloString::HS_MELEE_DASH_AIRBORNE ? 0.22 : 0.15000001;
				signed int melee_max_ticks = time_globals::seconds_to_ticks_impercise(melee_max_duration);
				if (melee_max_ticks < 0 || melee_get_time_to_target(object_index) <= melee_max_ticks)
					abort_melee_action = true;
			}
			if ((++biped_melee_info->melee_animation_update >= (int)biped_melee_info->max_animation_range || abort_melee_action)
				&& !sub877948Hook(object_index, 0, -1, biped_melee_info->field_30))
			{
				biped_melee_info->melee_flags &= 0xF7FFFFFF;
				biped_melee_info->melee_type_string_id = -1;
				MeleeHit = false;
			}
		}
	}

	void ApplyHooks()
	{
		melee_get_time_to_target = h2mod->GetAddress<p_melee_get_time_to_target*>(0x150784);
		melee_damage = h2mod->GetAddress<p_melee_damage*>(0x142D62);
		c_send_melee_damage_simulation_event = h2mod->GetAddress<p_send_melee_damage_simulation_event*>(0x1B8618);
		c_melee_environment_damage = h2mod->GetAddress<p_melee_environment_damage*>(0x13F26D);
		c_sub_88B54F = h2mod->GetAddress<p_sub_88B54F*>(0x17B54F);
		PatchCall(h2mod->GetAddress(0x143440), send_melee_damage_simulation_event);
		PatchCall(h2mod->GetAddress(0x14345A), melee_environment_damage);
		PatchCall(h2mod->GetAddress(0x143554), sub_88B54F);
		WritePointer(h2mod->GetAddress(0x41E524), (void*)simulation_melee_action_update_animation);
	}

	void Initialize()
	{
		//ApplyHooks();
		MeleeCollisionPatch();
	}
}
