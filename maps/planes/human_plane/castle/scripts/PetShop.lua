require "topic_list"

local activator = event.activator
local me = event.me
local msg = event.message

local tl = TopicList:New()

local function buyfunc(what)
local goods = 
{
["red ant"] = { 
price=10000, 
arch="ant_red",
name="pet ant",
level=5
},
["black ant"] = { 
price=50000, 
arch="ant_soldier",
name="pet ant",
level=7
},
["doggie"] = { 
price=200000, 
arch="dog_brown",
name="pet doggie",
level=13
}
}

-- Look up goods name
local pet_info = goods[what]
if pet_info == nil then
activator:Interface(1,
[[        
<h f="peasant.151"> 
<m t="Pet Shop" b="Sorry, but I didn't quite get that.">
<a t="Ok" c="/talk hello">
]]
)
return
end

-- check money
if activator:GetMoney() < pet_info.price then
activator:Interface(1,
[[        
<h f="peasant.151"> 
<m t="Pet Shop" b="Sorry, but you can't afford that pet.">
<a t="Ok" c="/talk hello">
]]
)
return
end        

-- Create new pet
local pet = me.map:CreateObject(pet_info.arch, 8, 20)
pet.name = pet_info.name
pet.level = pet_info.level
if not pet:MakePet(activator) then
activator:Interface(1,
[[        
<h f="peasant.151"> 
<m t="Pet Shop" b="Sorry, but you can't have any more pets.">
<a t="Ok" c="/talk hello">
]]
)
return
end        

-- take money
if activator:PayAmount(pet_info.price) == 0 then
me:SayTo(activator, "Uh. Something went very wrong...")
pet:DecreaseNrOf() -- Remove the pet again
return
end

activator:Write("You give the money to " .. me.name)

-- Make shop owner go and fetch the new pet
me:SayTo(activator, "Please wait a moment while I fetch your new pet for you")
local waypoint 
for obj in obj_inventory(me) do
if obj.name == "waypoint_pets" then
waypoint = obj
end
end
waypoint.f_cursed = true

-- Finish off
activator:Interface(-1)
end

tl:SetDefault( function() activator:Interface(1,
[[
<h f="peasant.151"> 
<m t="Pet Shop" b="Welcome to my humble shop. 
Here you can buy yourself a loving servant from the best of breeds.
I've got ^red^ and ^black^ ants for sale, or you can try out this 
adorable little ^doggie^--the tyke!

I can also offer you some general ^advice^ on pet handling.
"> 
]]
) end
)

tl:AddTopics( {"doggie", "dog"}, function() activator:Interface(1,
[[
<h f="peasant.151"> 
<m t="Doggie" b="
Ah, so you picked the little doggie!
Such a cute fella, I'm sure you'll like him. 
He takes a lot of love and makes a great companion. 
Pay ~20 gold~ and he's yours.

Do you want to buy this cute li'l doggie?
"> 
<i m="G" f="dog.ise.111" t="This li'l doggie needs lots of love!" b="blaha">
<a t="Accept" c="/talk buy doggie">
<d t="Decline" c="/talk hello">
]]
) end
)

tl:AddTopics( {"red", "red ant"}, function() activator:Interface(1,
[[
<h f="peasant.151"> 
<m t="Red Ant" b="
A good choice for a young active person such as yourself. 
The red ant is loyal and requires little care. 
It will be yours for only ~1 gold~.

Do you want to buy this?
"> 
<i m="G" f="ant_red.131" t="This red pet ant wants a new home" b="blaha">
<a t="Accept" c="/talk buy red ant">
<d t="Decline" c="/talk hello">
]]
) end
)

tl:AddTopics( {"black", "black ant"}, function() activator:Interface(1,
[[
<h f="peasant.151"> 
<m t="Black Ant" b="
Now, that is an excellent fit for you. 
A black ant will protect you and help hunt down your enemies. 
It is yours for only ~5 gold~.

Would you like to buy this?
"> 
<i m="G" f="ant_soldier.131" t="This black pet ant wants a new home" b="blaha">
<a t="Accept" c="/talk buy black ant">
<d t="Decline" c="/talk hello">
]]
) end
)

tl:AddTopics( {"advice", "pets"}, function() activator:Interface(1,
[[
<h f="peasant.151"> 
<m t="About Pets" b="
Pets are loyal companions that follow you around and help you out 
in your adventures.

If you get in a fight, they will try to help you out by attacking 
your targets. They will also defend themselves if attacked.

Remember that pets are not always easy to control. They have a 
mind of their own and can even rebel upon you if you abuse them.
Make sure to take care of your pet and heal it when needed.">
<a t="Ok" c="/talk hello">
]]
) end
)

tl:AddTopics("buy (.*)", buyfunc)

tl:AddTopics("leave", function() activator:Interface(-1) end)

tl:CheckMessage(event)