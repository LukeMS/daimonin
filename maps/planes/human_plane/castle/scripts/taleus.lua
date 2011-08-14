-- merc_taleus.lua
-- Teaches a single archery skill. Quest missing.
--

require("topic_list")
require("interface_builder")
require("quest_builder")

local qb        = QuestBuilder()
local pl        = event.activator
local me        = event.me
local ib        = InterfaceBuilder()
if not module_guildsLOADED then
    require("modules/guilds")
end
local gstat = module_guildsGetStatus("Mercenary", pl)

local skill_bow = game:GetSkillNr("bow archery")
local skill_cbow = game:GetSkillNr("crossbow archery")
local skill_sling = game:GetSkillNr("sling archery")
--

local function questGoal(questnr)
    qb:AddQuestTarget(questnr, 1, 1, "goblin", "Hagvash"):
    AddQuestItem(1, "quest_object", "power_crystal.101", "Raiding Party Clue")
    pl:Sound(0, 0, 2, 0)
    pl:Write("You take the quest '".. qb:GetName(questnr) .."'.", game.COLOR_NAVY)
end

qb:AddQuest("Rats in Stonehaven?", game.QUEST_KILLITEM, nil, nil, nil, nil, 1, questGoal, nil)
local questnr = qb:Build(pl)
local QStatus = qb:GetStatus(1)

local function quest_reminder()
    ib:SetTitle(qb:GetName(questnr))
    ib:AddMsg("Haven't you taken care of it yet? Come back when you have!\n")
    ib:AddQuestChecklist(qb, questnr)
    ib:SetButton("Back", "hi") 
end
    
local function quest_icons1()
	ib:AddSelect("Learn Archery skill of Sling ", "quest complete 1", "sling_small.101", "Fastest, but hits lowest of the three")
	ib:AddSelect("Learn Archery skill of Bow ", "quest complete 2", "bow_short.101", "Balanced between fast and strong")
	ib:AddSelect("Learn Archery skill of Crossbow ", "quest complete 3", "crossbow_small.101", "Strongest, but slowest of the three")
end

local function quest_body1()
    ib:SetHeader("st_003", me)
    ib:SetTitle(qb:GetName(questnr))
    ib:SetMsg("We have had reports about strange noises from around one of the buildings on the east side of the town.\n\n")
    ib:SetMsg("The castle guards think it is rats and won't look into it. I must remain here in case my shipment arrives, and most other warriors are otherwise occupied. It would be great if you could investigate and take care of any problems.\n\n")
    ib:SetMsg("It seems most of the noises come from the first building on the eastern road. The woman who lives there will tell you more. Just follow this road south to the intersection by the main gates. Then go east and follow the road to the first house on the right hand side.")
    ib:SetDesc("Investigate the noises and handle any problems", 0, 0, 0, 0)
    quest_icons1()
end

function topicDefault()
    ib:SetHeader("st_001", me)
    if gstat == game.GUILD_IN or module_guildsPlayerGuildless(pl) then
        if pl:FindSkill(skill_bow) ~= nil or pl:FindSkill(skill_cbow) ~= nil or pl:FindSkill(skill_sling) ~= nil or QStatus == game.QSTAT_DONE then
            ib:SetTitle("Hello again")
            ib:AddMsg("Your basic mercenary training is probably almost finished by now.")
        elseif QStatus == game.QSTAT_NO then
            ib:SetTitle("Greetings")
            ib:AddMsg("\n\nHello, mercenary. I am Archery Commander Taleus.")
            ib:AddMsg("\n\nI have taken over from my mentor, ^Chereth^, since she lost her eyes.")
            ib:AddMsg("\n\nWell, I know a lot about ^archery^ and I ^train^ members of the mercenary guild.")
        elseif QStatus == game.QSTAT_SOLVED then
            topicQuestComplete()
            return
        elseif QStatus == game.QSTAT_ACTIVE then
            quest_reminder()
        else
            pl:Write(me.name .." has nothing to say.", game.COLOR_NAVY)
            return
        end
    else
        ib:SetTitle("Hello")
        ib:SetMsg("I'm sorry, but I cannot teach you an archery skill due to your guild.")
        ib:AddMsg(" You can either join the Mercenary guild or leave your current guild.")
    end
end

-- Player asks for quests
function topQuest()
    if QStatus == game.QSTAT_ACTIVE then
        quest_reminder()
    elseif QStatus == game.QSTAT_NO then 
        quest_body1()
        ib:SelectOff()
        ib:SetAccept(nil, "accept quest") 
        ib:SetDecline(nil, "hi") 
    else
        topicDefault()
        return
    end
end
-- accepted: start the quest
function topAccept()
    if QStatus == game.QSTAT_NO then
        quest_body1()
        ib:SelectOff()
        qb:RegisterQuest(questnr, me, ib)
   else
       topicDefault()
        return
    end
end

function topChereth()
    ib:SetHeader("st_004", me)
    ib:SetTitle("Chereth")
    ib:SetMsg("\n\nYes, Chereth lost her eyes in a major battle. When you find her, she can tell you more.")
    ib:SetButton("Back", "hi")
end

function topArchery()
    ib:SetHeader("st_003", me)
    ib:SetTitle("Archery Info")
    ib:SetMsg("\n\nYes, there are three archery skills:\n")
    ib:AddMsg("\nBow Archery is the most common, firing arrows.\n")
    ib:AddMsg("\nSling Archery allows fast firing stones with less damage.\n")
    ib:AddMsg("\nCrossbow Archery uses x-bows and bolts. Slow but powerful.\n")
    if gstat ~= game.GUILD_IN and not module_guildsPlayerGuildless(pl) then
        ib:AddMsg("\nI teach archery for the mercenary guild, but I can see that you're not a member.\n")
        ib:AddMsg("You should talk to Cashin, who is in the Guildhall southeast of this castle, about joining the mercenary guild.")
    else
        ib:AddMsg("\nWell, there are the three different archery skills.\n\nI can teach you only ~*ONE*~ of them.\n")
        ib:AddMsg("You have to stick with it, so |choose wisely|.\n")
        ib:AddMsg("But before I teach you, I have a little ^quest^ for you.")
        ib:AddLink("Could you tell me what this 'quest' is?", "explain quest")
    end
    ib:SetButton("Back", "hi")
end
function topicQuestComplete(reward)
    if reward == nil then
		if QStatus == game.QSTAT_ACTIVE then
			quest_reminder()
		else	
			ib:AddMsg("Very well done! It was worse than I thought. I wonder where those kobolds came from--I thought we'd driven most of them out.\n\n")
			ib:AddMsg("I will take the crystal you found to Jahrlen, he might be able to understand it.\n\n")
			ib:AddMsg("Now you must choose your archery skill:")
			quest_icons1()
		end
	else
		if reward == "1" then
			ib:SetTitle("Sling archery")
			ib:SetMsg("Ah, you chose sling archery. An excellent choice!")
			pl:AcquireSkill(skill_sling, game.LEARN)
			pl:CreateObjectInsideEx("sling_small", game.IDENTIFIED,1)
			pl:CreateObjectInsideEx("sstone", game.IDENTIFIED,20)
		elseif reward == "2" then
			ib:SetTitle("Bow archery")
			ib:SetMsg("Ah, you chose bow archery. An excellent choice!")
			pl:AcquireSkill(skill_bow, game.LEARN)
			pl:CreateObjectInsideEx("bow_short", game.IDENTIFIED,1)
			pl:CreateObjectInsideEx("arrow", game.IDENTIFIED,20)
		elseif reward == "3" then
			ib:SetTitle("Crossbow archery")
			ib:SetMsg("Ah, you chose crossbow archery. An excellent choice!")
			pl:AcquireSkill(skill_cbow, game.LEARN)
			pl:CreateObjectInsideEx("crossbow_small", game.IDENTIFIED,1)
			pl:CreateObjectInsideEx("bolt", game.IDENTIFIED,20)
		end
		qb:Finish(1)
	end
end

tl = TopicList()
tl:AddGreeting(nil, topicDefault)
tl:SetDefault(topicDefault)
if gstat == game.GUILD_IN or module_guildsPlayerGuildless(pl) then
    if qb:GetStatus(1) < game.QSTAT_DONE then
        tl:AddTopics({"quest", "explain%s+quest"}, topQuest)
        tl:AddTopics({"accept", "accept%s+quest"}, topAccept) 
        if qb:GetStatus(1) == game.QSTAT_SOLVED then
            tl:AddTopics({"complete", "quest%s+complete"}, topicQuestComplete) 
            tl:AddTopics({"complete", "quest%s+complete%s*#?(%d*)"}, topicQuestComplete)
        end
    end
end
tl:AddTopics({"archery", "train"}, topArchery)
tl:AddTopics({"chereth", "rumors", "rumours", "rumor", "rumour"}, topChereth)
ib:ShowSENTInce(game.GUI_NPC_MODE_NPC, tl:CheckMessage(event, true))