local player = event.activator

local map1 = player.map
local map2 = player.map

if map1 == map2 then
    player:Write("map1 == map2! :)")
else
    player:Write("map1 != map2! :(")
end

local map3 = player.map
local map4 = game:ReadyMap(player.map.path, game.MAP_NEW)

if map3 == map4 then
    player:Write("map3 == map4! :)")
else
    player:Write("map3 != map4! :(")
end

