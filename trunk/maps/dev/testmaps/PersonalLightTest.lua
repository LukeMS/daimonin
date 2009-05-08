-------------------------------------------------------------------------------
-- PersonalLightTest.lua
--
-- Toggles activator's (who we assume is a player) personal light off and on.
-- When it is switched on,, it will be a random value between 1 and
-- MAX_DARKNESS).
-------------------------------------------------------------------------------
local player = event.activator

local value = game.PERSONAL_LIGHT_OFF
local from = player:GetPersonalLight()

if from == game.PERSONAL_LIGHT_OFF then
    value = math.random(game.PERSONAL_LIGHT_MIN, game.PERSONAL_LIGHT_MAX)
end

local to = player:SetPersonalLight(value)

player:Write(player:GetName() .. "'s personal light from " .. tostring(from) ..
             " to " .. tostring(to))
