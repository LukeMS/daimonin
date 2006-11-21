local ac = event.activator
local me = event.me

local pinfo = ac:GetPlayerInfo("APARTMENT_INFO")

if pinfo == nil then
	ac:SetPosition(game:ReadyMap("/emergency"), -1, -1)
else
	ac:SetPosition(game:ReadyMap(pinfo.race), pinfo.last_sp, pinfo.last_grace)
end
