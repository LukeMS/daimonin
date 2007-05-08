require("topic_list")
require("interface_builder")

local pl = event.activator
local me = event.me
local msg = string.lower(event.message)

local ib = InterfaceBuilder()
ib:SetHeader(me, me.name)

local sum
local spell

-- Note: "cause light wounds" is now taught through a quest in the village (Endar)
--       so teaching for payment is removed here. The code is however left in,
--       as a prototype should teaching other spells for payment needs to be added later.
--       Just the link is commented out.
--
--       Well, Cleo (who else?) found an exploit. So the clw code is now removed.

local function topicDefault()
    ib:SetTitle("The Church of the Tabernacle")
    ib:SetMsg("Hello! I am " .. me.name ..".\n\nWelcome to the church of the Tabernacle!\n\n")
    ib:AddMsg("If you are confused by my services, I can ^explain^ it.\nYou need some of our services?")
    ib:AddLink("Teach Minor Healing", "teach healing")
    ib:AddLink("", "")
    ib:AddLink("Remove Death Sickness", "cast sick")
    ib:AddLink("Remove Depletion", "cast deplete")
    ib:AddLink("Restoration", "cast restore")
    ib:AddLink("Cure Disease", "cast disease")
    ib:AddLink("Cure Poison", "cast poison")
    ib:AddLink("Remove Curse from items", "cast curse")
    ib:AddLink("Remove Damnation from items", "cast damn")
    pl:Interface(1, ib:Build())
end

local function topicExplain()
    ib:SetTitle("About our Services")
    ib:SetMsg("\n\nI can teach you basic spells that you can cast yourself.\n\n")
    ib:AddMsg("I can also cure various things by casting the named spells on you when you pay me the money.\n\n")
    ib:AddMsg("Deathsick is a stronger form of depletion.\nEverytime you die stats are depleted by death sickness.")
    ib:SetButton("Back", "hi")
    pl:Interface(1, ib:Build())
end

local function topicCast(what)
    if what=="sick" then
        ib:SetMsg("I can cast °Remove Deathsick° for " .. pl:ShowCost(100 + (4 * pl.level * pl.level)))
    elseif what == "deplete" then
        ib:SetMsg("I can cast °Remove Depletion° for ".. pl:ShowCost(5 * pl.level))
    elseif what == "restore" then
        ib:SetMsg("I can cast °Restoration° for 150 copper")
    elseif what == "poison" then
        ib:SetMsg("I will cast °Cure Poison° for " .. pl:ShowCost(5 * pl.level))
    elseif what == "disease" then
        ib:SetMsg("I can cast °Cure Disease° for ".. pl:ShowCost(100 * pl.level))
    elseif what == "curse" then
        ib:SetMsg("I can cast °Remove Curse° for ".. pl:ShowCost(100 * pl.level))
    else
        ib:SetMsg("I can cast °Remove Damnation° for " .. pl:ShowCost(100 + (3 * pl.level * pl.level)))
    end
    ib:AddMsg(".\n\nYou have " .. pl:ShowCost(pl:GetMoney()) .. ".\n\nDo you want me to do it now?") 
    ib:SetAccept(nil, "docast " .. what) 
    ib:SetDecline(nil, "hi")
    pl:Interface(1, ib:Build())
end

local function topicDoCast(what)
    if what == "sick" then
        sum = 100 + (4 * pl.level * pl.level)
        spell = "remove death sickness"
    elseif what == "deplete" then
        sum = 5 * pl.level
        spell = "remove depletion"
    elseif what == "restore" then
        sum = 150
        spell = "restoration"
    elseif what == "poison" then
        sum = 5 * pl.level
        spell = "cure poison"
    elseif what == "disease" then
        sum = 100 * pl.level
        spell = "cure disease"
    elseif what == "curse" then
        sum = 100 * pl.level
        spell = "remove curse"
    else
        sum = 100 + (3 * pl.level * pl.level)
        spell = "remove damnation"
    end
    ib:SetTitle("Casting " .. spell)
    if pl:PayAmount(sum) == 1 then
        me:CastSpell(pl, game:GetSpellNr(spell), 1, 0, "")
        ib:SetMsg("°** " .. me.name .. " takes your money **°\n\ndone!")
    else
        ib:SetMsg("You don't have enough money!")
    end
    ib:SetButton("Back", "Hi") 
    pl:Interface(1, ib:Build())
end

local function topicTeachHealing()
    ib:SetMsg("I can teach you °Minor Healing° for no charge.")
    ib:AddMsg("\n\nDo you want me to do it now?") 
    ib:SetAccept(nil, "doteach healing") 
    ib:SetDecline(nil, "hi")
    pl:Interface(1, ib:Build())
end

local function topicDoTeachHealing()
    spell = "minor healing"
    local spellno = game:GetSpellNr(spell)
    if pl:DoKnowSpell(spellno) then
        ib:SetMsg("You already know the spell '"..spell.."'!" )
    else
        ib:SetTitle("Teaching "..spell)
        local skill = game:GetSkillNr("divine prayers")
        if pl:FindSkill(skill) == nil then
            ib:AddMsg("First I will teach you the skill 'divine prayers'\nYou need this skill to cast prayer spells.\n\n")
            pl:AcquireSkill(skill, game.LEARN)
        end
        pl:AcquireSpell(spellno, game.LEARN)
        ib:AddMsg("You learn the spell '"..spell.."'")
    end
    ib:SetButton("Back", "Hi") 
    pl:Interface(1, ib:Build())
end

tl = TopicList()
tl:AddGreeting(nil, topicDefault)
tl:SetDefault(topicDefault)
tl:AddTopics("explain", topicExplain)
tl:AddTopics("cast (.*)", topicCast)
tl:AddTopics("docast (.*)", topicDoCast)
tl:AddTopics("teach healing", topicTeachHealing)
tl:AddTopics("doteach healing", topicDoTeachHealing)
tl:CheckMessage(event)
