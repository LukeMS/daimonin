local me = event.me
local activator = event.activator


for obj in obj_inventory(activator) do
    if obj.type == game.TYPE_WEAPON and obj.f_applied then
        me:SayTo(activator, "You are wielding a " .. obj.name)
        me:SayTo(activator, "You may not pass wielding a weapon")
        event.returnvalue = -1
        return
    end
end

me:SayTo(activator, "You may pass")