-- Fanrir Quest - Guildhall Startquests 1,2 and 3 - gh_fanrir.lua
--
-- This script is a bit complex, because it handles three quests
--
require("topic_list")
require("quest_manager")
require("interface_builder")

local pl        = event.activator
local me        = event.me

-- quest names must be unique, the player will store the name forever
local q_mgr_1 = QuestManager(pl, "Examine Fanrir's Sack")
local q_mgr_2 = QuestManager(pl, "Find Fanrir's Lunch")
local q_mgr_3 = QuestManager(pl, "Mouse Hunt")
local q_status_1 = q_mgr_1:GetStatus()
local q_status_2 = q_mgr_2:GetStatus()
local q_status_3 = q_mgr_3:GetStatus()

local ib = InterfaceBuilder()

-- We need the quest descriptions both for the dialogue and for the quest
-- list, so we put them in reusable variables
local quest1_body = [[
This is your first ~quest~! Open the sack next to me and examine the sandals inside. Then '~T~'alk to me again.

|Hints|
You have to learn how to use a container. A container is a chest, sack or bag, a shelf on a wall, a bookcase or a desk, or a similar object.

To open a container, first stand on top of it. Then, move the blue cursor over the container you want to open. Move the cursor with the ~cursor keys~, which have the arrows on them.

Next, apply the container to open it. Do this by pressing '~A~'.

After you open the container, you can examine the items inside the container. Move the cursor over them like before and press '~E~'.
]]

local quest2_body = [[
Just down the stairs over there to the west is my larder. In it are three chests.

Please go down there and find some food.

Careful though! I've been having problems with mice down there recently. Filthy vermin! Two of the chests have traps in them. The food is in the third, but I can't remember which is which!

If you see any of the mice just ignore them. They won't attack you if you leave them alone... They just eat my food! *grr*

|Hints|
To go down the stairs (or use any exit) just move the cursor over them and apply them as you did to open the sack.

If you're unlucky enough to open a trapped chest, the trap will be sprung immediately and you will take some damage.

If your hitpoints ever reach zero, you will die (but don't worry, these traps aren't that strong).
]]

local quest3_body = [[
Return to my larder. Remember those mice I told you about? Well it seems they're learning to avoid my traps.

Could you kill some of the mice please? I'll pay you if you can bring back at least two of their tails.

|Hints|
First turn on combat mode by pressing '~C~' (pressing it a second time turns combat mode off again.) You can see which mode you are in by looking at the icon in the lower left part of the screen.

When you see a mouse, simply press '~X~' to target the enemy.

Now, when you stand next to the mouse you will attack him.
]]

-- Primary entrypoint to any dialogue. Make sure to adjust the
-- text and available choices depending on the current quest status.
local function topicGreeting()
    if q_status_1 == game.QSTAT_NO then
        ib:SetMsg("Hello and welcome to Daimonin!\n")
        ib:AddMsg("\nMy name is " .. me:GetName() .. ".\n")
        ib:AddMsg("\nI have a ^quest^ for you. Just click the highlighted keyword (or press ~RETURN~ and type it in) and we can get started.\n")
        ib:AddMsg("\n|Hints|\n")
        ib:AddMsg("\nMost characters you will meet in Daimonin will offer you quests.")
        ib:AddLink("Tell me about the quest", "explain quest")
    elseif q_status_1 == game.QSTAT_DONE and q_status_2 == game.QSTAT_NO then
        ib:SetMsg("I don't know about you, but I'm a bit peckish now.\n")
        ib:AddMsg("\nI have an idea! For your next ^quest^ you can get yourself some food from my larder.")
        ib:AddLink("Tell me more about the quest", "explain quest")
    elseif q_status_1 == game.QSTAT_DONE and q_status_2 == game.QSTAT_DONE and q_status_3 == game.QSTAT_NO then
        ib:SetMsg("The last ^quest^ I have for you will teach you the basics of combat and recovering your health afterwards.")
        ib:AddLink("Tell me more about the quest", "explain quest")
    elseif  q_status_1 == game.QSTAT_DONE and q_status_2 == game.QSTAT_DONE and q_status_3 == game.QSTAT_DONE then
       -- TODO: check if the player has already been to cashin
       ib:SetMsg("I am sorry, I have no more to teach you.\n")
       ib:AddMsg("\nYour next step should be to go and talk to Cashin of the Mercenary Guild. You'll find him in the first building just down this road.")
    else
        ib:SetMsg("Hello again. Have you finished my quest?")
        ib:AddLink("Yes, I have finished it", "quest complete")
    end
    
    ib:SetHeader(me)
    ib:SetTitle("Hello")
    pl:Interface(1,ib:Build())
end

local function figureOutWeapon()
    -- First we need to find out the player's weapon skill
    skillitems = {
        [game:GetSkillNr("slash weapons") ] = "shortsword",
        [game:GetSkillNr("impact weapons")] = "mstar_small",
        [game:GetSkillNr("cleave weapons")] = "axe_small",
        [game:GetSkillNr("pierce weapons")] = "dagger_large"
    }
    
    for nr,weap in pairs(skillitems) do
        -- See if the player has one of the weapon skills above
        skill = pl:FindSkill(nr) 
        if skill then
            -- Only give a weapon if the player hasn't used the
            -- skill before.
            if skill.experience == 0 then
                -- TODO: only give the weapon if we can't find one
                -- belonging to the same skill group already in the player inventory
                return weap
            end
        end
    end
end

-- The player asks about available quests
local function topicQuest()
    local curr_q
    if q_status_1 == game.QSTAT_ACTIVE then curr_q = q_mgr_1 
    elseif q_status_2 == game.QSTAT_ACTIVE then curr_q = q_mgr_2 
    elseif q_status_3 == game.QSTAT_ACTIVE then curr_q = q_mgr_3 
    end 

    if curr_q then
        ib:SetTitle("Quest Incomplete")
        ib:SetMsg("You haven't completed the current quest yet. Please hurry up!\n\n")
        ib:AddQuestChecklist(curr_q)
        ib:AddMsg("\n\n|Hint|\nYou can check your progress on the quest in your questlist.\n")
        ib:AddMsg("Just click the 'Quest' button in the top right corner of the screen\n")
    elseif q_status_1 == game.QSTAT_NO then
        ib:SetTitle(q_mgr_1.name)
        ib:AddMsg(quest1_body)
        
        -- This will not be part of the quest list description
        ib:AddMsg("\n|Accept Quest?|\nTo start this quest, press the ~Accept~ button."..
            "The info above will then also become available in your quest list.")
        
        ib:SetAccept("Accept", "accept quest")
    elseif q_status_2 == game.QSTAT_NO then
        ib:SetTitle(q_mgr_2.name)
        ib:AddMsg(quest2_body)
        ib:SetAccept("Accept", "accept quest")
    elseif q_status_3 == game.QSTAT_NO then
        ib:SetTitle(q_mgr_3.name)
        ib:AddMsg(quest3_body)
        if figureOutWeapon() then
            ib:AddMsg("\n|Weapon Needed|\nDo you need a weapon? I'll give you a basic one if you accept the quest.")
        end
        ib:SetAccept("Accept", "accept quest")
    else
        topicGreeting()
        return
    end
    ib:SetHeader(me)
    pl:Interface(1,ib:Build())
end

-- The player wants to accept a quest. Activate the next accessible one.
-- TODO: make sure player can't have two of fanrir's quests active at the same
-- time.
local function topicAccept()
    if q_status_1 == game.QSTAT_NO then
        ib:SetTitle(q_mgr_1.name)
        ib:SetMsg(quest1_body)
        q_mgr_1:SetFinalStep(1) -- Needed since this is a QUEST_NORMAL
        if q_mgr_1:RegisterQuest(game.QUEST_NORMAL, ib) then
            pl:Sound(0, 0, 2, 0)
            pl:Write("You take the quest '"..q_mgr_1.name.."'.", game.COLOR_NAVY)
        end
    elseif q_status_2 == game.QSTAT_NO then
        ib:SetTitle(q_mgr_2.name)
        ib:SetMsg(quest2_body)
        if q_mgr_2:RegisterQuest(game.QUEST_ITEM, ib) then
            q_mgr_2:AddQuestItem(1, "quest_object", "bread02.101", "Fanrir's Lunch")
            pl:Sound(0, 0, 2, 0)
            pl:Write("You take the quest '"..q_mgr_2.name.."'.", game.COLOR_NAVY)
        end
    elseif q_status_3 == game.QSTAT_NO then
        ib:SetTitle(q_mgr_3.name)
        ib:SetMsg(quest3_body)
        if q_mgr_3:RegisterQuest(game.QUEST_KILLITEM, ib) then
            -- target and item (first 2=drop at 1/2 of kills, second 2=nrof we need)
            -- important: The 2 on AddQuestItem overrides the AddQuestTarget
            q_mgr_3:AddQuestTarget(2, 2, "mouse", "Mouse"):
                AddQuestItem(2, "quest_object", "tail_rat.101", "Mouse tail")
            pl:Sound(0, 0, 2, 0)
            pl:Write("You take the quest '"..q_mgr_3.name.."'.", game.COLOR_NAVY)

            new_weapon = figureOutWeapon()
            if new_weapon then
                me:SayTo(pl, "Here, take this weapon for the task")
                tmp = pl:CreateObjectInsideEx(new_weapon, 1, game.IDENTIFIED)
                pl:Apply(tmp, game.APPLY_ALWAYS)
            end
        end
    else
        topicGreeting()
        return
    end
    pl:Interface(-1)
end

-- The player claims to have completed a quest. Double check and
-- possibly give out rewards
local function topicQuestComplete()
    if q_status_1 == game.QSTAT_ACTIVE or
            q_status_2 == game.QSTAT_ACTIVE or
            q_status_3 == game.QSTAT_ACTIVE then
            topicQuest()
            return
    elseif q_status_1 == game.QSTAT_SOLVED then
        ib:SetTitle("Quest Complete")
        ib:SetMsg("Well done! I'll tell you what. As a reward for a job well done I'm going to give you those sandals.\n")
        ib:AddIcon("Sandals", "sandals.101", "Apply these to increase your resistances.")
        ib:SetButton("ok", "hello")
    
        q_mgr_1:Finish()
        pl:Sound(0, 0, 2, 0)
        pl:CreateObjectInsideEx("sandals", game.IDENTIFIED, 1)
    elseif q_status_2 == game.QSTAT_SOLVED then
        ib:SetTitle("Quest Complete")
        ib:SetMsg("Congratulations! Sorry again for the rocks but I'm sure it was worth it.\n")
        ib:AddIcon("Bread", "bread02.101", "Fanrir offers you some of his lunch as a reward.")
        ib:SetButton("ok", "hello")
        q_mgr_2:RemoveQuestItems()
        q_mgr_2:Finish()
        pl:Sound(0, 0, 2, 0)
        pl:CreateObjectInsideEx("bread1", game.IDENTIFIED, 4)
    elseif q_status_3 == game.QSTAT_SOLVED then
        ib:SetTitle("Quest Complete")
        ib:SetMsg("Many thanks! I hope that will be the end of them.\n")
        ib:AddMsg("\nNow you should go and talk to Mercenary Master Cashin inside the building down the road.\n")
        ib:AddMsg("\nThank you, " .. pl:GetName() .. "!")
        ib:SetButton("bye", "")
        
        q_mgr_3:RemoveQuestItems()
        q_mgr_3:Finish()
        pl:Sound(0, 0, 2, 0)
        pl:AddMoneyEx(10, 0, 0, 0)
    else
        topicGreeting()
        return
    end
    ib:SetHeader(me)
    pl:Interface(1,ib:Build())
end

tl = TopicList()
tl:SetDefault(topicGreeting)
tl:AddTopics({"quest", "explain%s+quest"}, topicQuest)
tl:AddTopics({"accept", "accept%s+quest"}, topicAccept)
tl:AddTopics({"complete", "quest%s+complete"}, topicQuestComplete)

tl:CheckMessage(event)
