local player = event.activator

local map1 = player.map
local map2 = player.map

if map1 == map2 then
    player:Write("map1 == map2 :)", game.COLOR_GREEN)
else
    player:Write("map1 != map2 :(", game.COLOR_RED)
end

local map3 = player.map
local map4 = game:ReadyMap(player.map.path, game.MAP_NEW)

if map3 == map4 then
    player:Write("map3 == map4 :)", game.COLOR_GREEN)
else
    player:Write("map3 != map4 :(", game.COLOR_RED)
end

if player.map == map4 then
    player:Write("player.map == map4 :)", game.COLOR_GREEN)
else
    player:Write("player.map != map4 :(", game.COLOR_RED)
end
