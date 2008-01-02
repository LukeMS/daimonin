-- merc_chereth.lua

require("topic_list")
require("interface_builder")

local pl = event.activator
local me = event.me

local ib = InterfaceBuilder()
ib:SetHeader(me, me.name)

-- Guild checks
local guild_tag = "Mercenary"
local guild_stat = game.GUILD_NO
local guild_force = nil
guild_force = pl:GetGuild(guild_tag)
if guild_force ~= nil then
    guild_stat = guild_force.sub_type_1
end

-- Archery skill checks
local skills    = {"bow archery", "crossbow archery", "sling archery"}
local skill_bow = game:GetSkillNr(skills[1])
local skill_cbow = game:GetSkillNr(skills[2])
local skill_sling = game:GetSkillNr(skills[3])
local pl_skill = -1
if pl:FindSkill(skill_bow) ~= nil then
    pl_skill = 1
elseif pl:FindSkill(skill_cbow) ~= nil then
    pl_skill = 2
elseif pl:FindSkill(skill_sling) ~= nil then
    pl_skill = 3
end

local function topicDefault()
    local join = "join"
    ib:SetTitle("Greetings!")
    ib:SetMsg("\n\nWelcome to the Mercenary guild.")
    if guild_stat ~= game.GUILD_IN then
        if guild_stat == game.GUILD_OLD then
            join = "rejoin"
        end
        ib:AddMsg("\n\nAs you are not a member, you should talk to ^Cashin^ upstairs first. ")
        ib:AddMsg("He will tell you how you can "..join..".\n\n")
        ib:AddMsg("Then come back and I will have more for you.")
    else
        ib:AddMsg("\n\nI am Chereth, and I used to teach archery here before I was blinded in a fight with some devilish creatures.\n\n")
        ib:AddMsg("Now ^Taleus^ teaches those skills.")
        if pl_skill > 0 then
            ib:AddMsg("\n\nBut you don't need me to tell you that. I can see that you are ")
            ib:AddMsg("skilled in "..skills[pl_skill].." already.")
        end
        ib:AddMsg("\n\nThen of course there is my dear friend ^Jahrlen^, across the room there.")
    end
    pl:Interface(1, ib:Build())
end

local function topCashin()
    ib:SetTitle("Cashin")
    ib:SetMsg("\n\nCashin is the Guild Master.\n\n")
    if guild_stat == game.GUILD_NO then
        ib:AddMsg("If you take his quest, he will grant you membership.")
    else
        ib:AddMsg("He will probably let you rejoin our guild if you ask him.")
    end
    ib:SetButton("Back", "hi")
    pl:Interface(1, ib:Build())
end

local function topTaleus()
    ib:SetTitle("Taleus")
    ib:SetMsg("\n\nTaleus is our Archery Commander now. He took over archery training for me after I lost my sight.")
    ib:AddMsg("\n\nHe has become an expert in all of the skills, and will be an excellent teacher for you.")
    ib:AddMsg("\n\nHe is usually practicing on our archery ranges outside, but if he has ")
    ib:AddMsg("gone off for more supplies someone at the range will surely know where you can find him.") 
    ib:SetButton("Back", "hi")
    pl:Interface(1, ib:Build())
end

local function topJahrlen()
    ib:SetTitle("Jahrlen")
    ib:SetMsg("\n\nAh yes, dear old Jahrlen. If you looked at him you wouldn't guess he is over 200 years old, would you?")
    ib:AddMsg("\n\nHe has a highly developed sense of humour, which is why we were laughing when you ")
    ib:AddMsg("came down the stairs.")
    ib:AddMsg("\n\nHe is our resident mage, and will probably be able to teach you some useful magic if you talk to him.")
    ib:SetButton("Back", "hi")
    pl:Interface(1, ib:Build())
end

local tl = TopicList()
tl:AddGreeting(nil, topicDefault)
tl:SetDefault(topicDefault)
tl:AddTopics("cashin", topCashin)
tl:AddTopics("taleus", topTaleus)
tl:AddTopics("jahrlen", topJahrlen)
tl:CheckMessage(event)
 
