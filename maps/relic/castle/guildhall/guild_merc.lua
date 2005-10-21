require("topic_list")

local activator = event.activator
local me = event.me
local guild_tag = "Mercenary"
local guild_rank = ""
local quest_arch_name = "quest_object"
local quest_item_name = "Cashin's leather cap"
local quest_id = "merc guild-1"
local guild_force = activator:GetGuildForce()
local trigger = nil
local quest_item = nil
local pinfo = nil
local quest_body = [[<m t="Return Cashim's Helm" b="Return Cashim's helm and he will let join you
the mecenery guild.

An ant from the old cellar has stolen the helm.

Enter the sewer hole next Cashim and search the cellar for the ant." >
<r t="Return Cashim's Helm" b="- bring Cashim his helm back" copper="20">
<i m="G" f="guild_merc" t="Mercenary guild membership" b="Mercenary guild membership">]]

-- status of guild/quest: 1= we are member, 2= no guild&quest, 3= old member, 4=we can join, 5=quest given but unsolved
function gstatus()
if guild_force.slaying == guild_tag then
return 1
else
-- check we are old member
pinfo = activator:GetPlayerInfo("GUILD_INFO")
while pinfo ~= nil do
-- we are old member. adjust ranks
if pinfo.slaying == guild_tag then
guild_rank = pinfo.title
if guild_rank == nil then
guild_rank = ""
end
return 3
end
pinfo = activator:GetNextPlayerInfo(pinfo)
end
-- we check first our quest_object is there and has been triggered
trigger = activator:CheckQuest(quest_id)
if trigger == nil then
return 2
-- for some reason we have the quest done but no guild info?!
else
quest_item = activator:CheckInventory(1,quest_arch_name, quest_item_name)
if trigger.last_eat == -1 then
return 4
elseif trigger.magic == 0 or  quest_item == nil then
return 5
end
return 4
end
end
end

function status_msg(sta)
if sta == 1 then
s_msg = [[Good to see you back, Mercenary!

If you look for a job or a mission,
talk to ^Jahrlen^ and his stuff.

They often had tasks and quests for us mercs.]]
elseif sta == 3 then
s_msg = [[Good to see you back!

You want rejoin us?
We would be honored to have you back in our ^guild^ ranks.]]
elseif sta == 2 then
s_msg =[[You are new here around, right?

If you look for a job and a guild...
Then you are right here!

I can tell you some about our ^guild^ if you like.]]
elseif sta == 4 then
s_msg = [[Good to see you back!

You have my helm?
Then let me see.

We would be honored to have you in our ^guild^ ranks.]]
elseif sta == 5 then
s_msg = [[Good to see you back!

You have my helm?
Remember: The ant in the cellar has it!
Enter the sewer hole here next to me and
search for it.

We would be honored to have you in our ^guild^ ranks.]]
end
return s_msg
end

function topicDefault()
local status = gstatus()
if status == 1 or status == 5 then
m_join =" "
else
m_join = [[<l t="Join the mercenary guild" c="/talk join">]]
end

activator:Interface(1, [[<h f="ranger.151">
<m t="Hello ]] .. activator.name .. [[!" b="We are the Mercenary Guild of Thraal.
Welcome to our guild halls!

]].. status_msg(status) ..[[">

]] .. m_join)
end

function become_member(sta)
if sta == 4 then
activator:Interface(1, [[<h f="ranger.151"><m t="The Mercenary Guild"
b="You got my helm! Great!

Well, you can keep it.
It will protect you in your coming fights.

Now, do you want join the mercenary guild?" >
<r t="Join the Mercenary Guild" b="Your guild change to Mercenary" copper="20">
<i m="G" f="guild_merc" t="Mercenary guild membership" b="Mercenary guild membership">
<i m="G" f="helm_leather.101" t="Cashim's leather helm" b="Cashim's old helmet">
<a c="/talk member"><d c="/talk hi">]])
elseif sta == 3 then
activator:Interface(1, [[<h f="ranger.151"><m t="The Mercenary Guild"
b="You are an old member of ours!

You can rejoin at any time.
You will get your old guild ranks back too." >
<r t="Rejoin the Mercenary Guild" b="Your guild change back to Mercenary">
<i m="G" f="guild_merc" t="Mercenary guild membership" b="Mercenary guild membership">
<a c="/talk member"><d c="/talk hi">]])
else
activator:Interface(-1, NULL)
end
end

tl = TopicList()
tl:SetDefault(topicDefault)

tl:AddTopics("join", function()
local status = gstatus()
local j_msg =""
if status == 2 then
activator:Interface(1, [[<h f="ranger.151">]]..quest_body..[[<a c="/talk take"><d c="/talk hi">]])
elseif status == 3 then
become_member(status)
elseif status == 4 then
become_member(status)
elseif status == 5 then
activator:Interface(1, [[<h f="ranger.151"><m t="Join the Mercenary Guild"
b="Where is my helm?

Remember what i told you.

An ant from the old cellar has stolen the helm.

Bring me the helm and you are in.
Just enter the sewer hole next to me." >]])
else
topicDefault()
end
end
)

tl:AddTopics("take", function()
activator:Sound(0, 0, 2, 0)
activator:AddQuest(quest_id, 0, 0, quest_body)
activator:Write("You take the quest 'Return Cashim's Helm'.", game.COLOR_NAVY)
activator:Interface(-1, "")
end
)

tl:AddTopics("member", function()
local status = gstatus()
if status == 4 then
-- setup a own PLAYER_INFO (name GUILD_INFO)
guild_info = activator:CreatePlayerInfo("GUILD_INFO")
guild_info.title = guild_rank -- thats our title in this guild
guild_info.slaying = guild_tag -- thats the guild_info tag of this guild
activator:SetGuildForce(guild_rank)  -- Our active guild, give us our profession title
guild_force.slaying = guild_tag -- thats the real tag to the guild_info
if quest_item ~= nil then
quest_item:DecreaseNrOf()
local rew = activator:CreateObjectInside("helm_leather", 1,1)
rew.name = quest_item_name
activator:Write("You got Cashim's leather helm.", game.COLOR_WHITE)
end
activator:AddMoney(20,0,0,0)
trigger:SetQuestStatus(-1)
activator:Write("You got 20 copper.", game.COLOR_WHITE)
activator:Write("Your guild change to Mercenary of Thraal.", game.COLOR_NAVY)
activator:Sound(0, 0, 0, 0)
elseif status == 3 then
activator:SetGuildForce(guild_rank)  -- Our active guild, give us our profession title
guild_force.slaying = guild_tag -- thats the real tag to the guild_info
activator:Write("You rejoined the Mercenary of Thraal.", game.COLOR_WHITE)
activator:Write("Your guild change to Mercenary of Thraal.", game.COLOR_NAVY)
activator:Sound(0, 0, 0, 0)
end
topicDefault()
end
)

tl:AddTopics("guild", function()
activator:Interface(1, [[<h f="ranger.151"><m t="The Mercenary Guild" b=
"Explain about the guild. more text.
keyword: ^Jahrlen^ and ^troops^"><a t="Next" c="/talk hi">]])
end
)

tl:AddTopics("jahrlen", function()
activator:Interface(1, [[<h f="ranger.151"><m t="Jahrlen, the Chronomancer" b=
"Jahrlen is our guild mage.

Well, normally we don't have a guild mage.
But we are at war here and he was assigned to us.

In fact, he is a high level chronomancer
and we are honored he helps us.

He is in our guild rooms down the stairs there!
Talk to him when you meet him.

He often has tasks and quests for us mercenaries."><a t="Next" c="/talk hi">]])
end
)

tl:AddTopics("troops", function()
activator:Interface(1, [[<h f="ranger.151"><m t="The Thraal Army" b=
"We, as part of the the Thraal army corps, are invading
these abandomed areas after the defeat of Moroch.

Well, the chronomancers ensured us after they created
the portal that we are still in the galactic main sphere.

But it seems to me that these lands have many wormholes
to other places...

Perhaps the long time under Morochs influence has weakened
the borders between the planes.

You should ask ^Jahrlen^ about it."><a t="Next" c="/talk hi">]])
end
)

tl:AddTopics("leave", function()
if guild_force.slaying ~= guild_tag then
me:SayTo(activator, "\nYou are not a member here...")
else
me:SayTo(activator, [[
Ok, you are out!
You can rejoin any time.]])

guild_force = activator:SetGuildForce("") -- "" forces the routine to insert a NULL in the obj...
guild_force.slaying = ""
end
topicDefault()
end
)

tl:CheckMessage(event)
