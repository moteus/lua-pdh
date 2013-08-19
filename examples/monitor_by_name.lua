
local PROCESS_NAME = arg[1] or "notepad"
local COUNTER_NAME = arg[2] or "% Processor Time"

local pdh = require "pdh"

local T = pdh.translate_path

local assert = function(...) if not ... then return assert(..., tostring((select(2, ...)))) end return ... end

local query = pdh.query()
local counterPid = assert(query:add_counter(T[[\Process(*)\ID Process]]))
local counterCpu = assert(query:add_counter(T([[\Process(*)\]] .. COUNTER_NAME)))

local process_list = {}

query:collect()
pdh.sleep(100)
while true do
  query:collect()

  -- support active instance list
  if #process_list == 0 then print("Enter counterPid") while true do
    local instance = 0
    local _, status = counterPid:as_double_array(function(i, name, pid)
      if name == PROCESS_NAME then
        table.insert(process_list, {i, name, pid})
      end
    end)
    if status then
      print("counterPid collect:", status)
      if status:no() == pdh.PDH_CSTATUS_INVALID_DATA then
        process_list = {}
      else pdh.sleep(1000) end
    else pdh.sleep(1000) end
    if #process_list > 0 then break end
    query:collect()
  end end print("Leave counterPid")

  -- display processor usage
  if #process_list > 0 then print("Enter counterCpu") while true do
    local _, status = counterCpu:as_double_array(function(i, name, cpu)
      for instance, t in ipairs(process_list) do
        if t[1] == i then
          -- ??? invalid list
          if name ~= t[2] then process_list = {} return false end
          local name = name .. '#' .. (instance - 1)
          name = counterCpu:path():gsub("%*#0", name):gsub("%*", name)
          local pid = t[3]
          print(os.date(), name, pid, cpu)
        end
      end
    end)
    if status then
      print("counterCpu collect:", status)
      if status:no() == pdh.PDH_CSTATUS_INVALID_DATA then
        process_list = {}
      end
    end
    print("-------------------------------")
    if #process_list == 0 then break end
    pdh.sleep(1000)
    query:collect()
  end end print("Leave counterCpu")

end

