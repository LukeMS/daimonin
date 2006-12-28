-- crashes server when activator is moved off pedestal
-- connect this script to a pedestal (event type Trigger)

local act = event.activator

ret = act:Move(game.SOUTH)

print("Move() returned " .. tostring(ret))