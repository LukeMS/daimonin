Object ai_horse
name Horse AI
race horse
msg
# This is an AI for horses, with a twist (or two)
processes:
look_for_other_mobs
look_for_objects
friendship
choose_enemy
attraction arch=apple01.101:100 arch=carrot01.101:100
# Food is type 6

moves: 
sleep
move_towards_enemy
move_towards_enemy_last_known_pos
avoid_repulsive_items
search_for_lost_enemy
move_towards_waypoint
investigate_attraction
move_towards_home

actions:
melee_attack_enemy
endmsg
face ai.101
sys_object 1
type 126
end
