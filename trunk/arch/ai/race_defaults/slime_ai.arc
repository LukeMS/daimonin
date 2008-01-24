Object slime_race_ai
name Slime race AI
race slimes
msg
# Slimes aren't supposed to be very smart
processes:
look_for_other_mobs
friendship
choose_enemy

moves: 
sleep
move_towards_enemy
move_towards_home

actions:
melee_attack_enemy
endmsg
face ai.101
sys_object 1
type 126
end
