/*
	THIS FILE IS A PART OF RDR 2 SCRIPT HOOK SDK
				http://dev-c.com
			(C) Alexander Blade 2019
*/

#include "script.h"
#include "keyboard.h"
#include "hashtostr.h"
#include <vector>
#include <fstream>
#include <string>
#include <set>


// debug text ingame
void DrawText(float x, float y, char* str)
{
	UI::DRAW_TEXT(GAMEPLAY::CREATE_STRING(10, "LITERAL_STRING", str), x, y);
}
void npcstat(Ped ped, const char* stat[])
{
	float y = 0;
	for (int i = 0; i < sizeof(stat); i++)
	{
		DrawText(0.45, y, (char*)stat[i]);
		y += 0.11;
	}
}

std::ofstream logfile("LanternOverhaul.log"); // log file

// variable
bool is_npcstat = false; // single npc debug
int is_night = 2; // 0 - DAY, 1 - PARTIALLY DARK, 2 - NIGHT
std::set<Hash> lanterngiven = {}; // to avoide unlimited lantern spawn when disarmed
bool find(Hash hash, std::set<Hash> Set)
{
	auto val = Set.find(hash);
	if (val != Set.end())
	{
		return true;
	}
	return false;
}


void main()
{
	while (true)
	{
		// get ingame time (npcs use lantern from 22:00 to 5:00)
		int time = CLOCK::GET_CLOCK_HOURS();
		if (time >= 20 && time <= 4)
		{
			is_night = 2;
		}
		else if (time == 19 || time == 5)
		{
			is_night = 1;
		}
		else if (time >= 6 && time <= 18)
		{
			is_night = 0;
		}
		
		// does not execute the below codes. 
		if (is_night == 0)
		{
			WAIT(0);
			continue;
		}


		// player id and player ped
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		// PLAYER::SET_PLAYER_INVINCIBLE(player, true); // infinite core - testing purpose
		
		// to get nearby npcs
		const int Arr_size = 150;
		Ped nearbyPeds[Arr_size];
		int radius = 600; // mod affect radius
		int numPeds = worldGetAllPeds(nearbyPeds, radius); // pool resets to 0 in town

		// weapon hash
		Hash lantern1 = 0x4A59E501;
		Hash lantern2 = 0xF62FB3A3;
		Hash unarmed = 0xA2719263;
		Hash curweapon = NULL;

		// debug
		int peds_in_rad = 0;

		// main iteration
		for (int i = 0; (i < numPeds); i++)
		{
			Ped ped = nearbyPeds[i];

			// only few of the npc use lantern in partial dark time.
			if (is_night == 1 && (rand() % 2))
			{
				continue;
			}

			// go to next ped if current ped is not human or is playerped
			if (!PED::IS_PED_HUMAN(ped) || ped == playerPed)
			{
				continue;
			}
			if (ENTITY::IS_ENTITY_DEAD(ped))
			{
				lanterngiven.erase(ENTITY::GET_ENTITY_MODEL(ped));
				continue;
			}

			// distance between player and npc
			Vector3 playercoo = ENTITY::GET_ENTITY_COORDS(playerPed, true, true);
			Vector3 npccoo = ENTITY::GET_ENTITY_COORDS(ped, true, true);
			float distance = BUILTIN::VDIST(playercoo.x, playercoo.y, playercoo.z, npccoo.x, npccoo.y, npccoo.z);

			// go to next ped if ped is not within radius
			if (distance > radius)
			{
				continue;
			}
			
			// debug
			peds_in_rad++;
			logfile << "\nPed: " << dehash(ENTITY::GET_ENTITY_MODEL(ped), pedsHashtoStr) << "\n";
			logfile << "Gender: " << PED::IS_PED_MALE(ped) << "\n";
			logfile << "Distance: " << distance << "\n";
			/*std::string coo = (std::to_string(npccoo.x / 1000) + " " + std::to_string(npccoo.y / 1000) + " " + std::to_string(npccoo.z / 1000));
			DrawText(npccoo.x/1000, npccoo.y/1000, (char*)coo.c_str());*/

			// 
			int attach_point = 0;
			if ((PED::IS_PED_USING_ANY_SCENARIO(ped) && TASK::IS_PED_WALKING(ped)) || (PED::IS_PED_USING_ANY_SCENARIO(ped) && TASK::IS_PED_RUNNING(ped)))
			{
				attach_point = 1;
			}
			else if (PED::IS_PED_USING_ANY_SCENARIO(ped) || TASK::IS_PED_SPRINTING(ped))
			{
				attach_point = 12;
			}
			else if (PED::GET_SEAT_PED_IS_USING(ped) == -2 || PED::IS_PED_GOING_INTO_COVER(ped) || PED::IS_PED_IN_COVER(ped, false, false) || PED::GET_PED_STEALTH_MOVEMENT(ped) || PED::IS_PED_IN_MELEE_COMBAT(ped))
			{
				attach_point = -1;
			}
			else if (TASK::IS_PED_STILL(ped) || PED::IS_PED_FULLY_ON_MOUNT(ped, true) || PED::GET_SEAT_PED_IS_USING(ped) == -1 || TASK::IS_PED_WALKING(ped) || TASK::IS_PED_RUNNING(ped))
			{
				attach_point = 0;
			}

			// check if ped is being unarmed or hogotied or lassoed
			
			// if npc doesn't have a lantern, give one
			logfile << "Ped's attach point: " << attach_point << "\n";
			bool haslantern = WEAPON::HAS_PED_GOT_WEAPON(ped, lantern1, 0, false) || WEAPON::HAS_PED_GOT_WEAPON(ped, lantern2, 0, false);
			if (haslantern == false && find(ENTITY::GET_ENTITY_MODEL(ped), lanterngiven) == false)
			{
				logfile << "Giving lantern" << "\n";
				if ((rand() % 2) == 0)
				{
					WEAPON::GIVE_WEAPON_TO_PED(ped, lantern1, 0, true, true, -1, false, 0.5, 1.0, 0x2CD419DC, true, 0.5, NULL);
					WEAPON::SET_CURRENT_PED_WEAPON(ped, lantern1, true, attach_point, NULL, NULL);
				}
				else
				{
					WEAPON::GIVE_WEAPON_TO_PED(ped, lantern2, 0, true, true, -1, false, 0.5, 1.0, 0x2CD419DC, true, 0.5, NULL);
					WEAPON::SET_CURRENT_PED_WEAPON(ped, lantern2, true, attach_point, NULL, NULL);
				}

				haslantern = WEAPON::HAS_PED_GOT_WEAPON(ped, lantern1, 0, false) || WEAPON::HAS_PED_GOT_WEAPON(ped, lantern2, 0, false);
				logfile << "Ped has lantern now: " << haslantern << "\n";
				lanterngiven.insert(ENTITY::GET_ENTITY_MODEL(ped));
			}

			// force ped to use lantern (ped keeps lantern inside while entering a anim)
			haslantern = WEAPON::HAS_PED_GOT_WEAPON(ped, lantern1, 0, false) || WEAPON::HAS_PED_GOT_WEAPON(ped, lantern2, 0, false);
			logfile << "Ped has lantern: " << haslantern << "\n";
			WEAPON::GET_CURRENT_PED_WEAPON(ped, &curweapon, NULL, attach_point, NULL);
			if (curweapon == unarmed && haslantern)
			{
				logfile << "setting current weapon as lantern" << "\n";
				if (WEAPON::HAS_PED_GOT_WEAPON(ped, lantern1, 0, false))
				{
					WEAPON::SET_CURRENT_PED_WEAPON(ped, lantern1, true, attach_point, NULL, NULL);
				}
				else
				{
					WEAPON::SET_CURRENT_PED_WEAPON(ped, lantern2, true, attach_point, NULL, NULL);
				}
			}

			// debug npc behaviour
			WEAPON::GET_CURRENT_PED_WEAPON(ped, &curweapon, NULL, attach_point, NULL);
			/*if (IsKeyJustUp(0x54) || is_npcstat)
			{
				if (is_npcstat)
				{
					is_npcstat = false;
				}
				else
				{
					is_npcstat = true;
				}

				Entity* playerAimon = NULL;
				PLAYER::GET_PLAYER_TARGET_ENTITY(player, playerAimon);
				if (playerAimon != NULL)
				{
					Ped playerAimonped = (Entity)(playerAimon);
					if (PED::IS_PED_HUMAN(playerAimonped))
					{
						logfile << "single npc stat" << "\n";
						is_npcstat = true;
						const char* stat[6];
						stat[0] = "Is ped still: "+(char)TASK::IS_PED_STILL(ped);
						stat[1] = "Is ped walking: "+(char)TASK::IS_PED_WALKING(ped);
						stat[2] = "Is ped running: "+(char)TASK::IS_PED_RUNNING(ped);
						stat[3] = "Is ped in scenario: "+(char)PED::IS_PED_USING_ANY_SCENARIO(ped);
						stat[4] = "Is ped fully mounted: "+(char)PED::IS_PED_FULLY_ON_MOUNT(ped, true);
						stat[5] = "Where is ped seating: "+(char)PED::GET_SEAT_PED_IS_USING(ped);
						npcstat(playerAimonped, stat);
					}
				}
			}*/
			logfile << "Is ped using Lantern: " << dehash(curweapon, weaponhashtostr) << "\n";
			logfile << "Is ped still: " << TASK::IS_PED_STILL(ped) << "\n";
			logfile << "Is ped walking: " << TASK::IS_PED_WALKING(ped) << "\n";
			logfile << "Is ped running: " << TASK::IS_PED_RUNNING(ped) << "\n";
			logfile << "Is ped in scenario: " << PED::IS_PED_USING_ANY_SCENARIO(ped) << "\n";
			logfile << "Is ped in combat: " << PED::IS_PED_IN_MELEE_COMBAT(ped) << "\n";
			logfile << "Is ped fully mounted: " << PED::IS_PED_FULLY_ON_MOUNT(ped, true) << "\n";
			logfile << "Is ped been lassoed: " << PED::IS_PED_LASSOED(ped) << "\n";
			logfile << "Is ped been hogtied: " << PED::IS_PED_HOGTIED(ped) << "\n";
			logfile << "Where is ped seating: " << (int)PED::GET_SEAT_PED_IS_USING(ped) << "\n";
		}

		//std::string coo = (std::to_string(playercoo.x/1000)+" "+std::to_string(playercoo.y/1000)+" "+std::to_string(playercoo.z/1000));
		//DrawText(0.49, 0.48, (char*)"x");

		logfile << "\nTotal peds: " << numPeds << " Peds in rad: " << peds_in_rad << "\n";
		WAIT(0);

	}
}

void ScriptMain()
{
	srand(GetTickCount());
	main();
}
