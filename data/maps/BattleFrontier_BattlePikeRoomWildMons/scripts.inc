BattleFrontier_BattlePikeRoomWildMons_MapScripts:: @ 825E41A
	map_script MAP_SCRIPT_ON_RESUME, BattleFrontier_BattlePikeRoomWildMons_MapScript1_25E47A
	map_script MAP_SCRIPT_ON_FRAME_TABLE, BattleFrontier_BattlePikeRoomWildMons_MapScript2_25E42A
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, BattleFrontier_BattlePikeRoomWildMons_MapScript2_25E466
	.byte 0

BattleFrontier_BattlePikeRoomWildMons_MapScript2_25E42A: @ 825E42A
	map_script_2 VAR_TEMP_0, 0, BattleFrontier_BattlePikeRoomWildMons_EventScript_25E43C
	map_script_2 VAR_TEMP_1, 1, BattleFrontier_BattlePikeRoomWildMons_EventScript_25E44A
	.2byte 0

BattleFrontier_BattlePikeRoomWildMons_EventScript_25E43C:: @ 825E43C
	setvar VAR_TEMP_0, 1
	pike_inwildmonroom
	end

BattleFrontier_BattlePikeRoomWildMons_EventScript_25E44A:: @ 825E44A
	frontier_set FRONTIER_DATA_CHALLENGE_STATUS, CHALLENGE_STATUS_LOST
	warp MAP_BATTLE_FRONTIER_BATTLE_PIKE_LOBBY, 255, 5, 6
	waitstate
	end

BattleFrontier_BattlePikeRoomWildMons_MapScript2_25E466: @ 825E466
	map_script_2 VAR_TEMP_4, 0, BattleFrontier_BattlePikeRoomWildMons_EventScript_25E470
	.2byte 0

BattleFrontier_BattlePikeRoomWildMons_EventScript_25E470:: @ 825E470
	setvar VAR_TEMP_4, 1
	turnobject EVENT_OBJ_ID_PLAYER, DIR_NORTH
	end

BattleFrontier_BattlePikeRoomWildMons_MapScript1_25E47A: @ 825E47A
	call BattleFrontier_BattlePikeRoom_EventScript_ResetSketchedMoves
	frontier_get FRONTIER_DATA_BATTLE_OUTCOME
	compare VAR_RESULT, B_OUTCOME_LOST
	goto_if_eq BattleFrontier_BattlePikeRoomWildMons_EventScript_25E4A3
	compare VAR_RESULT, B_OUTCOME_DREW
	goto_if_eq BattleFrontier_BattlePikeRoomWildMons_EventScript_25E4A3
	end

BattleFrontier_BattlePikeRoomWildMons_EventScript_25E4A3:: @ 825E4A3
	setvar VAR_TEMP_1, 1
	end

