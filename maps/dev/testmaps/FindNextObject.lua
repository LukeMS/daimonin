local fno = event.me
local player = event.activator
local nextobj = player:FindMarkedObject()
local message

if nextobj == nil then
    nextobj = player
end

nextobj = nextobj:FindNextObject(game.TYPE_CONTAINER, game.FNO_MODE_CONTAINERS)
player:Write("Next object is " .. tostring(nextobj) .. "!", game.COLOR_RED)
