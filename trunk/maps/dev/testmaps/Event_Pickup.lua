event.activator:Write("This is a pickup script!")
if(event.options == 'override') then
    event.activator:Write("Overriding normal behaviour...")
    event.returnvalue = 1
end
