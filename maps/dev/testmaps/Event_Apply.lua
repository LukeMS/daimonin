event.activator:Write("This is an APPLY script!")
if(event.options == 'override') then
    event.activator:Write("Overriding normal behaviour...")
    event.returnvalue = 1
end