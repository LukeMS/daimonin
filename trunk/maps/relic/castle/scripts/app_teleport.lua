local ac = event.activator
local me = event.me

local pinfo = ac:GetPlayerInfo("APARTMENT_INFO")

if pinfo == nil then
	me:SayTo(ac,"You don't own an apartment!")
	ac:Write("A strong force teleports you away.", 0)
	ac:SetPosition(17, 4)
else
    pinfo.race = "/relic/castle/castle_0101"
	pinfo.last_sp = 15
	pinfo.last_grace = 4
	ac:TeleportTo(pinfo.title, pinfo.item_level, pinfo.item_quality, 1)
end
