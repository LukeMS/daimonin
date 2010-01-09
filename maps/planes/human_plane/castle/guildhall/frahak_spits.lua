-------------------------------------------------------------------------------
-- frahak_spits.lua -- say on Frah'ak at castle_030a 4 7
-- 
-- A script to make Frah'ak spit at anyone who says anything near him.
-------------------------------------------------------------------------------
local frahak = event.me
local activator = event.activator

if math.random() >= 0.7 then
    frahak:Communicate("/spit " .. activator.name)
end
