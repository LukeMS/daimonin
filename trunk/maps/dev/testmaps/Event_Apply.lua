if event.me.f_applied then
    event.activator:Write("This is an UNAPPLY script!")
else
    event.activator:Write("This is an APPLY script!")
end

if(event.options == 'override') then
    event.activator:Write("Overriding normal behaviour...")
    event.returnvalue = 1
end
