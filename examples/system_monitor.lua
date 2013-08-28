---
-- send system counters to statsd server
--
-- depend on
-- lua-pdh (https://github.com/moteus/lua-pdh)
-- lua-statsd-client (https://github.com/stvp/lua-statsd-client)
-- lua-log (https://github.com/moteus/lua-log)

local CONFIG = {
  INTERVAL  = 1; -- sec
  LOG_LEVEL = "info";
  STATSD    = {
    NAMESPACE  = 'test.system_info';
    HOST       = "127.0.0.1";
    PORT       = "8125";
    SEND_TABLE = false;
  };
  COUNTERS  = {
    processor = {
      counter = [[\Processor(*)\% Processor Time]];
      array   = true;
    };
    available_bytes = {
      counter = [[\Memory\Available Bytes]];
      array   = false;
    };
    uptime = {
      counter = [[\System\System Up Time]];
      array   = false;
    };
    memory_pages = {
      counter = [[\Memory\Pages/sec]];
      array   = false;
    };
    disk_transfers = {
      counter = [[\PhysicalDisk(*)\Disk Transfers/sec]];
      array   = true;
    };
    disk_idle = {
      counter = [[\PhysicalDisk(*)\% Idle Time]];
      array   = true;
    };
    net_bps = {
      counter = [[\Network Interface(*)\Bytes Total/sec]];
      array   = true;
    };
  };
}

----------------------------------------------------------------------------

local pdh    = require "pdh"
local LOG    = require"log".new(
  CONFIG.LOG_LEVEL,
  require "log.writer.list".new(
    require "log.writer.stdout".new()
  ),
  require"log.formatter.concat".new('')
)
local statsd = assert(require "statsd"{
  host      = CONFIG.STATSD.HOST;
  port      = CONFIG.STATSD.PORT;
  namespace = CONFIG.STATSD.NAMESPACE;
})

if pdh.disabled() then
  LOG.fatal("Performance counters are disabled on this computer.")
  return -1
end

local query, status = pdh.query()
if not query then
  LOG.fatal("can not create PDH query: ", status)
  return -2
end

local counters = {}
for counter_name, cfg in pairs(CONFIG.COUNTERS) do
  local name = cfg.counter
  local tname, status = pdh.translate_path(name)
  if not tname then LOG.emerg("can not translate counter name `", name, "` :", status) else
    LOG.info("create new counter: `", name, "`(", tname, ") ...")
    local counter, status = pdh.counter(tname)
    if not counter then LOG.emerg("can not create counter `", name, "`(", tname, ") :", status) else
      local ok, status = query:add_counter(counter)
      if not ok then LOG.emerg("can not add counter to query`", name, "`(", tname, ") :", status) else
        LOG.info("create new counter `", counter_name, "` with `", tname, "`")
        counters[counter_name] = { counter;
          config  = cfg;
        }
      end
    end
  end
end

query:collect()
pdh.sleep(1)
while true do
  local ok, status = query:collect()
  if not ok then LOG.error("counter collection error :", status) else
    local gauges = {}
    for counter_name, counter in pairs(counters) do
      if counter.config.array then
        ok, status = counter[1]:as_double_array(function(i, name, value, status)
          local full_counter_name = counter_name .. "." .. name
          if status then
            LOG.error("get value error for `", full_counter_name, "` :", status)
            return
          end
          LOG.debug(full_counter_name, ' = ', value)
          gauges[full_counter_name] = value;
        end)
        if status then LOG.error("get value error for `", counter_name, "` :", status) end
      else
        local value, status = counter[1]:as_double()
        if not value then LOG.error("get value error for `", counter_name, "` :", status) else
          LOG.debug(counter_name, ' = ', value)
          gauges[counter_name] = value;
        end
      end
    end
    if CONFIG.STATSD.SEND_TABLE then
      statsd:gauge(gauges)
    else
      for name, value in pairs(gauges) do
        statsd:gauge(name, value)
      end
    end
  end
  pdh.sleep(CONFIG.INTERVAL * 1000)
end
