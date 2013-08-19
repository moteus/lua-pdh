local psapi = require "pdh.psapi"

local function msecToHMS(msec)
  local s = math.floor(msec/1000)
  local h = math.floor(s/3600); s = s - h * 3600
  local m = math.floor(s/60);   s = s - m * 60
  return h, m, s
end

local function hms(msec)
  return (string.format("%.2d:%.2d:%.2d", msecToHMS(msec)))
end

local KB = 2^10
local MB, GB = KB^2, KB^3
local MNAME = { [KB] ='KB'; [MB] ='MB'; [GB] ='GB'; }
local function M(n, K)
  return tostring((math.floor(n / K))) .. ' ' .. MNAME[K]
end

local process, status = psapi.process()
psapi.enum_processes(function(i, pid)
  local ok, status = process:open(pid)
  if not ok then print(pid, status)
  else
    local memory_info = process:memory_info()
    local creationTime, exitTime, kernelTime, userTime = process:times()
    print(i, pid, process:base_name() or "----", hms(kernelTime + userTime), M(memory_info.WorkingSetSize, KB), process:command_line())
  end
end)
process:destroy()
