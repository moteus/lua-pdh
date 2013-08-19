package = "pdh"
version = "scm-0"
source = {
  url = "https://github.com/moteus/lua-pdh/archive/master.zip",
  dir = "lua-pdh-master",
}

description = {
  summary = "Simple POP3 client library for Lua 5.1/5.2",
  homepage = "https://github.com/moteus/lua-pdh",
  license  = "MIT/X11",
}

dependencies = {
  "lua >= 5.1, < 5.3",
}

build = {
  type = "builtin",
  copy_directories = {"examples"},
  platforms = { windows = {
    modules = {
      [ "pdh.core"    ] = {
        sources = {
          'src/l52util.c', 'src/lpdh.c',
        };
        libraries = {'pdh', 'psapi', 'advapi32'};
      };
      [ "pdh"         ] = "lua/pdh.lua";
      [ "pdh.psapi"   ] = "lua/pdh/psapi.lua";
    }
  }}
}
