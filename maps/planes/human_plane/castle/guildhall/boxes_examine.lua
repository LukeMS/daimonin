-------------------------------------------------------------------------------
-- boxes_examine.lua | EXAMINE on big box at castle_030a 18 14
--                   | EXAMINE on box at castle_030a 18 13
--                   | EXAMINE on box at castle_030a 20 13
--                   | EXAMINE on box at castle_030a 21 13
--                   | EXAMINE on boxes at castle_030a 18 9
--                   | EXAMINE on straw bale at castle_030a 19 9
--                   | EXAMINE on 'boxes' at castle_030a 18 8
-------------------------------------------------------------------------------
local box = event.me
local player = event.activator

--------------------------------------
-- Only print the special examine messages if the player has been given the
-- quest. IOW he can't find what he doesn't know he is looking for. However, he
-- can always apply the box to enter the dungeon, regardless of if he has
-- accepted the quest or not.
--------------------------------------
if player:GetQuest("Rusty Rod Retrieval") then
    event.returnvalue = 1

    if box.type == game.TYPE_EXIT then
        player:Write("Aha! The dirt around the " .. box.name .. " shows " ..
                     "signs of recent activity and... yes, they have been " ..
                     "arranged to conceal a small hole in the ground. Hm, " ..
                     "looks like a squeeze but if you hold your breath you " ..
                     "could wriggle down there.")
    else
        player:Write("The " .. box.name .. " seems to be a perfectly " ..
                     "normal " .. box.name .. ".")
    end
end
