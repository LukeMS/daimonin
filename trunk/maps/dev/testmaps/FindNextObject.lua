local fno = event.me
local player = event.activator
local nextobj = player:FindMarkedObject()
local message

if nextobj == nil then
    nextobj = player
end

nextobj = nextobj:FindNextObject(game.TYPE_CONTAINER, nil, nil, nil, game.FNO_MODE_CONTAINERS)
if nextobj then
    message = nextobj.name .. "[" .. nextobj.count .. "]"
else
    message = "nil"
end
player:Write("Next object is " .. message .. "!", game.COLOR_RED)
