local ac = event.activator
local me = event.me

local pinfo = ac:GetPlayerInfo("APARTMENT_INFO")

if pinfo == nil then
	ac:TeleportTo("/relic/castle/castle_030a", 18, 1, 0)
else
	ac:TeleportTo(pinfo.race, pinfo.last_sp, pinfo.last_grace, 0)
end
