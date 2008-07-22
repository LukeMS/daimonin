-- Script to test AI object registration triggering

local me = event.me
local other = event.activator

me:Say("I saw " .. other.name .. ". friendship=" .. event.parameter1 .. ", attraction=" .. event.parameter2) 