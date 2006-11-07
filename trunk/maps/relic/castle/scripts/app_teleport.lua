local ac = event.activator
local me = event.me

local pinfo = ac:GetPlayerInfo("APARTMENT_INFO")

if pinfo == nil then
	me:SayTo(ac,"You don't own an apartment!")
	ac:Write("A strong force teleports you away.", 0)
	ac:Teleport(me.x+3, me.y-1,me.map)
else
    pinfo.race = "/relic/castle/castle_0101"
	pinfo.last_sp = 15
	pinfo.last_grace = 4
	ac:Teleport(pinfo.item_level, pinfo.item_quality,ac:ReadyUniqueMap(pinfo.title))
end
