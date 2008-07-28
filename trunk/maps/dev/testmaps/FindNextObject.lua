local fno = event.me
local player = event.activator
local next = player:FindMarkedObject()
if next == nil then
    next = player
end

next = next:FindNextObject(game.TYPE_CONTAINER)
player:Write(tostring(next), game.COLOR_RED)
