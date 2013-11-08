-------------------------------------------------------------------------------
-- npc.lua | Module
--
-- This module provides a table, npc, within the Module table. Within
-- Module.npc are several other elements which interact with each other to
-- create a structure and behaviours for a player /talking to an NPC.
--
-- The most basic use of the module is to require it from an NPC's (eg, a
-- friendly mob) TALK Event script. Before this the Event script must assign
-- two variables; player points to the GameObject of the player and npc points
-- to the GameObject of the NPC. Finally call the Module.npc.ShowInterface
-- function.
--
-- So the simplest possible TALK Event script which uses this module is:
--     require("module/npc")
--     ScriptContext:GetModule("npc").ShowInterface()
--
-- You will now have a fully SENTInce-compliant NPC. He'll be a little dull of
-- course, having no individual personality, offering no quests or services,
-- and basically nothing to justify his existence.
-------------------------------------------------------------------------------
---------------------------------------
--All modules require a script context, so ensure the Utility is loaded.
---------------------------------------
require("script_context")

---------------------------------------
-- If an npc module already exists in this script context, assume we have
-- already included this file, so bail out.
---------------------------------------
local m = ScriptContext:GetModule("npc")
if m ~= nil then
    local a, b, c = type(m)
    assert(a == "addon" and
        b == "module" and
        c == "npc",
        "Module 'npc' exists in script context but is not addon module npc!")
    return
end

---------------------------------------
-- Required Utilities.
--
-- We also create local variables pointing to the Utility tables so we can
-- refer to them by the familiar shorthand. Remember, locals are only visible
-- within the chunk (file) and within or below the block in which they were
-- declared. So these declarations do not extend outside this file.
---------------------------------------
-------------------
-------------------
local ib = ScriptContext:GetUtility("ib")
if ib == nil then
    require("interface_builder")
    ib = ScriptContext:AddUtility("ib", InterfaceBuilder())
end
if ib ~= nil then
    local a, b, c = type(ib)
    assert(a == "addon" and
        b == "utility" and
        c == "ib",
        "Utility 'ib' exists in script context but is not addon utility ib!")
end

-------------------
-------------------
local qb = ScriptContext:GetUtility("qb")
if qb == nil then
    require("quest_builder")
    qb = ScriptContext:AddUtility("qb", QuestBuilder())
end
if qb ~= nil then
    local a, b, c = type(qb)
    assert(a == "addon" and
        b == "utility" and
        c == "qb",
        "Utility 'qb' exists in script context but is not addon utility qb!")
end

-------------------
-------------------
local tl = ScriptContext:GetUtility("tl")
if tl == nil then
    require("topic_list")
    tl = ScriptContext:AddUtility("tl", TopicList())
end
if tl ~= nil then
    local a, b, c = type(tl)
    assert(a == "addon" and
        b == "utility" and
        c == "tl",
        "Utility 'tl' exists in script context but is not addon utility tl!")
end

---------------------------------------
-- Required GameObjects.
--
-- For each GameObject, first check if it is in the script context. If not,
-- check a default fallback in the event table. If this is a GameObject, add
-- it to the script context. If, by the end of the process, we have not got an
-- appropriate GameObject into the script context -- which is also pointed to
-- by a local variable -- generate an error. .
---------------------------------------
-------------------
-------------------
local player = ScriptContext:GetObject("player")
if player == nil and
    type(event.activator) == "GameObject" then
    player = ScriptContext:AddObject("player", event.activator)
end
if player == nil then
    error("No player could be found!")
end

-------------------
-------------------
local npc = ScriptContext:GetObject("npc")
if npc == nil and
    type(event.me) == "GameObject" then
    npc = ScriptContext:AddObject("npc", event.me)
end
if npc == nil then
    error("No npc could be found!")
end

---------------------------------------
-- Assign the local m to a table representing this module. Give it a metatable.
-- The __metatable metamethod identifies it as an addon module.
--
-- Then add it to the script context, reassigning m to point to the module
-- within the context.
---------------------------------------
m = {
    topic = {}, replace = {}, extend = {}, background = { subtopic = {} },
    quest = {}, rumours = { subtopic = {} }
}
setmetatable(m, {
    __metatable = function() return "addon", "module", "npc" end
})
m = ScriptContext:AddModule("npc", m)

---------------------------------------
-- The m.ShowInterface function opens an interface.
---------------------------------------
function m.ShowInterface()
    ib:ShowSENTInce(game.GUI_NPC_MODE_NPC, tl:CheckMessage(event, true))
end

---------------------------------------
-- The m:AddTopics method adds subtopics for the specified topic. The
-- argument topic must be a string of a standard topic. Only 'background' or
-- 'rumours' are allowed. The argument trigger must be a string or table of
-- strings as in tl:AddTopics(). These strings must be enclosed in parentheses
-- (ie, a capture).
---------------------------------------
function m:AddTopics(topic, trigger)
    local action
    if topic == "background" then
        action = self.topic.Background
    elseif topic == "rumours" then
        action = self.topic.Rumours
    else
        error("Unrecognised topic: " .. tostring(topic) .. "!")
    end
    tl:AddTopics(trigger, action)
    if type(trigger) ~= "table" then
        trigger = { trigger }
    end
    for _, v in ipairs(trigger) do
        table.insert(self[topic].subtopic, v)
    end
end

---------------------------------------
-- m.topic contains functions which handle the NPC's response to
-- topics which the player /talks to him.
--
-- Each function in m.topic first checks if a same name function
-- exists in m.replace. If so, this function is called. If not,
-- default behaviour for topic is done (often just invoking a few ib methods to
-- build a simple interface, but it can be much more complicated).
--
-- Then -- whether a replace was called or the default was run -- there is a
-- check for a same name function in m.extend. If so, this function is
-- called.
--
-- SENTInce specifies 5 so-called standard topics. These are topics to which
-- *all* NPCs should have a response; the response needn't be detailed -- it
-- may be a simple 'I have nothing to say about that topic' -- but the NPC must
-- know what is being said to him.
--
-- The standard topics are: greeting, background, quest, rumours, and services.
-- A couple of standard topics also have standardized subtopics. These are
-- described below.
--
-- Finally, there is also a default topic for when the NPC does not understand
-- what is being said to him.
---------------------------------------
-------------------
-- The m.topic.Greeting function handles the greeting standard topic.
-- The required default phrases to trigger this standard topic are:
-- 'greetings', 'hello', 'hey', and 'hi'.
--
-- This is a player's standard entrance point to a conversation with the NPC.
-------------------
function m.topic.Greeting()
    local replace = m.replace.Greeting
    if type(replace) == "function" then
        replace()
    else
        local profession = m.profession
        local prefix = m.rank
        if prefix == nil then
            prefix = profession
        end
        if type(prefix) == "string" then
            prefix = string.capitalize(prefix)
        end
        ib:SetHeader("st_001", npc, prefix)
        ib:SetMsg("Hello! My name is " .. npc:GetName() .. ".")
        if profession ~= nil then
            local article = "a"
            if string.match(profession, "^[aeiouAEIOU]") ~= nil then
                article = "an"
            end
            ib:AddMsg(" I am " .. article .. " " .. profession .. ".")
        end
        local qnr = qb:GetQuestNr(true)
        if qnr > 0 then
            if qb:GetStatus(qnr) == game.QSTAT_NO then
                ib:AddMsg("\n\nI have a ^quest^ you may be interested in.")
            else
                ib:AddMsg("\n\nHow's progress on that ^quest^?")
                ib:AddLink("I've done what you asked.", "quest complete")
                ib:AddLink("Slow. But I'm working on it.", "quest incomplete")
            end
            ib:SetLHSButton("Quest")
        end
    end
    local extend = m.extend.Greeting
    if type(extend) == "function" then
        extend()
    end
end

-------------------
-- The m.topic.Background function handles the background standard
-- topic. The required default phrases to trigger this standard topic are:
-- 'background', 'you', 'yourself', and the NPC's name.
--
-- The NPC talks about himself.
-------------------
function m.topic.Background(subject)
    local replace = m.replace.Background
    if type(replace) == "function" then
        replace(subject)
    else
        local prefix = m.rank
        if prefix == nil then
            prefix = m.profession
        end
        if type(prefix) == "string" then
            prefix = string.capitalize(prefix)
        end
        ib:SetHeader("st_002", npc, prefix)
        local handler = m.background.Topic, r
        if type(handler) == "function" then
            r = handler(subject)
        end
        for _, v in ipairs(m.rumours.subtopic) do
            if v == subject then
                r = true
                break
            end
        end
        if r == nil then
            ib:SetMsg("What you see is what you get with me.")
        end
        if qb:GetQuestNr(true) > 0 then
            ib:SetLHSButton("Quest")
        end
    end
    local extend = m.extend.Background
    if type(extend) == "function" then
        extend(subject)
    end
end

-------------------
-- The m.topic.Quest function handles the quest standard topic. The
-- required default phrases to trigger this standard topic are: 'quest', and
-- 'quests'.
--
-- The NPC offers a new quest if he has one for which the player is eligible,
-- prompts the player for a progress report on the current quest if he (the
-- player) is already on one, or makes his excuses if he has no (further)
-- quests (for which the player is eligible).
--
-- A SENTInce-compliant NPC must also recognise four standardized subtopics:
-- accept quest, decline quest, quest complete, and quest incomplete; the
-- triggering phrases for these subtopics are the words themselves. To prevent
-- confusion, the subtopics must be handled even if the NPC does not offer any
-- quests; by default they simply redirect to the quest standard topic.
-------------------
function m.topic.Quest()
    local replace = m.replace.Quest
    if type(replace) == "function" then
        replace()
    else
        local prefix = m.rank
        if prefix == nil then
            prefix = m.profession
        end
        if type(prefix) == "string" then
            prefix = string.capitalize(prefix)
        end
        ib:SetHeader("st_003", npc, prefix)
        local qnr = qb:GetQuestNr(true)
        local handler = m.quest.Offer, r
        if type(handler) == "function" then
            r = handler(qnr)
        end
        if r == nil then
            if qnr == 0 then
                ib:SetMsg("Sorry, I have no quests to offer you.")
            elseif qnr < 0 then
                ib:SetMsg("I have nothing for you at the moment.")
            else
                ib:SetMsg("~TODO~")
            end
        end
        if qnr ~= 0 then
            ib:SetTitle(qb:GetName(qnr))
            if qb:GetStatus(qnr) == game.QSTAT_NO then
                ib:SetAccept("Accept", "accept quest")
            else
                ib:AddMsg("\n\nHow's progress?")
                ib:AddLink("I've done what you asked.", "quest complete")
                ib:AddLink("Slow. But I'm working on it.",
                    "quest incomplete")
            end
        end
        end
    end
    local extend = m.extend.Quest
    if type(extend) == "function" then
        extend()
    end
end

-------------------
-- The m.topic.QuestAccept function handles the accept quest subtopic.
-- The required phrase to trigger this subtopic is: 'accept quest'.
--
-- Note that it is allowed to accept a quest even if it has not been offered
-- yet. This enables players who have been through it all before (for example,
-- they're playing with an alt) to simply cut to the chase with '/talk accept
-- quest'.
-------------------
function m.topic.QuestAccept()
    local replace = m.replace.QuestAccept
    if type(replace) == "function" then
        replace()
    else
        local prefix = m.rank
        if prefix == nil then
            prefix = m.profession
        end
        if type(prefix) == "string" then
            prefix = string.capitalize(prefix)
        end
        ib:SetHeader("st_003", npc, prefix)
        local qnr = qb:GetQuestNr(true)
        if qnr <= 0 then
            return m.topic.Quest()
        else
            if qb:GetStatus(qnr) ~= game.QSTAT_NO then
                ib:SetMsg("Whoah! Slow down there. You haven't even told me " ..
                    "how you got on with the current ^quest^.")
            else
                local handler = m.quest.Accept, r
                if type(handler) == "function" then
                    r = handler(qnr)
                end
                if r == nil then
                    ib:SetMsg("~TODO~")
                end
                ib:SetTitle(qb:GetName(qnr))
                if qb:IsRegistered(qnr) == false then
                    qb:RegisterQuest(qnr, npc, ib)
               end
            end
        end
    end
    local extend = m.extend.QuestAccept
    if type(extend) == "function" then
        extend()
    end
end

-------------------
-- The m.topic.QuestDecline function handles the decline quest
-- subtopic. The required phrase to trigger this subtopic is: 'decline quest'.
--
-- Note that it is allowed to decline a quest even if it has not been offered
-- yet. This enables players who have been through it all before (for example,
-- they're playing with an alt) to simply cut to the chase with '/talk decline
-- quest'.
-------------------
function m.topic.QuestDecline()
    local replace = m.replace.QuestDecline
    if type(replace) == "function" then
        replace()
    else
        local prefix = m.rank
        if prefix == nil then
            prefix = m.profession
        end
        if type(prefix) == "string" then
            prefix = string.capitalize(prefix)
        end
        ib:SetHeader("st_003", npc, prefix)
        local qnr = qb:GetQuestNr(true)
        if qnr <= 0 then
            return m.topic.Quest()
        else
            if qb:GetStatus(qnr) ~= game.QSTAT_NO then
                ib:SetMsg("Whoah! Slow down there. You haven't even told me " ..
                    "how you got on with the current ^quest^.\n\n")
                ib:AddMsg("|[Quests can be skipped via the Quest Log " ..
                    "(press ~Q~ when not talking to an NPC).]|")
            else
                local handler = m.quest.Decline, r
                if type(handler) == "function" then
                    r = handler(qnr)
                end
                if r == nil then
                    ib:SetMsg("That's a shame. Oh well, your choice.")
                end
            end
        end
    end
    local extend = m.extend.QuestDecline
    if type(extend) == "function" then
        extend()
    end
end

-------------------
-- The m.topic.QuestComplete function handles the quest complete
-- subtopic. The required phrase to trigger this subtopic is: 'quest complete'.
-- 
-- Remember that just because the player says something doesn't make it so.
-- Always check the quest status.
-------------------
function m.topic.QuestComplete(reward)
    local replace = m.replace.QuestComplete
    if type(replace) == "function" then
        replace(reward)
    else
        local prefix = m.rank
        if prefix == nil then
            prefix = m.profession
        end
        if type(prefix) == "string" then
            prefix = string.capitalize(prefix)
        end
        ib:SetHeader("st_003", npc, prefix)
        local qnr = qb:GetQuestNr(true)
        if qnr <= 0 then
            return m.topic.Quest()
        end
        if qb:GetStatus(qnr) ~= game.QSTAT_SOLVED then
            ib:SetMsg("|** " .. npc:GetName() .. " looks you up and " ..
                "down **|\n\n") 
            if qb:GetStatus(qnr) == game.QSTAT_NO then
                ib:AddMsg("I fail to see how you've managed that as I have " ..
                    "yet to give you the ^quest^.")
            elseif qb:GetStatus(qnr) == game.QSTAT_ACTIVE then
                qb:AddItemList(qnr, ib)
                ib:AddMsg("\n\n")
                ib:AddMsg("No, you have yet to do everything I asked of " ..
                    "you.")
            end
        else
            local handler = m.quest.Complete, r
            if type(handler) == "function" then
                r = handler(qnr, reward)
            end
            ib:SetTitle(qb:GetName(qnr))
            if r ~= false then
                if r == nil then
                    ib:SetMsg("~TODO~")
                elseif type(r) ~= "string" then
                    r = nil
                end
                qb:Finish(qnr, r)
                if qb:GetQuestNr(false) > qnr then
                    ib:SetLHSButton("Quest")
                end
            end
        end
    end
    local extend = m.extend.QuestComplete
    if type(extend) == "function" then
        extend(reward)
    end
end

-------------------
-- The m.topic.QuestIncomplete function handles the quest incomplete
-- subtopic. The required phrase to trigger this subtopic is: 'quest
-- incomplete'.
-- 
-- Remember that just because the player says something doesn't make it so.
-- Always check the quest status.
-------------------
function m.topic.QuestIncomplete()
    local replace = m.replace.QuestIncomplete
    if type(replace) == "function" then
        replace()
    else
        local prefix = m.rank
        if prefix == nil then
            prefix = m.profession
        end
        if type(prefix) == "string" then
            prefix = string.capitalize(prefix)
        end
        ib:SetHeader("st_003", npc, prefix)
        local qnr = qb:GetQuestNr(true)
        if qnr <= 0 then
            return m.topic.Quest()
        end
        if qb:GetStatus(qnr) ~= game.QSTAT_ACTIVE then
            ib:SetMsg("|** " .. npc.name .. " looks at you quizzically " ..
                "**|\n\n")
            if qb:GetStatus(qnr) == game.QSTAT_NO then
                ib:AddMsg("I fail to see how you've managed that as I have " ..
                    "yet to give you the ^quest^.")
            elseif qb:GetStatus(qnr) == game.QSTAT_SOLVED then
                ib:SetTitle(qb:GetName(qnr))
                qb:AddItemList(qnr, ib)
                ib:AddMsg("\n\n")
                ib:AddMsg("Um, you might want to rethink your answer...")
            end
        else
            local handler = m.quest.Incomplete, r
            if type(handler) == "function" then
                r = handler(qnr)
            end
            ib:SetTitle(qb:GetName(qnr))
            if r == nil then
                ib:SetMsg("Never mind, I am patient.")
            end
        end
    end
    local extend = m.extend.QuestIncomplete
    if type(extend) == "function" then
        extend()
    end
end

-------------------
-- The m.topic.Rumours function handles the rumours standard topic.
-- The required default phrases to trigger this standard topic are: 'rumor',
-- 'rumors', 'rumour', and 'rumours'.
--
-- The NPC talks about gossip he has heard.
--
-- The details of the rumours system are still TODO but essentially each NPC
-- will inherit a file of rumours based on his locality (ie, town or other area
-- where he resides). Each triggering of this topic prints out one random
-- rumour (eg, gossip about another NPC, feature, or legend in the locality, a
-- hint about where a quest may be obtained or how a quest might be solved,
-- etc); these rumours may or may not be truthful, useful, etc. Each NPC can
-- also add to, or override completely, these local rumours with his own
-- personal stock.
-------------------
function m.topic.Rumours(subject)
    local replace = m.replace.Rumours
    if type(replace) == "function" then
        replace()
    else
        local prefix = m.rank
        if prefix == nil then
            prefix = m.profession
        end
        if type(prefix) == "string" then
            prefix = string.capitalize(prefix)
        end
        ib:SetHeader("st_004", npc, prefix)
        local handler = m.rumours.Topic, r
        if type(handler) == "function" then
            r = handler(subject)
        end
        for _, v in ipairs(m.background.subtopic) do
            if v == subject then
                r = true
                break
            end
        end
        if r == nil then
            ib:SetMsg(Locality.SpreadRumour(Module))
        end
        if qb:GetQuestNr(true) > 0 then
            ib:SetLHSButton("Quest")
        end
    end
    local extend = m.extend.Rumours
    if type(extend) == "function" then
        extend()
    end
end

-------------------
-- The m.topic.Services function handles the services standard topic.
-- The required default phrases to trigger this standard topic are: 'service',
-- and 'services'.
--
-- The NPC lists any services which he offers. A service might be a task which
-- he can perform (such as identifying goods or opening a bank account), a
-- spell which he can cast (such as remove death sickness), goods which he can
-- supply (such as booze or an apartment), etc. Usually this is for a fee but
-- theirs no reason why services should not be in return for a quest being
-- completed or other recompense, or even for free.
-------------------
function m.topic.Services()
    local replace = m.replace.Services
    if type(replace) == "function" then
        replace()
    else
        local prefix = m.rank
        if prefix == nil then
            prefix = m.profession
        end
        if type(prefix) == "string" then
            prefix = string.capitalize(prefix)
        end
        ib:SetHeader("st_005", npc, prefix)
        ib:SetMsg("Sorry, I have no services to offer you.")
        if qb:GetQuestNr(true) > 0 then
            ib:SetLHSButton("Quest")
        end
    end
    local extend = m.extend.Services
    if type(extend) == "function" then
        extend()
    end
end

-------------------
-- m.topic.Default() handles unrecognised phrases, giving a nice
-- helpful 'try different words'-type response.
--
-- Remember, the topic may be perfectly reasonable but just something this NPC
-- does not specifically respond to, too complex to parse (perhaps there is a
-- simpler way to say the same thing), or complete gobbledigook (maybe the
-- player did a typo). The default response should be valid in all three cases.
-------------------
function m.topic.Default()
    local replace = m.replace.Default
    if type(replace) == "function" then
        replace()
    else
        local prefix = m.rank
        if prefix == nil then
            prefix = m.profession
        end
        if type(prefix) == "string" then
            prefix = string.capitalize(prefix)
        end
        ib:SetHeader(npc, npc, prefix)
        ib:SetMsg("I'm sorry. I didn't understand.\n\n")
        ib:AddMsg("|[The interface's parsing is extremely limited, so keep " ..
            "your input simple. Usually type only a noun (name of " ..
            "something), such as 'quest', or sometimes a verb (an action " ..
            "to do) and a noun, such as 'repair sword', to talk about a " ..
            "subject. But it may also be that this particular NPC simply " ..
            "does not know anything about that topic.]|")
    end
    local extend = m.extend.Default
    if type(extend) == "function" then
        extend()
    end
end

---------------------------------------
-- m.tl contains the default phrase list to trigger the above topics.
---------------------------------------
tl:AddGreeting(nil, m.topic.Greeting)
tl:AddBackground(string.lower(npc.name), m.topic.Background)
tl:AddQuest(nil, m.topic.Quest)
tl:AddTopics("accept quest", m.topic.QuestAccept)
tl:AddTopics("decline quest", m.topic.QuestDecline)
tl:AddTopics({ "quest complete", "quest complete (.+)" },
    m.topic.QuestComplete)
tl:AddTopics("quest incomplete", m.topic.QuestIncomplete)
tl:AddRumours(nil, m.topic.Rumours)
tl:AddServices(nil, m.topic.Services)
tl:SetDefault(m.topic.Default)
