local sahrel = event.activator

if math.random() >= 0.7 then
    return
end

local frahak = {
    [1] = "Frak",
    [2] = "Frok",
    [3] = "Frohog",
    [4] = "Frahik",
    [5] = "prisoner",
    [6] = "ya wimp",
    [7] = "big cheef Frook",
}

frahak = frahak[math.random(table.getn(frahak))]

local taunt = {
    [1] = "How's it going, " .. frahak .. "? *Heh heh heh*",
    [2] = "Ah, " .. frahak .. ", not escaped I see? *Heh heh*",
    [3] = "Are we having fun yet, " .. frahak .. "?",
    [4] = string.capitalize(frahak) .. ", called a lawyer yet? *Heh heh*",
    [5] = "Don't complain, " .. frahak .. ". This is a picnic compared to the Dom.",
    [6] = "Maybe this'll learn ya, eh " .. frahak .. "?",
    [7] = "Enjoying life, " .. frahak .. "? *Heh*",
    [8] = "Don't see too many as deserving as you, " .. frahak .. "! *Heh heh*",
    [9] = "If I had my way I'd throw away the key, " .. frahak .. ".",
}

sahrel:Communicate(taunt[math.random(table.getn(taunt))])
