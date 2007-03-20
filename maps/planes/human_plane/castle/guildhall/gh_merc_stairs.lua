local me = event.me
local pl = event.activator
local guild_tag = "Mercenary"
local map1 = "/planes/human_plane/castle/castle_030a"
local map2 = "/planes/human_plane/castle/guildhall/merc_up"
local guild_tag = "Mercenary"
local guild_stat = game.GUILD_NO
local guild_force = nil
local my_y = me.y
local x = 0
local y = 0
guild_force = pl:GetGuild(guild_tag)
if guild_force ~= nil then
    guild_stat = guild_force.sub_type_1
end

if guild_stat == game.GUILD_IN then
    x = 1
    y = my_y -5
    pl:SetPosition(game:ReadyMap(map2), x, y, game.MFLAG_FIXED_POS)
    pl:Write("\nYou may enter.\n", 0)
    pl:Write("A magic guardian force moves you down the stairs.\n", 0)
else
    if guild_stat == game.GUILD_OLD then
        pl:Write("Only current members of the Mercenaries may enter here.\n", 0)
        pl:Write("See the Guildmaster if you wish to rejoin.\n", 0)
    else
        pl:Write("Only members of the Mercenaries may enter here.\n", 0)
        pl:Write("See the Guildmaster if you wish to join this guild.\n", 0)
    end
    x = 10
    y = my_y +1
    pl:SetPosition(game:ReadyMap(map1), x, y, game.MFLAG_FIXED_POS)
    pl:Write("A strong magic guardian force pushes you back.\n", 0)
end
event.returnvalue = 1
