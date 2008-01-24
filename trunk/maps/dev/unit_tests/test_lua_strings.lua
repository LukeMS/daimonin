local message = event.message
local me = event.me

if message == "long" then
    me.name = "init"
    me.message = string.rep("!", 9000)
    print (me:Save())
    event.returnvalue = -1
elseif message == "newline" then
    me.name = "init"
    me.title = "title_line_1\ntitle_line_2"
    if string.find(game:LoadObject(me:Save()):Save(), "title_line_2") then
        me.slaying="success"
    else
        me.slaying="failure"
    end
elseif message == "endmsg" then
    me.name = "init"
    me.message = "hello1\nendmsg\ntitle of cheating\nmsg\nhello2"
    obj = game:LoadObject(me:Save())
    if string.find(obj.message, "hello1.*hello2") then
        me.slaying="success"
    else
        me.slaying="failure"
    end
else
    event.returnvalue = 1
end
