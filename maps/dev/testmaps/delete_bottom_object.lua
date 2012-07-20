local ac = event.activator

if ac:GetGmasterMode() ~= game.GMASTER_MODE_SA then

    return

end

for obj in map_objects(ac.map, ac.x, ac.y) do

    if obj.type ~= game.TYPE_PLAYER then

        ac:Write('Slurp! The ' .. obj:GetName() .. ' was removed!')
        obj:Remove()
        event.returnvalue = 1
        return

    end

end

event.returnvalue = 1