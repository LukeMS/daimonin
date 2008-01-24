require("topic_list");
require("interface_builder")

local pl 		= event.activator
local me        = event.me
local msg       = string.lower(event.message)

local ib = InterfaceBuilder()
ib:SetHeader(me, me.name)

local function topicDefault()
    ib:SetTitle("Add Skills to Player")
    ib:AddMsg("I am the Skillgiver.\nType ^learn <skillname>^ or ^unlearn <skillname>^ (Note: unlearn skills is not implemented atm - the ideas of the daimonin skill system is to collect the skills - and don't lose them).")
    pl:Interface(1, ib:Build())
end

tl = TopicList()

tl:AddTopics("learn (.*)",
    function(skillname)
        skill = game:GetSkillNr(skillname)
        if skill == -1 then
            me:SayTo(pl,"Unknown skill \""..skillname.."\"" )
        else
            if pl:FindSkill(skill) ~= nil then
                me:SayTo(pl,"You already learned this skill." )
            else
                pl:AcquireSkill(skill, game.LEARN)
            end
        end
        topicDefault()
    end
)
tl:AddGreeting(nil, topicDefault)
tl:SetDefault(topicDefault)

tl:CheckMessage(event)
