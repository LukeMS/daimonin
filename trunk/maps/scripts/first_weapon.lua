-------------------------------------------------------------------------------
-- first_weapon.lua | helper script to give player a first weapon
--
-- Create and auto-equip a weapon of the appropriate type if player has not
-- already been given one by this script, has 0 exp, and has no weapon
-- equipped. So this is not foolproof, but should be good enough given the low
-- value of this starter weapon.
-------------------------------------------------------------------------------
local npc = event.me
local player = event.activator
local ds = DataStore("first_weapon", player)

if not ds:Get("weapon given") then
    ds:Set("weapon given", true)
    ds:WasChanged()

    local weaponarch =
    {
        [game:GetSkillNr("slash weapons") ] = "shortsword",
        [game:GetSkillNr("impact weapons")] = "mstar_small",
        [game:GetSkillNr("cleave weapons")] = "axe_small",
        [game:GetSkillNr("pierce weapons")] = "dagger_large"
    }

    for k, v in pairs(weaponarch) do
        local skill = player:FindSkill(k) 

        if skill and
           skill.experience == 0 and
           not player:GetEquipment(game.EQUIP_WEAPON1) then
            local weapon = player:CreateObjectInside(v, game.IDENTIFIED)

            assert(weapon, "Could not create weapon!")

            if npc.type == game.TYPE_MONSTER then
                player:Write(npc.name .. " puts a " .. weapon:GetName() ..
                             " in your hands.")
                player:Write(string.capitalize(gender_objective[npc:GetGender()]) ..
                             " says: Here, take this weapon.")
            end

            player:Apply(weapon, game.APPLY_ALWAYS)

            break
        end
    end
end
