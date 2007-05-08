local ac = event.activator
local me = event.me

local pinfo = ac:GetPlayerInfo("APARTMENT_INFO")
local path
local loc
-- 7 may 2007 added yield as work around for intermittent apartment access and exit - tehill
event.returnvalue = 1
coroutine.yield(.03)
loc = game:FindPlayer(ac.name)
if me.x == loc.x and me.y == loc.y then

    if pinfo == nil then
	   ac:SetPosition(game:ReadyMap("/emergency"), -1, -1)
    else
        -- convert exit map path if pre-reorganization:
        -- change /lost_worlds to /planes/demon_plane
        -- change /relic to /planes/human_plane
        path = pinfo.race
        if string.sub(path, 1, 12) == "/lost_worlds" then
            pinfo.race = "/planes/demon_plane"..string.sub(path, 13)
        elseif string.sub(path, 1, 6) == "/relic" then
            pinfo.race = "/planes/human_plane"..string.sub(path, 7)
        end

	   ac:SetPosition(game:ReadyMap(pinfo.race), pinfo.last_sp, pinfo.last_grace)
    end
end
event.returnvalue = 1