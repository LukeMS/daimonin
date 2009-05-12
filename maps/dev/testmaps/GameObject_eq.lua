local player = event.activator

local obj1 = game:LoadObject("arch goldcoin\nend")
local obj2 = game:LoadObject("arch goldcoin\nend")

if obj1 == obj2 then
    player:Write("obj1 == obj2 :(", game.COLOR_GREEN)
else
    player:Write("obj1 != obj2 :)", game.COLOR_RED)
end

if obj1 == obj1 then
    player:Write("obj1 == obj1 :)", game.COLOR_GREEN)
else
    player:Write("obj1 != obj1 :(", game.COLOR_RED)
end
