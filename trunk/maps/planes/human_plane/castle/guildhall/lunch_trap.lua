-------------------------------------------------------------------------------
-- lunch_trap.lua -- TRIGGER on trap at fanrir_larder_0000 4 1
--
-- Part of Advisor Fanrir's second quest. Introduces
-- newbs to traps, damage and eating to heal.
-------------------------------------------------------------------------------
local trap = event.me
local player = event.activator

require("interface_builder")
local ib = InterfaceBuilder()

-- Make sure we run this script only once
local ds=DataStore("fanrir", player)

if ds:Get("Find Fanrir's Lunch trap sprung") == true then
    event.activator:Write("Oh no, not again...")

    return
end

ds:Set("Find Fanrir's Lunch trap sprung", true)
trap.environment:CreateObjectInside("food_generic", 1, 1, 5)

-- Give the player some time to realise that something happened
coroutine.yield(1)

ib:SetHeader("monk.151", "Advisor Fanrir")
ib:SetMsg("|** Fanrir shouts down the stairs **|\n")
ib:AddMsg("\nAre you ok?\n")
ib:AddMsg("\nSorry about that. The ceiling is a bit unstable in there.\n")
ib:AddMsg("\nBut don't worry, you can just eat some food to recover your health.\n")
ib:AddMsg("\n`Hint -- Picking up items`\n")
ib:AddMsg("\nTo pick up the food, there are two steps:\n")
ib:AddMsg("\n|(1)| move the cursor over the object with the ~CURSOR KEYS~; " ..
          "and\n")
ib:AddMsg("\n|(2)| press ~G~.\n")
ib:AddMsg("\nThis moves the food to your inventory (you'll remember you " ..
          "open this by holding down ~SHIFT~).\n")
ib:AddMsg("\n`Hint -- Dropping items`\n")
ib:AddMsg("\nWhile in your inventory you can drop items. Again, there are " ..
          "two steps:\n")
ib:AddMsg("\n|(1)| move the cursor over the object with the ~CURSOR KEYS~; " ..
          "and\n")
ib:AddMsg("\n|(2)| press ~D~.\n")
ib:AddMsg("\nBut I'm sure you don't want to drop anything now! So eat " ..
          "some food to replenish your health (~G~et it, open your " ..
          "inventory, then ~A~pply the food).")
ib:ShowSENTInce(game.GUI_NPC_MODE_RHETORICAL)
