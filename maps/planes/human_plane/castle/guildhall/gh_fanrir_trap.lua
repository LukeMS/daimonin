--
-- Part of Advisor Fanrir's second quest. Introduces
-- newbs to traps, damage and eating to heal.
--

require "interface_builder"

-- Make sure we run this script only once
local ds=DataStore("gh_fanrir", event.activator)
if ds:Get("lunch_trap") == true then
    event.activator:Write("Oh no, not again...")
    return
end
ds:Set("lunch_trap", true)

event.me.environment:CreateObjectInside("food_generic", 1, 1, 5)

-- Give the player some time to realise that something happened
coroutine.yield(1)

ib = InterfaceBuilder()
ib:SetHeader("monk.151", "Advisor Fanrir")
--ib:SetTitle("Fanrir Shouts From the Outside:")
--ib:SetMsg("Are you ok?\n\nSorry about that. The ceiling is a bit unstable in there.\nBut don't worry, your wounds will heal given some time.\n\nYou can also heal faster by °eating° food. Get the rest of the food from the chest and use the °E° key to °Eat° it. Eating will speed up healing a lot.")
ib:SetMsg("|** Fanrir shouts down the stairs **|\n")
ib:AddMsg("\nAre you ok?\n")
ib:AddMsg("\nSorry about that. The ceiling is a bit unstable in there.\n")
ib:AddMsg("\nBut don't worry, you can just eat some food to recover your health.\n")
ib:AddMsg("\n|Hints|\n")
ib:AddMsg("\nTo pick up the food move the cursor over it and press the '~G~' key.\n")
ib:AddMsg("\nNow eat the food by applying it in the same way that you open containers or use stairs.")

event.activator:Interface(1, ib:Build())
