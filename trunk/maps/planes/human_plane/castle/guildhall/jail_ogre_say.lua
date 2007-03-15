-- make ogre spit when jail guard or some else tease him
if string.lower(event.message) == "still there, frah'ak?" then
	event.me:Communicate("/spit " .. event.activator.name)
end