--This script creates a tome.  Add as apply script to a container.  Change arch tome to any
--other readable arch (if desired), change name to desired name, change the contents of the
--message (msg\n desired message\nendmsg) for your custom text.  Change grave to container name.
local grave   = event.me
local player  = event.activator


local book = game:LoadObject("arch tome\nname Notes of Lon'Li'Uni'Sm\nmsg \nMost of the text is ruined from moisture but you can make out a few words on the last page... ...Moroch .... so foul .... done to me .... I shall be avenged like the ....\nendmsg")
book:InsertInside(grave)
