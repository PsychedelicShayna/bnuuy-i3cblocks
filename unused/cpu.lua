#!/usr/bin/env luajit

local PREV = "/tmp/.i3blocks_cpu"

-- Read current CPU line from /proc/stat
local current_file = io.open("/proc/stat", "r")
local current_line = current_file:read("*l"):match("^cpu%s+(.+)$")
current_file:close()

-- Read previous data
local prev_data = ""
local prev_file = io.open(PREV, "r")
if prev_file then
    prev_data = prev_file:read("*a")
    prev_file:close()
end

-- Write current data to prev file
local prev_file = io.open(PREV, "w")
prev_file:write(current_line)
prev_file:close()

if prev_data == "" then
    print("CPU: ...")
    os.exit(0)
end

-- Parse current: split whitespace, get fields 1-9 (indices 1-9 in Lua)
local current_fields = {}
for field in current_line:gmatch("%S+") do
    table.insert(current_fields, tonumber(field))
end
local idle_now = current_fields[5]
local total_now = 0
for i = 1, 9 do total_now = total_now + (current_fields[i] or 0) end

-- Parse previous
local prev_fields = {}
for field in prev_data:gmatch("%S+") do
    table.insert(prev_fields, tonumber(field))
end
local idle_before = prev_fields[5]
local total_before = 0
for i = 1, 9 do total_before = total_before + (prev_fields[i] or 0) end

local diff_idle = idle_now - idle_before
local diff_total = total_now - total_before
local diff_usage = math.floor((1000 * (diff_total - diff_idle) / diff_total + 5) / 10)

-- Get frequency (using io.popen for speed)
local freq = "N/A"
local pipe = io.popen("./cpufreq.sh")
if pipe then
    freq = pipe:read("*l") or "N/A"
    pipe:close()
end

print(string.format("%d%% %s", diff_usage, freq))
print("")

-- Color hinting
if diff_usage > 80 then
    print("#cc241d")
elseif diff_usage > 50 then
    print("#d65d0e")
end
