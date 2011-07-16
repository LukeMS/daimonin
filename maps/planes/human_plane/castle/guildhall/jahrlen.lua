-- jahrlen.lua

-- Jahrlen offers quests to Wizard guild members and guildless players only:
-- 1. (Currently merged with second quest due to lack of time) Reward: wizardry skill
-- 2. Kill Rat King. Location: Rat infested well. Lvl: 2-3. Reward: probe spell.
-- 3. Retrieve Rusty Rod. Location: Beneath Guild Hall. Lvl: 4-5. Reward: Magic Bullet

require("topic_list")
require("quest_builder")
require("interface_builder")
if not module_guildsLOADED then
    require("modules/guilds")
end

local pl        = event.activator
local me        = event.me
local qb        = QuestBuilder()
local ib        = InterfaceBuilder()

local function questGoal(questnr)
    if questnr == 1 then
        qb:AddQuestTarget(questnr, 0, 1, "rat_d", "Rat King"):
                AddQuestItem(1, "quest_object", "tail_rat.101", "Rat King's tail")
    elseif questnr == 2 then
        qb:AddQuestItem(questnr, 1, "quest_object", "rod_light.101", "Rusty Rod")
    end
    pl:Sound(0, 0, 2, 0)
    pl:Write("You take the quest '"..qb:GetName(questnr).."'.", game.COLOR_NAVY)
end

qb:AddQuest("Rat King of the Water Well", game.QUEST_KILLITEM, nil, nil, nil,
            nil, 1, questGoal, nil)
qb:AddQuest("Rusty Rod Retrieval", game.QUEST_ITEM, nil, nil, nil,
            nil, 1, questGoal, nil)

local questnr = qb:Build(pl)
local qstat_1 = qb:GetStatus(1)
local qstat_2 = qb:GetStatus(2)

-- Guild checks
local guild_tag = "Wizard"
local guild_stat = module_guildsGetStatus(guild_tag, pl)
-- 
-- Helper functions
--

-- Helper fucntion for spell teaching
local function teachSpell(spell)
    local skill = game:GetSkillNr("wizardry spells")
    if pl:FindSkill(skill) == nil then
        ib:AddMsg("\n\nFirst I will teach you the needed skill: ~wizardry spells~")
        pl:AcquireSkill(skill, game.LEARN)
    end
    local spellno = game:GetSpellNr(spell)
    if pl:DoKnowSpell(spellno) then
        ib:AddMsg("\n\nYou already know the spell '^"..spell.."^'!" )
    else
        pl:AcquireSpell(spellno, game.LEARN)
        ib:AddMsg("\n\nYou learn the spell '^"..spell.."^'")
    end
end

local function quest1_body()    
    ib:SetTitle(qb:GetName(questnr))
    ib:AddMsg("Yes, we could use some help.\n")
    ib:AddMsg("\nIn the last few days we have noticed some problems with our main water source.\n")
    ib:AddMsg("\nIt seems that rats have invaded the caverns under our water well and are making the water unsafe to " ..
              "drink.\n")
    ib:AddMsg("\nEnter the well and kill the Rat King!\n")
    ib:AddMsg("\nBring me his tail as a trophy and then I will teach you the ^Probe^ spell.\n")
    ib:AddMsg("\nThe well is located between the smithy and the shop, just east of here.")
    ib:SetDesc("Kill the Rat King and bring me back his tail.", 0, 0, 0, 0)
end

local function quest2_body()
    ib:SetTitle(qb:GetName(questnr))
    ib:AddMsg("I was refurbishing a rusty old rod I found in a pile of rubbish. When it is repaired, and its magic is " ..
              "restored, it will actually be quite valuable.\n")
    ib:AddMsg("\nHowever, a nasty little hobgoblin by the name of ^Mahch^ crept in here late one night and stole it.\n")
    ib:AddMsg("\nTeach him a lesson he won't forget, and I will reward you by teaching you the ^Magic Bullet^ spell.\n")
    ib:AddMsg("\nThe guards have seen Mahch sneaking around the boxes outside the guild, but they have never been able " ..
              "to catch him.")
    ib:SetDesc("Do whatever it takes to get the rusty rod back from Mahch the hobgoblin. Then return the rod to me.",
               0, 0, 0, 0)
end

-- Generate a simple and generic reminder for an unfinished quest
local function quest_reminder()
    ib:SetTitle(qb:GetName(questnr))
    ib:SetMsg("You haven't done as I asked. Please finish your quest then come back.")
end

--
-- Main dialogue topics
--

function topicGreeting()
    if guild_stat ~= game.GUILD_IN and module_guildsPlayerGuildless(pl) == false then
        -- They are not in the guild and they are not guildless.
            ib:SetHeader("st_001", me)
            ib:SetMsg("I'm sorry, but I cannot teach members of your guild these spells.\n\n")
    elseif qstat_1 == game.QSTAT_NO then
        -- First real welcome text.
        ib:SetHeader("st_001", me)
        ib:SetMsg("\n\nHello! I am Jahrlen, war ^Chronomancer^ of Thraal.\n\n")
        ib:AddMsg("I can teach you the wizardry skill and the ^Probe^ and ^Magic Bullet^ spells.\n\n")
        ib:AddMsg("But you will have to do something for me. Are you interested?")
        ib:AddLink("Please tell me about the quest", "explain quest")
    elseif qstat_1 == game.QSTAT_DONE and qstat_2 == game.QSTAT_NO then
        ib:SetHeader("st_001", me)
        ib:SetMsg("You have done a good job with the rats. I still have the ^Magic Bullet^ spell to teach you. If you retrieve an item I have lost, I'll teach it to you.")
        ib:AddLink("Tell me about the quest", "explain quest")
    elseif qstat_1 == game.QSTAT_DONE and qstat_2 == game.QSTAT_DONE then
        ib:SetHeader("st_001", me)
        ib:SetMsg("Thank you for getting my rod back. I think that soon it will be as good as new again. Two weeks.\n\n")
        ib:AddMsg("I have already taught you ^Probe^ and ^Magic Bullet^. I have nothing more for you to do.\n\n")
        ib:AddMsg("I suggest you now go and talk to Taleus about learning archery. His old teacher Chereth, who is standing over there, might know where he is.\n\n")
        ib:AddMsg("If you meet Taleus, tell him his ^Chronomancer^ said hello.")
    else
        -- We will autmatically reward or remind about the quest in the QuestComplete() function
        topicQuestComplete()
        return
    end
    
    ib:SetTitle("Greetings!")
end

-- The player asks about available quests
function topicQuest()
    if unfinished_q then
        quest_reminder()
    elseif qstat_1 == game.QSTAT_NO then
        ib:SetHeader("st_003", me)
        quest1_body()
        ib:SetAccept("Accept", "accept quest")
    elseif qstat_1 == game.QSTAT_DONE and qstat_2 == game.QSTAT_NO then
        ib:SetHeader("st_003", me)
        quest2_body()
        ib:SetAccept("Accept", "accept quest")
    else
        topicGreeting()
        return
    end
end

-- The player wants to accept a quest. Activate the next accessible one.
-- TODO: make sure player can't have two quests active at the same time.
function topicAccept()
    if qstat_1 == game.QSTAT_NO then
        ib:SetHeader("st_003", me)
        quest1_body()
        qb:RegisterQuest(questnr, me, ib)
    elseif qstat_1 == game.QSTAT_DONE and qstat_2 == game.QSTAT_NO then
        ib:SetHeader("st_003", me)
        quest2_body()
        qb:RegisterQuest(questnr, me, ib)
    else
        topicGreeting()
        return
    end
end

-- The player claims to have completed a quest. Double check and
-- possibly give out rewards
function topicQuestComplete()
    if unfinished_q then
        quest_reminder()
    elseif qstat_1 == game.QSTAT_SOLVED then
        ib:SetHeader("st_003", me)
        ib:SetTitle("Quest Complete")
        ib:SetMsg("Very well done. Hopefully we won't have any more rat trouble for some time.\n\n")
        --ib:AddMsg("I'll now teach you the ~Wizardry Spells~ skill and the ^Probe^ spell")
        teachSpell("probe") 
        ib:SetButton("Back", "hello")
        qb:Finish(questnr)
        pl:Sound(0, 0, 2, 0)
    elseif qstat_2 == game.QSTAT_SOLVED then
        ib:SetHeader("st_003", me)
        ib:SetTitle("Quest Complete")
        ib:SetMsg("Great! It seems Mahch had made himself a pretty good nest down there. I'm glad you made it back okay.")
        ib:SetButton("Back", "hello")
        teachSpell("magic bullet")
        qb:Finish(questnr)
        pl:Sound(0, 0, 2, 0)
    else
        topicGreeting()
        return
    end
end

--
-- Rumour/info topics
--

local function topChrono()
    ib:SetHeader("st_002", me)
    ib:SetTitle("Chronomancer")
    ib:SetMsg("\n\nYes, I am a master of the chronomancers of Thraal.\n\n")
    ib:AddMsg("We are one of the more powerful wizard guilds. ")
    ib:AddMsg("Perhaps when you are higher in level and stronger...\n\n")
    ib:AddMsg("Hmm... If you ever meet ^Rangaron^ in your travels, tell him that Jahrlen has sent you.\n\n")
    ib:AddMsg("Don't ask me more now.")
    ib:SetButton("Back", "hi")
end

local function topRangaron()
    ib:SetHeader("st_004", me)
    ib:SetTitle("Rangaron")
    ib:SetMsg("\n\nI said 'Don't ask me more now'! You have problems with your ears??\n\n")
    ib:AddMsg("If you meet Rangaron, and I'm sure you will, then tell him what I told you.")
    ib:AddMsg("\nHis assistant Lon'Li'Uni'Sm may know where he is.")
    ib:SetButton("Back", "hi")
end

local function topProbe()
    ib:SetHeader("st_002", me)
    ib:SetTitle("Probe")
    ib:SetMsg("\n\nI can teach you the ~Probe~ spell.\n\n")
    ib:AddMsg("It is one of the most useful information spells you can learn!\n\n")
    ib:AddMsg("Cast on unknown creatures it will grant you knowledge of their powers and weaknesses.\n\n")
    ib:AddMsg("The spell itself is very safe. Creatures will not notice that they were probed. ")
    ib:AddMsg("They will not get angry or attack.")
    ib:SetButton("Back", "hi")
end

local function topMagicBullet()
    ib:SetHeader("st_002", me)
    ib:SetTitle("Magic Bullet")
    ib:SetMsg("\n\nI can teach you the ~Magic Bullet~ spell.\n\n")
    ib:AddMsg("This can be used to attack enemies from a distance.\n\n")
    ib:AddMsg("The more powerful you become in the art of Wizardry Spells, the more damage ")
    ib:AddMsg("your Magic Bullet will do.\n\n")
    ib:AddMsg("But be careful with its use: a Magic Bullet will travel until it hits something, ")
    ib:AddMsg("be it a wall or a monster that could be beyond your range of vision. Be assured that ")
    ib:AddMsg("a monster you disturb like this will hunt you down!")
    ib:SetButton("Back", "hi")
end

local function topMahch()
    ib:SetHeader("st_002", me)
    ib:SetTitle("Mahch")
    ib:SetMsg("\n\nYes, he is a nasty little hobgoblin.\n\n")
    ib:AddMsg("He creeps around at night stealing things.\n\n")
    ib:AddMsg("We have tried to see where his bolt hole is, and we think it might be somewhere ")
    ib:AddMsg("close to some boxes outside of the Mercenery guild.")
    ib:SetButton("Back", "hi")
end

tl = TopicList()
tl:AddGreeting(nil, topicGreeting)
tl:SetDefault(topicGreeting)

if guild_stat == game.GUILD_IN or module_guildsPlayerGuildless(pl) == true then
    tl:AddTopics({"quest", "explain%s+quest"}, topicQuest)
    tl:AddTopics({"accept", "accept%s+quest"}, topicAccept)
    tl:AddTopics({"complete", "quest%s+complete"}, topicQuestComplete)

    tl:AddTopics("chronomancer", topChrono)
    tl:AddTopics("rangaron", topRangaron)
    tl:AddTopics("probe", topProbe)
    tl:AddTopics("magic%s+bullet", topMagicBullet)
    tl:AddTopics("mahch", topMahch)
end

ib:ShowSENTInce(game.GUI_NPC_MODE_NPC, tl:CheckMessage(event, true))