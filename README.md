```Lua
local pdh = require "pdh"

assert(not pdh.disabled())

local query = pdh.query()

local path    = pdh.translate_path([[\Processor(_Total)\% Processor Time]])
local counter = query:add_counter(path)

query:collect()
pdh.sleep(1)
while true do
  query:collect()
  print(path, " => ", math.floor(counter:as_double()))
  pdh.sleep(1000)
end
```

[![Bitdeli Badge](https://d2weczhvl823v0.cloudfront.net/moteus/lua-pdh/trend.png)](https://bitdeli.com/free "Bitdeli Badge")

