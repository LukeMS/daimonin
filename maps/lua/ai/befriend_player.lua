-- inventory iterator
function find_teddy(where)
    for obj in obj_inventory(where) do
        if obj.name == 'teddy' then return obj end
    end
end

-- We store the name of our friend in the teddy.
local teddy = find_teddy(event.me)
if teddy and teddy.slaying == '' then
    local ai = event.me:GetAI()
    local knowns = ai:GetKnownMobs()

    -- See if we can see a player
    for i,obj in pairs(knowns) do
        if obj.type == game.TYPE_PLAYER then
            teddy.slaying = obj.name
            ai:Register(obj, 1000) -- We really like this player
            event.me:SayTo(obj, "My bestest friend!")
            break
        end
    end
end
