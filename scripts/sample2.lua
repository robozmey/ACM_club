
function enter()
    room = {  }
    room["background"] = coridor2
    room["say"]        = "Hello"
    room["type"]       = "Homework"
    room["homework"]   = {}
    room["homework"][0] = {contest_id = 1286, letter = "A", name = "Girlyanda"}
    room["homework"][1] = {contest_id = 1286, letter = "B", name = "Ints on tree"}
    room["characters"] = {happy[yana], neutral[sasha]}
    room["speaker"]    = name[sasha]
    return room
end

function next(variant)
    glob_sets(sasha, "muda")
    return "sample3" -- next file is sample.lua
end