-- Tests whether the "event" global is untouched after
-- a recursive script trigger. 
--
-- The Communicate() call below triggers LuaTester's script
-- before continuing execution.

-- print ("Drow before: " .. event.me.name)

local name = event.me.name
event.me:Communicate("This is a test, my name is " .. event.me.name)

-- print ("Drow after: " .. event.me.name)

local newname = event.me.name
if newname == name then
    event.me:Say("Test successful, I'm still me");
else
    event.me:Say("Test failed! I'm not " .. name .. " anymore ");
end
