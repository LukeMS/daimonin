require("topic_list")
require("interface_builder")

local pl = event.activator
local me = event.me

sglow_app_tag = "SGLOW_APP_INFO"
appid_cheap ="cheap"
appid_normal ="normal"
appid_expensive ="expensive"
appid_luxurious ="luxurious"
ap_1 = "/stoneglow/appartment_1"
ap_2 = "/stoneglow/appartment_2"
ap_3 = "/stoneglow/appartment_3"
ap_4 = "/stoneglow/appartment_4"

pinfo = pl:GetPlayerInfo(sglow_app_tag)

local ib = InterfaceBuilder()
ib:SetHeader(me, me.name)

function topicGreeting()
ib:SetTitle("At the apartments")
ib:AddMsg("Welcome to the apartment house.\n")
ib:AddMsg("I can sell you an ^apartment^.\n")
ib:AddMsg("I have ^cheap^, ^normal^, ^expensive^ or ^luxurious^ ones.")
pl:Interface(1, ib:Build())
end

function updateAp(ap_old, ap_new, pid, x, y)
ib:SetTitle("Upgrading finished")
ib:AddMsg("You pay the money.\n", 0)
ib:AddMsg("~Darlin is casting some strange magic.~\n")
map_old = game:ReadyMap(ap_old, 2, pl)
map_new = game:ReadyMap(ap_new, 6, pl)
if map_old == nil or map_new == nil then
ib:AddMsg("Something is wrong... Call a DM!\n");
else
game:TransferMapItems(map_old, map_new, x, y)
map_new:Save()
pinfo.slaying = pid
--TODO: get a safer version to delete apts
--game:FileUnlink(map_old.path)
map_old:Delete(1)
ib:AddMsg("\nDone! Your new apartment is ready.\n")
end
pl:Interface(1, ib:Build())
end

function reset()
if pinfo ~= nil then
pinfo.last_heal = -1
end
end

function topicApartment()
ib:SetTitle("About apartments")
ib:AddMsg("An apartment is a kind of unique place you can buy.\n")
ib:AddMsg("Only you can enter it!\n")
ib:AddMsg("You can safely store or drop your items there.\n")
ib:AddMsg("They will not vanish over time:\n")
ib:AddMsg("If you leave the game they will be still there\n")
ib:AddMsg("when you come back later.\n")
ib:AddMsg("Apartments have different size and styles.\n")
ib:AddMsg("You can have only one apartment at once in the city,\n")
ib:AddMsg("but you can ^upgrade^ it.")
ib:SetButton("Back", "hi")
pl:Interface(1, ib:Build())
reset()
end

function topicCheap()
ib:SetTitle("Cheap Apartment")
ib:AddMsg("The cheap apartment will cost you 30 silver.\n")
ib:AddMsg("It has only a bed and a chest.\n")
ib:AddMsg("Every apartment is a kind of ^pocket dimension^.\n")
ib:AddMsg("You can enter it by using the teleporter there.\n")
ib:AddMsg("Say ^sell me a cheap apartment^ to buy it!\n")
ib:AddMsg("Choose wisely!")
ib:SetButton("Back", "hi")
pl:Interface(1, ib:Build())
reset()
end

function topicNormal()
ib:SetTitle("Normal Apartment")
ib:AddMsg("The normal apartment will cost you 250 silver.\n")
ib:AddMsg("It has some storing devices and some furniture.\n")
ib:AddMsg("Every apartment is a kind of ^pocket dimension^.\n")
ib:AddMsg("You can enter it by using the teleporter there.\n")
ib:AddMsg("Say ^sell me a normal apartment^ to buy it!\n")
ib:AddMsg("Choose wisely!")
ib:SetButton("Back", "hi")
pl:Interface(1, ib:Build())
reset()
end

function topicExpensive()
ib:SetTitle("Expensive Apartment")
ib:AddMsg("The expensive apartment will cost you 15 gold.\n")
ib:AddMsg("It is large for a single apartment and has many places to store items including a nice bedroom.\n")
ib:AddMsg("Every apartment is a kind of ^pocket dimension^.\n")
ib:AddMsg("You can enter it by using the teleporter there.\n")
ib:AddMsg("Say ^sell me an expensive apartment^ to buy it!\n")
ib:AddMsg("Choose wisely!")
ib:SetButton("Back", "hi")
pl:Interface(1, ib:Build())
reset()
end

function topicLux()
ib:SetTitle("Luxurious Apartment")
ib:AddMsg("The luxurious apartment will cost you 200 gold.\n")
ib:AddMsg("It is very large for a single apartment and has a lot of places to store items including a nice bedroom.\n")
ib:AddMsg("Every apartment is a kind of ^pocket dimension^.\n")
ib:AddMsg("You can enter it by using the teleporter there.\n")
ib:AddMsg("Say ^sell me an luxurious apartment^ to buy it!\n")
ib:AddMsg("Choose wisely!")
ib:SetButton("Back", "hi")
pl:Interface(1, ib:Build())
reset()
end

function topicDimension()
ib:SetTitle("About apartment dimension")
ib:AddMsg("A pocket dimension is a magical mini dimension")
ib:AddMsg("in an outer plane. They are very safe and no thief will ever be able to enter.")
ib:SetButton("Back", "hi")
pl:Interface(1, ib:Build())
reset()
end

function topicUpgrade()
ib:SetTitle("About upgrading apartment")

if pinfo == nil then
ib:AddMsg("You don't have any apartment to upgrade.\n")
else
pinfo.last_heal = -1
ib:AddMsg("Apartment upgrading will work like this then\n")
ib:AddMsg("a.) Choose your new home in the upgrade procedure.\n")
ib:AddMsg("b.) You get *no* money back for your old apartment.\n")
ib:AddMsg("c.) All items in your old apartment are *automatically* transfered, including items in containers. They appear in a big pile in your new apartment.\n")
ib:AddMsg("d.) Your old apartment is exchanged with your new one.\n")
ib:AddMsg("Upgrading will also work to change an expensive apartment to a cheap one.\n")
ib:AddMsg("Go to upgrade ^procedure^ when you want upgrade now.\n")
end
ib:SetButton("Back", "hi")
pl:Interface(1, ib:Build())
end

function topicProcedure()
ib:SetTitle("Upgrade procedure")
if pinfo == nil then
ib:AddMsg("\nYou don't have any apartment to upgrade.\n")
else
pinfo.last_heal = -1
ib:AddMsg("After you choose i will ask you to confirm.\n\n")

if pinfo.slaying == appid_cheap then
ib:AddMsg("Your current home here is a cheap apartment.\n")
ib:AddMsg("You can upgrade it to:")
ib:AddLink("normal apartment","upgrade to normal apartment")
ib:AddLink("expensive apartment","upgrade to expensive apartment")
ib:AddLink("luxurious apartment","upgrade to luxurious apartment")

elseif pinfo.slaying == appid_normal then
ib:AddMsg("Your current home here is a normal apartment.\n")
ib:AddMsg("You can upgrade it to:")
ib:AddLink("cheap apartment","upgrade to cheap apartment")
ib:AddLink("expensive apartment","upgrade to expensive apartment")
ib:AddLink("luxurious apartment","upgrade to luxurious apartment")

elseif(pinfo.slaying == appid_expensive) then
ib:AddMsg("Your current home here is a expensive apartment.\n")
ib:AddMsg("You can upgrade it to:")
ib:AddLink("cheap apartment","upgrade to cheap apartment")
ib:AddLink("normal apartment","upgrade to normal apartment")
ib:AddLink("luxurious apartment","upgrade to luxurious apartment")

else
ib:AddMsg("Your current home here is a luxurious apartment.\n")
ib:AddMsg("You can upgrade it to:")
ib:AddLink("cheap apartment","upgrade to cheap apartment")
ib:AddLink("normal apartment","upgrade to normal apartment")
ib:AddLink("expensive apartment","upgrade to expensive apartment")

end
end
ib:SetButton("Back", "upgrade")
pl:Interface(1, ib:Build())
end

function topicConfirm()
ib:SetTitle("Confirm your choice!")
if pinfo.last_heal==1 then
ib:AddMsg("You have chosen to upgrade to �cheap apartment�.\nIt will cost you �30 silver!�\n")
end
if pinfo.last_heal==2 then
ib:AddMsg("You have chosen to upgrade to �normal apartment�.\nIt will cost you �250 silver!�\n")
end
if pinfo.last_heal==3 then
ib:AddMsg("You have chosen to upgrade to �expensive apartment�.\nIt will cost you �15 gold!�\n")
end
if pinfo.last_heal==4 then
ib:AddMsg("You have chosen to upgrade to �luxurious apartment�.\nIt will cost you �200 gold!�\n")
end
ib:AddMsg([[
Be sure it is the correct choice, then:
Choose �Acccept� button again and it will be done.
Choose �Decline� to cancel it.]])

ib:SetAccept(nil, "upgrade_confirmation2")
ib:SetDecline(nil, "hi")

pl:Interface(1, ib:Build())
end

function topicConfirmAgain()
ib:SetTitle("Upgrade results")
if pinfo == nil then
ib:AddMsg("You don't have any apartment to upgrade.\n")
else
if pinfo.last_heal <= 0 then
ib:AddMsg("Go first to the upgrade ^procedure^ before you confirm your choice.\n")
else
if pinfo.slaying == appid_cheap then
old_ap = ap_1
elseif pinfo.slaying == appid_normal then
old_ap = ap_2
elseif pinfo.slaying == appid_expensive then
old_ap = ap_3
elseif pinfo.slaying == appid_luxurious then
old_ap = ap_4
end
if pinfo.last_heal == 1 then
if pl:PayAmount(3000) ~= 1 then
ib:AddMsg("Sorry, you don't have enough money!\n");
else
updateAp(old_ap, ap_1, appid_cheap, 1, 2)
end
elseif pinfo.last_heal == 2 then
if pl:PayAmount(25000) ~= 1 then
ib:AddMsg("Sorry, you don't have enough money!\n");
else
updateAp(old_ap, ap_2, appid_normal, 1, 2)
end
elseif pinfo.last_heal == 3 then
if pl:PayAmount(150000) ~= 1 then
ib:AddMsg("Sorry, you don't have enough money!\n");
else
updateAp(old_ap, ap_3, appid_expensive, 1, 2)
end
elseif pinfo.last_heal == 4 then
if pl:PayAmount(2000000) ~= 1 then
ib:AddMsg("Sorry, you don't have enough money!\n");
else
updateAp(old_ap, ap_4, appid_luxurious, 2, 1)
end
end
end
pinfo.last_heal = -1
end
pl:Interface(1, ib:Build())
end

function topicSellCheap()
ib:SetTitle("Apartment sale results")
if pinfo == nil then -- no app - all ok
if pl:PayAmount(3000) == 1 then
ib:AddMsg("You pay the money.\n")
pinfo = pl:CreatePlayerInfo(sglow_app_tag)
pinfo.slaying = appid_cheap -- thats the apartment type info
ib:AddMsg("Darlin is casting some strange magic.\n")
ib:AddMsg("Congratulations! That was all!\n");
ib:AddMsg("I have summoned your apartment right now.\n")
ib:AddMsg("Enter the teleporter and you will be there!\n")
ib:AddMsg("Have a good day.\n")
else
ib:AddMsg("Sorry, you don't have enough money!\n")
end
else
pinfo.last_heal = -1
ib:AddMsg("You have bought an apartment in the past here.\n")
ib:AddMsg("You can ^upgrade^ it.\n")
end
ib:SetButton("Back", "hi")
pl:Interface(1, ib:Build())
end

function topicSellNormal()
ib:SetTitle("Apartment sale results")
if pinfo == nil then -- no app - all ok
if pl:PayAmount(25000) == 1 then
ib:AddMsg("You pay the money.\n")
pinfo = pl:CreatePlayerInfo(sglow_app_tag)
pinfo.slaying = appid_normal -- thats the apartment type info
ib:AddMsg("Darlin is casting some strange magic.\n")
ib:AddMsg("Congratulations! That was all!\n");
ib:AddMsg("I have summoned your apartment right now.\n")
ib:AddMsg("Enter the teleporter and you will be there!\n")
ib:AddMsg("Have a good day.\n")
else
ib:AddMsg("Sorry, you don't have enough money!\n")
end
else
pinfo.last_heal = -1
ib:AddMsg("You have bought an apartment in the past here.\n")
ib:AddMsg("You can ^upgrade^ it.\n")
end
ib:SetButton("Back", "hi")
pl:Interface(1, ib:Build())
end

function topicSellExpensive()
ib:SetTitle("Apartment sale results")
if pinfo == nil then -- no app - all ok
if pl:PayAmount(150000) == 1 then
ib:AddMsg("You pay the money.\n")
pinfo = pl:CreatePlayerInfo(sglow_app_tag)
pinfo.slaying = appid_expensive -- thats the apartment type info
ib:AddMsg("Darlin is casting some strange magic.\n")
ib:AddMsg("Congratulations! That was all!\n");
ib:AddMsg("I have summoned your apartment right now.\n")
ib:AddMsg("Enter the teleporter and you will be there!\n")
ib:AddMsg("Have a good day.\n")
else
ib:AddMsg("Sorry, you don't have enough money!\n")
end
else
pinfo.last_heal = -1
ib:AddMsg("You have bought an apartment in the past here.\n")
ib:AddMsg("You can ^upgrade^ it.\n")
end
ib:SetButton("Back", "hi")
pl:Interface(1, ib:Build())
end

function topicSellLuxurious()
ib:SetTitle("Apartment sale results")
if pinfo == nil then -- no app - all ok
if pl:PayAmount(2000000) == 1 then
ib:AddMsg("You pay the money.\n")
pinfo = pl:CreatePlayerInfo(sglow_app_tag)
pinfo.slaying = appid_luxurious -- thats the apartment type info
ib:AddMsg("Darlin is casting some strange magic.\n")
ib:AddMsg("Congratulations! That was all!\n");
ib:AddMsg("I have summoned your apartment right now.\n")
ib:AddMsg("Enter the teleporter and you will be there!\n")
ib:AddMsg("Have a good day.\n")
else
ib:AddMsg("Sorry, you don't have enough money!\n")
end
else
pinfo.last_heal = -1
ib:AddMsg("You have bought an apartment in the past here.\n")
ib:AddMsg("You can ^upgrade^ it.\n")
end
ib:SetButton("Back", "hi")
pl:Interface(1, ib:Build())
end

function topicUpgradeToCheap()
ib:SetTitle("Upgrade to cheap")
if pinfo == nil then
ib:AddMsg("You don't have any apartment to upgrade.")
ib:SetButton("Back", "hi")
else
if(pinfo.slaying == appid_cheap) then
pinfo.last_heal = -1
ib:AddMsg("You can't upgrade to the same size.")
ib:SetButton("Back", "upgrade")
else
pinfo.last_heal = 1
ib:AddMsg([[
The cheap apartment will cost you 30 silver.
NOW LISTEN then to do the upgrade you must confirm it.
You really want upgrade to a cheap apartment now?
Choose 'Acccept' button and it will be done.
Choose 'Decline' to cancel it.]])
ib:SetAccept(nil, "upgrade_confirmation1")
ib:SetDecline(nil, "hi")
end
end
pl:Interface(1, ib:Build())
end

function topicUpgradeToNormal()
ib:SetTitle("Upgrade to normal")
if pinfo == nil then
ib:AddMsg("You don't have any apartment to upgrade.")
ib:SetButton("Back", "hi")
else
if(pinfo.slaying == appid_normal) then
pinfo.last_heal = -1
ib:AddMsg("You can't upgrade to the same size.")
ib:SetButton("Back", "upgrade")
else
pinfo.last_heal = 2
ib:AddMsg([[
The cheap apartment will cost you 250 silver.
NOW LISTEN then to do the upgrade you must confirm it.
You really want upgrade to a normal apartment now?
Choose 'Acccept' button and it will be done.
Choose 'Decline' to cancel it.]])
ib:SetAccept(nil, "upgrade_confirmation1")
ib:SetDecline(nil, "hi")
end
end
pl:Interface(1, ib:Build())
end

function topicUpgradeToExpensive()
ib:SetTitle("Upgrade to expensive")
if pinfo == nil then
ib:AddMsg("You don't have any apartment to upgrade.")
ib:SetButton("Back", "hi")
else
if(pinfo.slaying == appid_expensive) then
pinfo.last_heal = -1
ib:AddMsg("You can't upgrade to the same size.")
ib:SetButton("Back", "upgrade")
else
pinfo.last_heal = 3
ib:AddMsg([[
The cheap apartment will cost you 15 gold.
NOW LISTEN then to do the upgrade you must confirm it.
You really want upgrade to a expensive apartment now?
Choose 'Acccept' button and it will be done.
Choose 'Decline' to cancel it.]])
ib:SetAccept(nil, "upgrade_confirmation1")
ib:SetDecline(nil, "hi")
end
end
pl:Interface(1, ib:Build())
end

function topicUpgradeToLuxurious()
ib:SetTitle("Upgrade to luxurious")
if pinfo == nil then
ib:AddMsg("You don't have any apartment to upgrade.")
ib:SetButton("Back", "hi")
else
if(pinfo.slaying == appid_luxurious) then
pinfo.last_heal = -1
ib:AddMsg("You can't upgrade to the same size.")
ib:SetButton("Back", "upgrade")
else
pinfo.last_heal = 4
ib:AddMsg([[
The cheap apartment will cost you 200 gold.
NOW LISTEN then to do the upgrade you must confirm it.
You really want upgrade to a luxurious apartment now?
Choose 'Acccept' button and it will be done.
Choose 'Decline' to cancel it.]])
ib:SetAccept(nil, "upgrade_confirmation1")
ib:SetDecline(nil, "hi")
end
end
pl:Interface(1, ib:Build())
end


tl = TopicList()

tl:AddTopics("upgrade to cheap apartment", topicUpgradeToCheap)
tl:AddTopics("upgrade to normal apartment", topicUpgradeToNormal)
tl:AddTopics("upgrade to expensive apartment", topicUpgradeToExpensive)
tl:AddTopics("upgrade to luxurious apartment", topicUpgradeToLuxurious)

tl:AddTopics("sell me a cheap apartment", topicSellCheap)
tl:AddTopics("sell me a normal apartment", topicSellNormal)
tl:AddTopics("sell me an expensive apartment", topicSellExpensive)
tl:AddTopics("sell me an luxurious apartment", topicSellLuxurious)
tl:AddTopics("upgrade_confirmation1", topicConfirm)
tl:AddTopics("upgrade_confirmation2", topicConfirmAgain)
tl:AddTopics({"pocket dimension", "pocket", "dimension"},topicDimension)
tl:AddGreeting(nil, topicGreeting)
tl:AddTopics("upgrade", topicUpgrade)
tl:AddTopics("procedure", topicProcedure)
tl:AddTopics("apartment",topicApartment)
tl:AddTopics("cheap", topicCheap)
tl:AddTopics("normal", topicNormal)
tl:AddTopics("expensive", topicExpensive)
tl:AddTopics("luxurious", topicLux)
tl:CheckMessage(event)