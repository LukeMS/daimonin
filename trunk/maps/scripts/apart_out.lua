local ac = event.activator
local me = event.me

local pinfo = ac:GetPlayerInfo("APARTMENT_INFO")

if pinfo == nil then
	ac:Teleport(-1,-1, game:ReadyMap("/emergency"))
else
	ac:Teleport(pinfo.last_sp, pinfo.last_grace, game:ReadyMap(pinfo.race))
end
