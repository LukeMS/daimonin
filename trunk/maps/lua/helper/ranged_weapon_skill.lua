-------------------------------------------------------------------------------
-- ranged_weapon_skill.lua | Helper
--
-- Figure out which weapon skill the player has.
-------------------------------------------------------------------------------
---------------------------------------
--All Helpers require a script context, so ensure the Utility is loaded.
---------------------------------------
require("script_context")

---------------------------------------
-- Helpers define 1 function, nothing else (the function name is irrelevant).
---------------------------------------
local function f(player)
    assert(type(player) == "GameObject" and player.type == game.TYPE_PLAYER,
        "Arg #1 must be player GameObject!")
    local sname = { "Bow", "Crossbow", "Sling" }
    for _, v in ipairs(sname) do
        local obj, mode = player:GetSkill(v .. " Archery")
        if obj ~= nil then
            return obj, mode
        end
    end
end

---------------------------------------
-- Add the function to the script context under the key of the Helper script
-- name.
---------------------------------------
ScriptContext:AddFunction("ranged_weapon_skill", f)
