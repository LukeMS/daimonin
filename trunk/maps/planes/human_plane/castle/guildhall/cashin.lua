----------------------------------------------------------------------
-- cashin.lua -- TALK on NPC at castle_030a 11 10
--
-- Guildmaster Cashin is the Guildmaster of the Mercenary Guild.
-- He is a serious man, especially about the guild.
--
-- But his personality must be balanced with his gameplay function as
-- new players' first real link (Fanrir doesn't count) to the gameworld.
-------------------------------------------------------------------------------
-- QUESTS
--
-- 1. "The Mercenary Guild Quest":
-- Requirements: None.
-- Purpose     : Intended as the first 'real' quest
--               (ie, Fanrir can safely be skipped).
-- Scenario    : The Ant Scout Leader in the guild cellar has Cashin's helm.
-- Goal        : Kill the ant and return the helm to Cashin.
-- Reward      : Merc Guild membership and Cashin's helm.
-------------------------------------------------------------------------------
-- SERVICES
---------------------------------------------------------------------------------
local npc = event.me
local player = event.activator

---------------------------------------
---------------------------------------
if not module_guildsLOADED then
    require("modules/guilds")
end

-------------------
-------------------
local gstat = module_guildsGetStatus("Mercenary", player)

---------------------------------------
---------------------------------------
require("interface_builder")

local ib = InterfaceBuilder()

---------------------------------------
---------------------------------------
require("quest_builder")

local qb = QuestBuilder()

-------------------
-------------------
local function questGoal(questnr)
    ---------
    -- "The Mercenary Guild Quest"
    ---------
    if questnr == 1 then
        local target = qb:AddQuestTarget(1, 1, 1, "ant_red",
                                         "Ant Scout Leader")

        target:AddQuestItem(1, "quest_object", "helm_leather.101",
                        "Cashin's helm")
        require("/scripts/first_weapon")
    end
end

-------------------
-------------------
local function questReward(questnr)
    ---------
    -- "The Mercenary Guild Quest"
    ---------
    if questnr == 1 then
        local helm = player:CreateObjectInside("helm_leather", game.IDENTIFIED)

        assert(helm, "Could not create helm!")
        helm.name = "Cashin's old helm"
        helm.f_is_named = 1

        ---------
        -- Auto-equip the helm if player has a bare head.
        ---------
        if not player:GetEquipment(game.EQUIP_HELM) then
            player:Apply(helm, game.APPLY_ALWAYS)
        end

        module_guildsJoin("Mercenary", player)
    end
end

-------------------
-------------------
qb:AddQuest("The Mercenary Guild Quest", game.QUEST_KILLITEM, nil, nil, nil,
            nil, 1, questGoal, questReward)

local questnr = qb:Build(player)

---------------------------------------
-- DEFAULT: The player has tried an unrecognised topic.
---------------------------------------
-------------------
-- topic_default() handles unrecognised topics, giving a nice helpful 'try
-- different words' response.
-- Remember, the topic may be perfectly reasonable but just something this NPC
-- does not specifically respond to, too complex to parse (perhaps there is a
-- simpler way to say the same thing), or complete gobbledigook (maybe the
-- player did a typo). The default response should be valid in all three cases.
-------------------
local function topic_default()
    ib:SetTitle("What?")
    ib:SetMsg("I'm sorry. I didn't understand.\n")
    ib:AddMsg("\n|[The interface's parsing is extremely limited, so keep " ..
              "your input simple. Usually type only a noun (name of " ..
              "something), such as 'quest', or sometimes a verb (an action " ..
              "to do) and a noun, such as 'repair sword', to talk about a " ..
              "subject. But it may also be that this particular NPC simply " ..
              "does not know anything about that topic.]|")
end

---------------------------------------
-- GREETING: A standard topic. The player has said 'hello', etc.
---------------------------------------
-------------------
-- topic_greeting() handles this standard topic.
-------------------
local function topic_greeting()
    ib:SetHeader("st_001", npc)

    ---------
    -- Player is not a guild member and never has been.
    ---------
    if gstat == game.GUILD_NO then
        ib:SetMsg("Greetings! I am " .. npc:GetName() .. ". Welcome to the " ..
                  "^Mercenary Guild^ of ^Thraal^!\n")
        ib:AddMsg("\nOur troops are an important part of the imperial " ..
                  "defence.\n")
        ib:AddMsg("\n|** " .. npc.name .. " looks you up and down **|\n")

        if qb:GetStatus(1) == game.QSTAT_NO then
            ib:AddMsg("\nYou look like you have some potential. Do you " ..
                      "want to join our guild? You really would be welcome " ..
                      "in our ranks.")
            ib:AddLink("Yes, I would like to join the Mercenary Guild.",
                       "join")
            ib:AddLink("No thanks, maybe later.", "")
            ib:SetLHSButton("Services")
        elseif questnr > 0 then
            ib:AddMsg("\nI hope that ^quest^ I gave you isn't proving too " ..
                      "difficult for you.")
            ib:SetLHSButton("Quest")
        end
    ---------
    -- Player is not a guild member but used to be.
    ---------
    elseif gstat == game.GUILD_OLD then
        if math.random() >= 0.50 then
            ib:SetMsg("Ah! Look who's back?\n")
        else
            ib:SetMsg("Well if it isn't ~ex~-Guildmember " .. player.name ..
                      ".\n")
        end

        ib:AddMsg("\nAre you ready to rejoin the guild yet? Thraal needs " ..
                  "all the defenders it can get nowadays.")
        ib:AddLink("Yes, I would like to rejoin the Mercenary Guild.",
                   "rejoin")
        ib:AddLink("Not now thanks, maybe later.", "")
        ib:SetLHSButton("Services")
    ---------
    -- Player is currently a guild member.
    ---------
    else
        ib:SetMsg("Hello Guildmember " .. player.name .. ".\n")
        ib:AddMsg("\n|** You salute each other **|\n")

        ---------
        -- Player has completed all quests.
        ---------
        if questnr == 0 then
            ib:SetMsg("I have nothing more for you to do.\n")
            ib:AddMsg("\nThe guild values experience. Go and get some.")
            ib:SetLHSButton("Services")
        ---------
        -- Next quest is disallowed (ie, player does not meet level/skill req).
        ---------
        elseif questnr < 0 then
            ib:SetMsg("I have nothing for you to do at the moment.\n")
            ib:AddMsg("\nThe guild values experience. Come back later.")
            ib:SetLHSButton("Services")
        ---------
        -- Player is eligible for a quest.
        ---------
        else
            ib:AddMsg("\nAs a member you are eligible to take guild ^quests^.")
            ib:SetLHSButton("Quest")
            ib:SetRHSButton("Services")
        end
    end
end

---------------------------------------
-- BACKGROUND: A standard topic.
---------------------------------------
-------------------
-------------------
-------------------
-------------------
local function topic_backgroundGuild()
    ib:SetHeader("st_002", npc)
    ib:SetMsg("As part of the ^Thraal^ army corps, we are reclaiming this " ..
              "plane after the defeat of Moroch.")

    if gstat == game.GUILD_IN and
       questnr > 0 then
        ib:SetLHSButton("Quest")
        ib:SetRHSButton("Services")
    else
        ib:SetLHSButton("Services")
    end
end

-------------------
-------------------
local function topic_backgroundMembers(subject)
    ib:SetHeader("st_002", npc)

    ---------
    -- Player is not a guild member.
    ---------
    if gstat ~= game.GUILD_IN then
        ib:SetMsg("I don't discuss guildmembers with civilians.")
        ib:SetLHSButton("Services")
    ---------
    -- Player is a guild member.
    ---------
    else
        ---------
        -- "Chereth"
        ---------
        if subject == "chereth" then
            ib:SetMsg("Former Archery Commander Chereth is now our Supply " ..
                      "Chief.\n")
            ib:AddMsg("\nAlthough she was seriously wounded in battle she " ..
                      "is still a great expert in archery.")
        ---------
        -- "Jahrlen"
        ---------
        elseif subject == "jahrlen" then
            ib:SetMsg("Jahrlen is our guild mage.\n")
            ib:AddMsg("\nOf course, normally we don't have a guild mage, " ..
                      "since we are mercenaries. But we are at war here " ..
                      "and he was assigned to us, so we treat him as one " ..
                      "of our own.\n")
            ib:AddMsg("\nIn fact he is a high level chronomancer and we " ..
                      "are honored to have him help us.\n")
            ib:AddMsg("\nHe is normally downstairs in the strategy room.")
--        ---------
--        -- "Taleus"
--        ---------
--        elseif subject == "taleus" then
        ---------
        -- Catch all
        ---------
        else
            ib:SetMsg("Ah, " .. string.capitalize(subject) .. "! A fine " ..
                      "guildmember.")
        end

        if questnr > 0 then
            ib:SetLHSButton("Quest")
            ib:SetRHSButton("Services")
        else
            ib:SetLHSButton("Services")
        end
    end
end

---------------------------------------
-- QUEST: A standard topic.
---------------------------------------
-------------------
-- This handles general quest enquiries.
-------------------
local function topic_quest()
    ib:SetHeader("st_003", npc)

    ---------
    -- Player has completed all quests.
    ---------
    if questnr == 0 then
        ib:SetMsg("I have nothing more for you.\n")
        ib:AddMsg("\nThe guild values experience. Go and get some.")
    ---------
    -- Next quest is disallowed (ie, player does not meet level/skill req).
    ---------
    elseif questnr < 0 then
        ib:SetMsg("I have nothing for you at the moment.\n")
        ib:AddMsg("\nThe guild values experience. Come back later.")
    ---------
    -- Player is eligible for a quest.
    ---------
    else
        ib:SetTitle(qb:GetName(questnr))

        ---------
        -- "The Mercenary Guild Quest"
        ---------
        if questnr == 1 then
            ---------
            -- Player has not yet accepted the quest.
            ---------
            if qb:GetStatus(1) == game.QSTAT_NO then
                ib:SetMsg("Before you can join the guild I have a small " ..
                          "task for you.\n")
                ib:AddMsg("\nWe have had a problem with giant ants for " ..
                          "some time now. See this hole to the old guild " ..
                          "cellar beside me? One of those silly ants down " ..
                          "there has stolen my old helmet!\n")
                ib:AddMsg("\nEnter the hole and kill the ants! Don't " ..
                          "worry, they are weak.\n")
                ib:AddMsg("\nBring back the helmet and you can join the " ..
                          "guild.\n")
                ib:AddMsg("\nThere are four rooms in the cellar. I think the " ..
                          "Ant Scout Leader in the fourth room has my helmet.")
                ---------
                -- Lets give a hint to the newbs.
                ---------
                if player.level <= 2 then
                    ib:AddMsg("\n\n|Hint -- Close Combat|\n")
                    ib:AddMsg("\nHm, you don't look too experienced with " ..
                              "your weapon... Well, here is a tip from an " ..
                              "old wardog.\n")
                    ib:AddMsg("\nWhen you get down there, try fighting a " ..
                              "few ants in the first three rooms before " ..
                              "going for the Scout Leader. They won't " ..
                              "attack you until you attack them.\n")
                    ib:AddMsg("\nWhen you're ready, go for the Scout " ..
                              "Leader. He is a lot more aggressive and " ..
                              "will chase you on sight. Don't show him any " ..
                              "fear!\n")
                    ib:AddMsg("\nIf you run away like a coward he'll have " ..
                              "time to recover from his wounds, so it is " ..
                              "important that you keep on the offensive.\n")
                    ib:AddMsg("\nOh, and I wouldn't try going any deeper " ..
                              "into the nest just yet...")

                    if player:GetQuest("Find Fanrir's Lunch") then
                        ib:AddMsg("\n\nDon't forget what Fanrir told you " ..
                                  "about eating food to recover health.")
                    end
                end

                ib:SetAccept("Accept", "accept quest")
            ---------
            -- Quest is active/solved.
            ---------
            else
                ib:SetMsg("Have you retrieved my old helm yet?")
                ib:AddLink("Yes, I slew the ant and recovered your helm.",
                           "quest complete")
                ib:AddLink("No, I have not done it yet.", "quest incomplete")
            end
        end
    end
end

-------------------
-- topic_questAccept() handles player accepting a quest.
-- Note that it is allowed to accept a quest even if it has not been offered
-- yet.
-- This is by design (all right, also it just worked out that way ;)) as it
-- enables players who have been through it all before (for example they're
-- playing with an alt) to simply cut to the chase with '/talk accept quest'.
-------------------
local function topic_questAccept()
    ib:SetHeader("st_003", npc)

    ---------
    -- Player has completed all quests or next quest is disallowed (ie, player
    -- does not meet level/skill req).
    ---------
    if questnr <= 0 then
        return topic_quest()
    end

    ---------
    -- Player is in the middle of a quest already.
    ---------
    if qb:GetStatus(questnr) ~= game.QSTAT_NO then
        ib:SetMsg("Whoah! Slow down there. You haven't even told me how " ..
                  "you got on with the previous ^quest^.")
    ---------
    -- Player is eligible for a new quest.
    ---------
    else
        ib:SetTitle(qb:GetName(questnr))
        ---------
        -- "The Mercenary Guild Quest"
        ---------
        if questnr == 1 then
            ib:SetDesc("Enter the hole in the floor near Cashin in the " ..
                       "Guild Hall Mercenary Guild.\n")
            ib:AddDesc("\nSlay the Ant Scout Leader and retrieve Cashin's " ..
                       "helm.")
            qb:RegisterQuest(1, npc, ib)
        end
    end
end

-------------------
-- topic_questDecline() handles player declining a quest.
-- Note that it is allowed to decline a quest even if it has not been offered
-- yet.
-- This is by design (all right, also it just worked out that way ;)) as it
-- enables players who have been through it all before (for example they're
-- playing with an alt) to simply cut to the chase with '/talk decline quest'.
-------------------
local function topic_questDecline()
    ib:SetHeader("st_003", npc)

    ---------
    -- Player has completed all quests or next quest is disallowed (ie, player
    -- does not meet level/skill req).
    ---------
    if questnr <= 0 then
        return topic_quest()
    end

    ---------
    -- Player is in the middle of a quest already.
    ---------
    if qb:GetStatus(questnr) ~= game.QSTAT_NO then
        ib:SetMsg("Lets take this one step at a time, shall we?")
    ---------
    -- Player is eligible for a new quest.
    ---------
    else
        ib:SetMsg("|** " .. npc.name .. " glares at you without saying a " ..
                  "word for a long time **|\n")
        ib:AddMsg("\nDeclining a Guild quest... I see.")
    end
end

-------------------
-- topic_questComplete() handles player claiming to have solved the current
-- quest.
-- Remember that just because player says something doesn't make it so.
-- Always check the quest status.
-------------------
local function topic_questComplete()
    ib:SetHeader("st_003", npc)

    ---------
    -- Player has completed all quests or next quest is disallowed (ie, player
    -- does not meet level/skill req).
    ---------
    if questnr <= 0 then
        return topic_quest()
    end

    ---------
    -- Player lied!
    ---------
    if qb:GetStatus(questnr) ~= game.QSTAT_SOLVED then
        ib:SetMsg("|** " .. npc.name .. " looks you up and down **|\n")

        ---------
        -- Quest not even accepted yet.
        ---------
        if qb:GetStatus(questnr) == game.QSTAT_NO then
            ib:AddMsg("\nI fail to see how you've managed that as I have " ..
                      "yet to give you the ^quest^.")
        ---------
        -- Quest is active.
        ---------
        elseif qb:GetStatus(questnr) == game.QSTAT_ACTIVE then
            ib:AddMsg("\n")
            qb:AddItemList(questnr, ib)
            ib:AddMsg("\nNo, you have yet to do everything I asked of you.")
        end
    ---------
    -- Quest really is complete.
    ---------
    else
        ib:SetTitle(qb:GetName(questnr))
        ---------
        -- "The Mercenary Guild Quest"
        ---------
        if questnr == 1 then
            ib:SetMsg("Very well done! As promised, I will make you a " ..
                      "member of the ~Mercenary Guild~.\n")
            ib:AddMsg("\n|** " .. npc.name .. " enters your name in the " ..
                      "member roll **|\n")
            ib:AddMsg("\nWelcome, Guildmember " .. player.name .. "!\n")
            ib:AddMsg("\n|** " .. npc.name .. " looks fondly at his helmet " ..
                      "for a moment **|\n")
            ib:AddMsg("\nTell you what. You should also take this. You've " ..
                      "earned it.")
            ib:AddIcon("Membership of the ~Mercenary Guild~!", "guild_merc",
                       "", 0)
            ib:AddIcon("Cashin's old helm", "helm_leather.101",
                       "It will protect you well, as it did me.")
            qb:Finish(1, "Cashin's old helm")
        end
    end
end

-------------------
-- topic_questIncomplete() handles player claiming to have not solved the
-- current quest.
-- Remember that just because player says something doesn't make it so.
-- Always check the quest status.
-------------------
local function topic_questIncomplete()
    ib:SetHeader("st_003", npc)

    ---------
    -- Player has completed all quests or next quest is disallowed (ie, player
    -- does not meet level/skill req).
    ---------
    if questnr <= 0 then
        return topic_quest()
    end

    ---------
    -- Player lied!
    ---------
    if qb:GetStatus(questnr) ~= game.QSTAT_ACTIVE then
        ib:SetMsg("|** " .. npc.name .. " looks at you quizzically **|\n")

        ---------
        -- Quest not even accepted yet.
        ---------
        if qb:GetStatus(questnr) == game.QSTAT_NO then
            ib:AddMsg("\nI fail to see how you've managed that as I have " ..
                      "yet to give you the ^quest^.")
        ---------
        -- Quest is solved.
        ---------
        elseif qb:GetStatus(questnr) == game.QSTAT_SOLVED then
            ib:SetTitle(qb:GetName(questnr))
            ib:AddMsg("\n")
            qb:AddItemList(questnr, ib)
            ib:AddMsg("\nUm, you might want to rethink your answer...")
        end
    ---------
    -- Player is really as useless as he claims...
    ---------
    else
        ib:SetTitle(qb:GetName(questnr))
        ib:SetMsg("Never mind, I am patient.")
    end
end

---------------------------------------
-- RUMOURS: A standard topic.
---------------------------------------
-------------------
-- topic_rumours() handles this standard topic.
-------------------
local function topic_rumours()
    ib:SetHeader("st_004", npc)
    ib:SetMsg("~TODO~")

    if gstat == game.GUILD_IN and
       questnr > 0 then
        ib:SetLHSButton("Quest")
        ib:SetRHSButton("Services")
    else
        ib:SetLHSButton("Services")
    end
end

---------------------------------------
-- SERVICES: A standard topic.
---------------------------------------
-------------------
-- topic_services() handles this standard topic.
-------------------
local function topic_services()
    ib:SetHeader("st_005", npc)
    ib:SetMsg("As Guildmaster, I'm in charge of membership.")

    if gstat == game.GUILD_NO then
        ib:AddLink("Join the guild", "join")
    elseif gstat == game.GUILD_OLD then
        ib:AddLink("Rejoin the guild", "rejoin")
    else
        ib:AddLink("Leave the guild", "leave")
    end

    if questnr > 0 then
        ib:SetLHSButton("Quest")
    end
end

-------------------
-------------------
local function topic_servicesJoin()
    ib:SetHeader("st_005", npc)

    ---------
    -- Player is not a guildmember and never has been (means he can not have
    -- done the first quest).
    ---------
    if gstat == game.GUILD_NO then
        ib:SetMsg("So you want to join the guild?\n")
        ib:AddMsg("\n|** " .. npc.name .. " gives you the once over " ..
                  "again **|\n")
        ib:AddMsg("\nWell with the ogre problem here in the Province and " ..
                  "Moroch causing untold trouble all over, a person of " ..
                  "your apparent calibre would be more than welcome.\n")
        ib:AddMsg("\nBut first I need you to prove your mettle by " ..
                  "completing a simple ^task^.")
        ib:SetLHSButton("Quest")
        ib:SetRHSButton("Services")
    else
        ---------
        -- Player used to be a guildmember but left.
        ---------
        if gstat == game.GUILD_OLD then
            ib:SetMsg("|** " .. npc.name .. " frowns, but then his face " ..
                      "relaxes **|\n")
            ib:AddMsg("\nWell times are tough and good mercenaries don't " ..
                      "grow on trees, so no benefit in recriminations.\n")
            ib:AddMsg("\nWelcome back, Guildmember " .. player.name .. "!")
            module_guildsJoin("Mercenary", player)
        ---------
        -- Player is currently a guildmember.
        ---------
        else
            ib:SetMsg("|** " .. npc.name .. " looks down the member roll " ..
                      "**|\n")
            ib:AddMsg("\nErr... I admire your enthusiasm but you already " ..
                      "joined the guild.")
        end

        if questnr > 0 then
            ib:SetLHSButton("Quest")
            ib:SetRHSButton("Services")
        else
            ib:SetLHSButton("Services")
        end
    end
end

-------------------
-------------------
local function topic_servicesLeave()
    ib:SetHeader("st_005", npc)
    ib:SetMsg("|** " .. npc.name .. " looks down the member roll **|\n")

    if gstat ~= game.GUILD_IN then
        ib:AddMsg("\nAs you aren't a guildmember, I can't say I'm sorry " ..
                  "to lose you.")
    elseif module_guildsLeave("Mercenary", player) then
        ib:AddMsg("\nSorry to lose you, Guildmember " .. player.name .. ".")
    end

    if questnr > 0 then
        ib:SetLHSButton("Quest")
        ib:SetRHSButton("Services")
    else
        ib:SetLHSButton("Services")
    end
end

---------------------------------------
---------------------------------------
require("topic_list")

local tl = TopicList()

tl:SetDefault(topic_default)
tl:AddGreeting(nil, topic_greeting)
tl:AddBackground(nil, topic_background)
tl:AddTopics({ "guild", "mercenary'?s?", "mercenaries'?",
               "mercenary'?s? guild", "mercenaries'? guild" },
              topic_backgroundGuild)
tl:AddTopics({ "(chereth)", "(jahrlen)", "(taleus)" }, topic_backgroundMembers)
tl:AddQuest({ "task" }, topic_quest)
tl:AddTopics("accept quest", topic_questAccept)
tl:AddTopics("decline quest", topic_questDecline)
tl:AddTopics("quest complete", topic_questComplete)
tl:AddTopics("quest incomplete", topic_questIncomplete)
tl:AddRumours(nil, topic_rumours)
tl:AddServices(nil, topic_services)
tl:AddTopics({ "join", "rejoin", "join guild", "rejoin guild" },
             topic_servicesJoin)
tl:AddTopics({ "leave", "leave guild"}, topic_servicesLeave)
ib:ShowSENTInce(game.GUI_NPC_MODE_NPC, tl:CheckMessage(event, true))
