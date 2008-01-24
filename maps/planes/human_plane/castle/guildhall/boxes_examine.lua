local me        = event.me
local activator = event.activator

-- Only print the special examine messages if the player has been given the quest.
-- IOW he can't find what he doesn't know he is looking for.
-- However, he can always apply the boxes to enter the dungeon, regardless of if he has accepted the quest or not.
if activator:GetQuest("Rusty Rod Retrieval") then
    event.returnvalue = 1
    local msg = "The " .. me.name .. " seems to be a perfectly normal " .. me.name .. "."
    if me.type == game.TYPE_EXIT then
        msg = "Aha! The dirt around the " .. me.name .. " shows signs of recent activity and... " ..
              "yes, they have been arranged to conceal a small hole in the ground. " ..
              "Hm, looks like a squeeze but if you hold your breath you could wriggle down there."
    end
    activator:Write(msg .. "\n")
end
