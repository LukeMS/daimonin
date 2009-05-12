local player = event.activator

local map1 = player.map
local map2 = player.map

if map1 == map2 then
    player:Write("map1 == map2 :)", game.COLOR_GREEN)
else
    player:Write("map1 != map2 :(", game.COLOR_RED)
end

map2 = game:ReadyMap(player.map.path, game.MAP_NEW)

if player.map == map4 then
    player:Write("player.map == map2 :(", game.COLOR_GREEN)
else
    player:Write("player.map != map2 :)", game.COLOR_RED)
end

if map1 == map2 then
    player:Write("map1 == map2 :(", game.COLOR_GREEN)
else
    player:Write("map1 != map2 :(", game.COLOR_RED)
end

player:Write("This is a bug!")
