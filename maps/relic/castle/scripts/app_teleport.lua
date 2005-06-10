require("topic_list")

shavn_app_tag = "SHAVN_APP_INFO"
appid_cheap = "cheap"
appid_normal = "normal"
appid_expensive = "expensive"
appid_luxurious = "luxurious"

me = event.me
activator = event.activator

pinfo = activator:GetPlayerInfo(shavn_app_tag) -- search for the right player info
if pinfo == nil then -- no apartment - teleport him back
	me:SayTo(activator, "You don't own an apartment here!")
	activator:Write("A strong force teleports you away.", 0)
	activator:SetPosition(9, 18)
else
	pinfo_appid = pinfo.slaying -- thats the apartment tag
	if pinfo_appid == appid_cheap then
		activator:TeleportTo("/castle/appartments/appartment_1", 1, 2, 1)
	elseif pinfo_appid == appid_normal then
		activator:TeleportTo("/castle/appartments/appartment_2", 1, 2, 1)
	elseif pinfo_appid == appid_expensive then
		activator:TeleportTo("/castle/appartments/appartment_3", 1, 2, 1)
	elseif pinfo_appid == appid_luxurious then
		activator:TeleportTo("/castle/appartments/appartment_4", 2, 1, 1)
	else
		me:SayTo(activator, "Wrong apartment ID??!")
		activator:Write("A strong force teleporting you backwards.", 0)
		activator:SetPosition(7, 20)
    end
end
