#include "lua.h"
#include "l52util.h"
#include <assert.h>
#include <Pdh.h>
#include <PdhMsg.h>

#define LPDH_EXPORT __declspec(dllexport)

#define LPDH_CONCAT_STATIC_ASSERT_IMPL_(x, y) LPDH_CONCAT1_STATIC_ASSERT_IMPL_ (x, y)
#define LPDH_CONCAT1_STATIC_ASSERT_IMPL_(x, y) x##y
#define LPDH_STATIC_ASSERT(expr) typedef char LPDH_CONCAT_STATIC_ASSERT_IMPL_(static_assert_failed_at_line_, __LINE__) [(expr) ? 1 : -1]

#define ASSERT_SAME_SIZE(a, b) LPDH_STATIC_ASSERT( sizeof(a) == sizeof(b) )
#define ASSERT_SAME_OFFSET(a, am, b, bm) LPDH_STATIC_ASSERT( (offsetof(a,am)) == (offsetof(b,bm)) )

static const char *LPDH_QUERY   = "PDH Query";
static const char *LPDH_COUNTER = "PDH Counter";
static const char *LPDH_ERROR   = "PDH Error";
static const char *LPDH_COUNTER_NAMES = "PDH Counter names";

#define FLAG_TYPE unsigned char
#define FLAG_DESTROYED (FLAG_TYPE)1 << 0
#define FLAG_OPEN      (FLAG_TYPE)1 << 1

typedef struct lpdh_query_tag{
  FLAG_TYPE flags;
  PDH_HQUERY handle;
}lpdh_query_t;

typedef struct lpdh_counter_tag{
  FLAG_TYPE flags;
  HCOUNTER handle;
  char path[PDH_MAX_COUNTER_PATH + 1];
}lpdh_counter_t;

#define LPDH_ERROR_LIBRARY    1
#define LPDH_ERROR_SYSTEM     2
#define LPDH_ERROR_PDH        3

typedef struct lpdh_error_tag{
  int   type;
  DWORD status;
} lpdh_error_t;

//{ LPDH_PROCEED_ERROR

#define LPDH_PROCEED_ERROR()                                                       \
  LPDH_PROCEED_ERROR_NODE(ERROR_SUCCESS)                                           \
                                                                                   \
  LPDH_PROCEED_ERROR_NODE(PDH_CSTATUS_VALID_DATA)                                  \
  LPDH_PROCEED_ERROR_NODE(PDH_CSTATUS_NEW_DATA)                                    \
  LPDH_PROCEED_ERROR_NODE(PDH_CSTATUS_NO_MACHINE)                                  \
  LPDH_PROCEED_ERROR_NODE(PDH_CSTATUS_NO_INSTANCE)                                 \
  LPDH_PROCEED_ERROR_NODE(PDH_MORE_DATA)                                           \
  LPDH_PROCEED_ERROR_NODE(PDH_CSTATUS_ITEM_NOT_VALIDATED)                          \
  LPDH_PROCEED_ERROR_NODE(PDH_RETRY)                                               \
  LPDH_PROCEED_ERROR_NODE(PDH_NO_DATA)                                             \
  LPDH_PROCEED_ERROR_NODE(PDH_CALC_NEGATIVE_DENOMINATOR)                           \
  LPDH_PROCEED_ERROR_NODE(PDH_CALC_NEGATIVE_TIMEBASE)                              \
  LPDH_PROCEED_ERROR_NODE(PDH_CALC_NEGATIVE_VALUE)                                 \
  LPDH_PROCEED_ERROR_NODE(PDH_DIALOG_CANCELLED)                                    \
  LPDH_PROCEED_ERROR_NODE(PDH_END_OF_LOG_FILE)                                     \
  LPDH_PROCEED_ERROR_NODE(PDH_ASYNC_QUERY_TIMEOUT)                                 \
  LPDH_PROCEED_ERROR_NODE(PDH_CANNOT_SET_DEFAULT_REALTIME_DATASOURCE)              \
  LPDH_PROCEED_ERROR_NODE(PDH_UNABLE_MAP_NAME_FILES)                               \
  LPDH_PROCEED_ERROR_NODE(PDH_PLA_VALIDATION_WARNING)                              \
  LPDH_PROCEED_ERROR_NODE(PDH_CSTATUS_NO_OBJECT)                                   \
  LPDH_PROCEED_ERROR_NODE(PDH_CSTATUS_NO_COUNTER)                                  \
  LPDH_PROCEED_ERROR_NODE(PDH_CSTATUS_INVALID_DATA)                                \
  LPDH_PROCEED_ERROR_NODE(PDH_MEMORY_ALLOCATION_FAILURE)                           \
  LPDH_PROCEED_ERROR_NODE(PDH_INVALID_HANDLE)                                      \
  LPDH_PROCEED_ERROR_NODE(PDH_INVALID_ARGUMENT)                                    \
  LPDH_PROCEED_ERROR_NODE(PDH_FUNCTION_NOT_FOUND)                                  \
  LPDH_PROCEED_ERROR_NODE(PDH_CSTATUS_NO_COUNTERNAME)                              \
  LPDH_PROCEED_ERROR_NODE(PDH_CSTATUS_BAD_COUNTERNAME)                             \
  LPDH_PROCEED_ERROR_NODE(PDH_INVALID_BUFFER)                                      \
  LPDH_PROCEED_ERROR_NODE(PDH_INSUFFICIENT_BUFFER)                                 \
  LPDH_PROCEED_ERROR_NODE(PDH_CANNOT_CONNECT_MACHINE)                              \
  LPDH_PROCEED_ERROR_NODE(PDH_INVALID_PATH)                                        \
  LPDH_PROCEED_ERROR_NODE(PDH_INVALID_INSTANCE)                                    \
  LPDH_PROCEED_ERROR_NODE(PDH_INVALID_DATA)                                        \
  LPDH_PROCEED_ERROR_NODE(PDH_NO_DIALOG_DATA)                                      \
  LPDH_PROCEED_ERROR_NODE(PDH_CANNOT_READ_NAME_STRINGS)                            \
  LPDH_PROCEED_ERROR_NODE(PDH_LOG_FILE_CREATE_ERROR)                               \
  LPDH_PROCEED_ERROR_NODE(PDH_LOG_FILE_OPEN_ERROR)                                 \
  LPDH_PROCEED_ERROR_NODE(PDH_LOG_TYPE_NOT_FOUND)                                  \
  LPDH_PROCEED_ERROR_NODE(PDH_NO_MORE_DATA)                                        \
  LPDH_PROCEED_ERROR_NODE(PDH_ENTRY_NOT_IN_LOG_FILE)                               \
  LPDH_PROCEED_ERROR_NODE(PDH_DATA_SOURCE_IS_LOG_FILE)                             \
  LPDH_PROCEED_ERROR_NODE(PDH_DATA_SOURCE_IS_REAL_TIME)                            \
  LPDH_PROCEED_ERROR_NODE(PDH_UNABLE_READ_LOG_HEADER)                              \
  LPDH_PROCEED_ERROR_NODE(PDH_FILE_NOT_FOUND)                                      \
  LPDH_PROCEED_ERROR_NODE(PDH_FILE_ALREADY_EXISTS)                                 \
  LPDH_PROCEED_ERROR_NODE(PDH_NOT_IMPLEMENTED)                                     \
  LPDH_PROCEED_ERROR_NODE(PDH_STRING_NOT_FOUND)                                    \
  LPDH_PROCEED_ERROR_NODE(PDH_UNKNOWN_LOG_FORMAT)                                  \
  LPDH_PROCEED_ERROR_NODE(PDH_UNKNOWN_LOGSVC_COMMAND)                              \
  LPDH_PROCEED_ERROR_NODE(PDH_LOGSVC_QUERY_NOT_FOUND)                              \
  LPDH_PROCEED_ERROR_NODE(PDH_LOGSVC_NOT_OPENED)                                   \
  LPDH_PROCEED_ERROR_NODE(PDH_WBEM_ERROR)                                          \
  LPDH_PROCEED_ERROR_NODE(PDH_ACCESS_DENIED)                                       \
  LPDH_PROCEED_ERROR_NODE(PDH_LOG_FILE_TOO_SMALL)                                  \
  LPDH_PROCEED_ERROR_NODE(PDH_INVALID_DATASOURCE)                                  \
  LPDH_PROCEED_ERROR_NODE(PDH_INVALID_SQLDB)                                       \
  LPDH_PROCEED_ERROR_NODE(PDH_NO_COUNTERS)                                         \
  LPDH_PROCEED_ERROR_NODE(PDH_SQL_ALLOC_FAILED)                                    \
  LPDH_PROCEED_ERROR_NODE(PDH_SQL_ALLOCCON_FAILED)                                 \
  LPDH_PROCEED_ERROR_NODE(PDH_SQL_EXEC_DIRECT_FAILED)                              \
  LPDH_PROCEED_ERROR_NODE(PDH_SQL_FETCH_FAILED)                                    \
  LPDH_PROCEED_ERROR_NODE(PDH_SQL_ROWCOUNT_FAILED)                                 \
  LPDH_PROCEED_ERROR_NODE(PDH_SQL_MORE_RESULTS_FAILED)                             \
  LPDH_PROCEED_ERROR_NODE(PDH_SQL_CONNECT_FAILED)                                  \
  LPDH_PROCEED_ERROR_NODE(PDH_SQL_BIND_FAILED)                                     \
  LPDH_PROCEED_ERROR_NODE(PDH_CANNOT_CONNECT_WMI_SERVER)                           \
  LPDH_PROCEED_ERROR_NODE(PDH_PLA_COLLECTION_ALREADY_RUNNING)                      \
  LPDH_PROCEED_ERROR_NODE(PDH_PLA_ERROR_SCHEDULE_OVERLAP)                          \
  LPDH_PROCEED_ERROR_NODE(PDH_PLA_COLLECTION_NOT_FOUND)                            \
  LPDH_PROCEED_ERROR_NODE(PDH_PLA_ERROR_SCHEDULE_ELAPSED)                          \
  LPDH_PROCEED_ERROR_NODE(PDH_PLA_ERROR_NOSTART)                                   \
  LPDH_PROCEED_ERROR_NODE(PDH_PLA_ERROR_ALREADY_EXISTS)                            \
  LPDH_PROCEED_ERROR_NODE(PDH_PLA_ERROR_TYPE_MISMATCH)                             \
  LPDH_PROCEED_ERROR_NODE(PDH_PLA_ERROR_FILEPATH)                                  \
  LPDH_PROCEED_ERROR_NODE(PDH_PLA_SERVICE_ERROR)                                   \
  LPDH_PROCEED_ERROR_NODE(PDH_PLA_VALIDATION_ERROR)                                \
  LPDH_PROCEED_ERROR_NODE(PDH_PLA_ERROR_NAME_TOO_LONG)                             \
  LPDH_PROCEED_ERROR_NODE(PDH_INVALID_SQL_LOG_FORMAT)                              \
  LPDH_PROCEED_ERROR_NODE(PDH_COUNTER_ALREADY_IN_QUERY)                            \
  LPDH_PROCEED_ERROR_NODE(PDH_BINARY_LOG_CORRUPT)                                  \
  LPDH_PROCEED_ERROR_NODE(PDH_LOG_SAMPLE_TOO_SMALL)                                \
  LPDH_PROCEED_ERROR_NODE(PDH_OS_LATER_VERSION)                                    \
  LPDH_PROCEED_ERROR_NODE(PDH_OS_EARLIER_VERSION)                                  \
  LPDH_PROCEED_ERROR_NODE(PDH_INCORRECT_APPEND_TIME)                               \
  LPDH_PROCEED_ERROR_NODE(PDH_UNMATCHED_APPEND_COUNTER)                            \
  LPDH_PROCEED_ERROR_NODE(PDH_SQL_ALTER_DETAIL_FAILED)                             \
  LPDH_PROCEED_ERROR_NODE(PDH_QUERY_PERF_DATA_TIMEOUT)                             \
                                                                                   \
  LPDH_PROCEED_ERROR_NODE(ERROR_INVALID_FUNCTION)                                  \
  LPDH_PROCEED_ERROR_NODE(ERROR_FILE_NOT_FOUND)                                    \
  LPDH_PROCEED_ERROR_NODE(ERROR_PATH_NOT_FOUND)                                    \
  LPDH_PROCEED_ERROR_NODE(ERROR_TOO_MANY_OPEN_FILES)                               \
  LPDH_PROCEED_ERROR_NODE(ERROR_ACCESS_DENIED)                                     \
  LPDH_PROCEED_ERROR_NODE(ERROR_INVALID_HANDLE)                                    \
  LPDH_PROCEED_ERROR_NODE(ERROR_ARENA_TRASHED)                                     \
  LPDH_PROCEED_ERROR_NODE(ERROR_NOT_ENOUGH_MEMORY)                                 \
  LPDH_PROCEED_ERROR_NODE(ERROR_INVALID_BLOCK)                                     \
  LPDH_PROCEED_ERROR_NODE(ERROR_BAD_ENVIRONMENT)                                   \
  LPDH_PROCEED_ERROR_NODE(ERROR_BAD_FORMAT)                                        \
  LPDH_PROCEED_ERROR_NODE(ERROR_INVALID_ACCESS)                                    \
  LPDH_PROCEED_ERROR_NODE(ERROR_INVALID_DATA)                                      \
  LPDH_PROCEED_ERROR_NODE(ERROR_OUTOFMEMORY)                                       \
  LPDH_PROCEED_ERROR_NODE(ERROR_INVALID_DRIVE)                                     \
  LPDH_PROCEED_ERROR_NODE(ERROR_CURRENT_DIRECTORY)                                 \
  LPDH_PROCEED_ERROR_NODE(ERROR_NOT_SAME_DEVICE)                                   \
  LPDH_PROCEED_ERROR_NODE(ERROR_NO_MORE_FILES)                                     \
  LPDH_PROCEED_ERROR_NODE(ERROR_WRITE_PROTECT)                                     \
  LPDH_PROCEED_ERROR_NODE(ERROR_BAD_UNIT)                                          \
  LPDH_PROCEED_ERROR_NODE(ERROR_NOT_READY)                                         \
  LPDH_PROCEED_ERROR_NODE(ERROR_BAD_COMMAND)                                       \
  LPDH_PROCEED_ERROR_NODE(ERROR_CRC)                                               \
  LPDH_PROCEED_ERROR_NODE(ERROR_BAD_LENGTH)                                        \
  LPDH_PROCEED_ERROR_NODE(ERROR_SEEK)                                              \
  LPDH_PROCEED_ERROR_NODE(ERROR_NOT_DOS_DISK)                                      \
  LPDH_PROCEED_ERROR_NODE(ERROR_SECTOR_NOT_FOUND)                                  \
  LPDH_PROCEED_ERROR_NODE(ERROR_OUT_OF_PAPER)                                      \
  LPDH_PROCEED_ERROR_NODE(ERROR_WRITE_FAULT)                                       \
  LPDH_PROCEED_ERROR_NODE(ERROR_READ_FAULT)                                        \
  LPDH_PROCEED_ERROR_NODE(ERROR_GEN_FAILURE)                                       \
  LPDH_PROCEED_ERROR_NODE(ERROR_SHARING_VIOLATION)                                 \
  LPDH_PROCEED_ERROR_NODE(ERROR_LOCK_VIOLATION)                                    \
  LPDH_PROCEED_ERROR_NODE(ERROR_WRONG_DISK)                                        \
  LPDH_PROCEED_ERROR_NODE(ERROR_SHARING_BUFFER_EXCEEDED)                           \
  LPDH_PROCEED_ERROR_NODE(ERROR_HANDLE_EOF)                                        \
  LPDH_PROCEED_ERROR_NODE(ERROR_HANDLE_DISK_FULL)                                  \
  LPDH_PROCEED_ERROR_NODE(ERROR_NOT_SUPPORTED)                                     \
  LPDH_PROCEED_ERROR_NODE(ERROR_MORE_DATA)                                         \
  LPDH_PROCEED_ERROR_NODE(ERROR_NETWORK_ACCESS_DENIED)                             \
  LPDH_PROCEED_ERROR_NODE(ERROR_BAD_NET_NAME)                                      \
  LPDH_PROCEED_ERROR_NODE(ERROR_INVALID_PARAMETER)                                 \
 
//}

//{

#define LPDH_PROCEED_CONST()                                                    \
  LPDH_PROCEED_CONST_NODE(PDH_FMT_NOSCALE)                                      \
  LPDH_PROCEED_CONST_NODE(PDH_FMT_NOCAP100)                                     \
  LPDH_PROCEED_CONST_NODE(PDH_FMT_1000)                                         \

//}

static lpdh_counter_t *lpdh_getcounter_at (lua_State *L, int i);

//{ Error

// to correct work with error type
LPDH_STATIC_ASSERT(sizeof(PDH_STATUS) <= sizeof(DWORD));

LPDH_STATIC_ASSERT(sizeof(((lpdh_error_t*)0)->status) <= (sizeof(void*)));

static lpdh_error_t *lpdh_geterror_at (lua_State *L, int i) {
  lpdh_error_t *err = (lpdh_error_t *)lutil_checkudatap (L, i, LPDH_ERROR);
  luaL_argcheck (L, err != NULL, 1, "PDH Error expected");
  return err;
}

//{ push error

static int lpdh_error_push_system_message(lua_State *L, DWORD status){
  char errbuff[256];
  int sz;
  sz = FormatMessage(
    FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,status,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
    errbuff, sizeof(errbuff), NULL);
  if(sz > 1) sz -= 2; 
  lua_pushlstring(L, errbuff, sz);
  return 1;
}

static int lpdh_error_push_pdh_message(lua_State *L, DWORD status){
  HMODULE hPDH = GetModuleHandle("PDH.DLL");
  if(hPDH){
    char errbuff[256];
    int sz;
    sz = FormatMessage(
      FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS |  FORMAT_MESSAGE_FROM_HMODULE,
      hPDH,status,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
      errbuff, sizeof(errbuff), NULL
     );
    if(sz){
      if(sz > 1) sz -= 2;
      lua_pushlstring(L, errbuff, sz);
      return 1;
    }
  }
  return lpdh_error_push_system_message(L, status);
}

static int lpdh_error_push_library_message(lua_State *L, DWORD status){
  lua_pushstring(L,"unknown");
  return 1;
}

static int lpdh_error_push_message(lua_State *L, lpdh_error_t *err, const char* def){
  if(err->type == LPDH_ERROR_PDH)     return lpdh_error_push_pdh_message     (L, err->status);
  if(err->type == LPDH_ERROR_SYSTEM)  return lpdh_error_push_system_message  (L, err->status);
  if(err->type == LPDH_ERROR_LIBRARY) return lpdh_error_push_library_message (L, err->status);
  lua_pushstring(L, def);
  return 1;
}

//}

//{ mnemo

static const char* lpdh_error_system_mnemo(DWORD status, const char* def){
#define LPDH_PROCEED_ERROR_NODE(VALUE) if(status == VALUE){static const char *name = #VALUE; return name;}
  LPDH_PROCEED_ERROR()
#undef LPDH_PROCEED_ERROR_NODE
  return def;
}

static const char* lpdh_error_pdh_mnemo(DWORD status, const char* def){
  return lpdh_error_system_mnemo(status, def);
}

static const char* lpdh_error_library_mnemo(DWORD status, const char* def){
  return def;
}

static const char* lpdh_error_mnemo_(lpdh_error_t *err, const char* def){
  if(err->type == LPDH_ERROR_PDH)     return lpdh_error_pdh_mnemo     (err->status, def);
  if(err->type == LPDH_ERROR_SYSTEM)  return lpdh_error_system_mnemo  (err->status, def);
  if(err->type == LPDH_ERROR_LIBRARY) return lpdh_error_library_mnemo (err->status, def);
  return def;
}

static int lpdh_error_push_mnemo(lua_State *L, lpdh_error_t *err, const char* def){
  lua_pushstring(L, lpdh_error_mnemo_(err, def));
  return 1;
}

//}

static void lpdh_error_pushstring(lua_State *L, lpdh_error_t *err){
  void *ptr_status = (void*)err->status;
  lpdh_error_push_message(L, err, "unknown");
  lua_pushfstring(L, "[%s] %s (0x%p)",
    lpdh_error_mnemo_(err, "UNKNOWN"),
    lua_tostring(L, -1),
    ptr_status
  );
  lua_remove(L, -2);
}

static int lpdh_error_mnemo(lua_State *L){
  return lpdh_error_push_mnemo(L, lpdh_geterror_at(L,1), "<UNKNOWN>");
}

static int lpdh_error_message(lua_State *L){
  return lpdh_error_push_message(L, lpdh_geterror_at(L,1), "unknown");
}

static int lpdh_error_number(lua_State *L){
  lpdh_error_t *err = lpdh_geterror_at(L,1);
  lua_pushnumber(L, err->status);
  return 1;
}

static int lpdh_error_type(lua_State *L){
  lpdh_error_t *err = lpdh_geterror_at(L,1);
  lua_pushnumber(L, err->type);
  return 1;
}

static int lpdh_error_tostring(lua_State *L){
  lpdh_error_t *err = lpdh_geterror_at(L,1);
  lpdh_error_pushstring(L, err);
  return 1;
}

static int lpdh_error_push(lua_State *L, int type, DWORD status){
  lpdh_error_t *err;
  assert((type == LPDH_ERROR_LIBRARY) || (type == LPDH_ERROR_SYSTEM) || (type == LPDH_ERROR_PDH));
  err = lutil_newudatap(L, lpdh_error_t, LPDH_ERROR);
  err->type   = type;
  err->status = status;
  return 1;
}

static int lpdh_error(lua_State *L, int type, DWORD status){
  lua_pushnil(L);
  lpdh_error_push(L, type, status);
  return 2;
}

#define lpdh_error_system(L, S) lpdh_error(L, LPDH_ERROR_SYSTEM, S)

#define lpdh_error_library(L, S) lpdh_error(L, LPDH_ERROR_LIBRARY, S)

#define lpdh_error_pdh(L, S) lpdh_error(L, LPDH_ERROR_PDH, S)

static int lpdh_pass(lua_State *L){
  lua_pushboolean(L, 1);
  return 1;
}

//}

//{ Registry

static struct lpdh_reg_path_tag{
  HKEY key;
  const char *path;
} LPDH_DISABLED_REG_PATH[] = {
  {HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib"},
  {HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\PerfOS\\Performance"},
  {HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\PerfProc\\Performance"},
  {0, NULL}
};

const char *LPDH_DISABLED_REG_NAME = "Disable Performance Counters";

static DWORD lpdh_reg_read_dword(HKEY key, const char *path, const char *name, DWORD *value){
  DWORD keyType; DWORD keySize = sizeof(DWORD);
  HKEY hKey;
  DWORD Status = RegOpenKeyEx(key, path, 0, KEY_READ, &hKey);
  if(Status != ERROR_SUCCESS) return Status;
  Status = RegQueryValueEx(hKey, name, 0, &keyType, (LPBYTE)value, &keySize);
  RegCloseKey(hKey);
  return Status;
}

static int lpdh_disabled(lua_State *L){
  int i = 0;
  while(1){
    struct lpdh_reg_path_tag *ptr = &LPDH_DISABLED_REG_PATH[i++];
    DWORD status, value;
    if(!ptr->path) break;
    status = lpdh_reg_read_dword(ptr->key, ptr->path, LPDH_DISABLED_REG_NAME, &value);
    if(status == ERROR_SUCCESS){
      if(value) return lpdh_pass(L);
    }
    else if(status != ERROR_FILE_NOT_FOUND)
      return lpdh_error_system(L, status);
  }
  lua_pushboolean(L, 0);
  return 1;
}

static DWORD lpdh_push_en_counter_names(lua_State *L){
  DWORD Status, keyType, keySize = 1;
  int i;
  char tmp, *buf;

  Status = RegQueryValueEx(
   HKEY_PERFORMANCE_DATA, "Counter 009", NULL, 
   &keyType, &tmp, &keySize
  );

  if(ERROR_MORE_DATA == Status){
    for(i = 0; i < 3;++i){
      buf = malloc(keySize);
      if(!buf) return PDH_MEMORY_ALLOCATION_FAILURE;

      Status = RegQueryValueEx(
        HKEY_PERFORMANCE_DATA, "Counter 009", NULL, 
        &keyType, (LPBYTE)buf, &keySize
      );

      if(Status == ERROR_SUCCESS){
        const char *key=buf, *value;
        lua_newtable(L);
        while(*key){
          value = key + strlen(key)+1;
          if(!*value) break;

          lua_pushstring(L, value);
          lua_rawseti(L, -2, atol(key));
        
          lua_pushstring(L, value);
          lua_pushnumber(L, atol(key));
          lua_rawset(L, -3);

          key = value + strlen(value)+1;
        }
        free(buf);
        return ERROR_SUCCESS;
      }
      free(buf);
      if(ERROR_MORE_DATA != Status) break;
    }
  }
  if(ERROR_SUCCESS == Status){// ?
    Status = ERROR_MORE_DATA;
  }
  return Status;
}

static int lpdh_load_en_counter_names(lua_State *L){
  int top = lua_gettop(L) + 1;
  lua_rawgetp(L, LUA_REGISTRYINDEX, LPDH_COUNTER_NAMES);
  if(lua_isnil(L, -1)){
    DWORD Status;
    lua_pop(L, 1);
    Status = lpdh_push_en_counter_names(L);
    if(Status != ERROR_SUCCESS){
      return lpdh_error_system(L, Status);
    }
    lua_settop(L, top);
    lua_pushvalue(L, -1);
    lua_rawsetp(L, LUA_REGISTRYINDEX, LPDH_COUNTER_NAMES);
  }
  assert(top == lua_gettop(L));
  return 1;
}

//}

//{ Path

static PDH_STATUS lpdh_translate_name(lua_State *L, const char *machineName, const char *name, DWORD *pIndex, char** const result){
  DWORD Index;

  if(!pIndex){
    if(!name) return PDH_INVALID_ARGUMENT;
    pIndex = &Index;
  }

  if(name){ // find index
    int top = lua_gettop(L);
    lua_pushstring(L, name);
    lua_rawget(L, -2);
    if(lua_isnumber(L, -1)){
      *pIndex = (DWORD)lua_tonumber(L, -1);
      lua_pop(L, 1);
    }
    else{ // index not found
      size_t size = strlen(name) + 1;
      lua_pop(L, 1);
      assert(top == lua_gettop(L));
      *result = malloc(size);
      if(!*result) return PDH_MEMORY_ALLOCATION_FAILURE;
      strcpy(*result, name);
      return ERROR_SUCCESS;
    }
    assert(top == lua_gettop(L));
  }

  {
    char tmp[2];
    PDH_STATUS Status;
    DWORD size=1;
    Status = PdhLookupPerfNameByIndex(machineName, *pIndex, tmp, &size);
    if(Status != PDH_MORE_DATA) return Status;

    *result = malloc(size);
    if(!*result) return PDH_MEMORY_ALLOCATION_FAILURE;

    return PdhLookupPerfNameByIndex(machineName, *pIndex, *result, &size);
  }
}

static int lpdh_path_translate(lua_State *L){
//! @fixme ????
// \Processor(*)\% Processor Time => \Processor(*#0)\% Processor Time
// do we need remove instance name manually or just relay on PdhMakeCounterPath?
// On XP it works correctly

  PDH_STATUS Status;
  PPDH_COUNTER_PATH_ELEMENTS elements = 0;
  char *objectName = 0, *counterName = 0;

  int n = lpdh_load_en_counter_names(L);
  if(n != 1) return n;

  { // parse
    const char *path = lutil_isudatap(L, 1, LPDH_COUNTER)?lpdh_getcounter_at(L,1)->path:luaL_checkstring(L, 1);
    DWORD size = 0;

    Status = PdhParseCounterPath(path, NULL, &size, 0);
    if(Status != PDH_MORE_DATA){
      goto cleanup;
    }

    elements = (PDH_COUNTER_PATH_ELEMENTS *)malloc(size);

    if(!elements){
      Status = PDH_MEMORY_ALLOCATION_FAILURE;
      goto cleanup;
    }

    Status = PdhParseCounterPath(path, elements, &size, 0);
    if(Status != ERROR_SUCCESS){
      goto cleanup;
    }
  }

  Status = lpdh_translate_name(L, elements->szMachineName, elements->szObjectName, 0, &objectName);
  if(!objectName) goto cleanup;

  Status = lpdh_translate_name(L, elements->szMachineName, elements->szCounterName, 0, &counterName);
  if(!counterName) goto cleanup;

  elements->szCounterName = counterName;
  elements->szObjectName  = objectName;

  lua_pop(L, 1);

  { // make
    char CounterPathBuffer[PDH_MAX_COUNTER_PATH];
    DWORD CounterPathLength;
    CounterPathLength = sizeof(CounterPathBuffer);

    Status = PdhMakeCounterPath(elements, CounterPathBuffer, &CounterPathLength, 0);
    if(Status == ERROR_SUCCESS)
      lua_pushstring(L, CounterPathBuffer);
  }

cleanup:

  if(counterName)
    free(counterName);

  if(objectName)
    free(objectName);

  if(elements)
    free(elements);
  
  if(Status != ERROR_SUCCESS){
    return lpdh_error_pdh(L, Status);
  }

  return 1;
}

static int lpdh_path_expand(lua_State *L){
  const char *path = lutil_isudatap(L, 1, LPDH_COUNTER)?lpdh_getcounter_at(L,1)->path:luaL_checkstring(L, 1);
  DWORD flags = luaL_optinteger(L, 2, 0);
  PDH_STATUS Status;
  PSTR  Paths = NULL;
  DWORD BufferSize = 0;
  
  Status = PdhExpandWildCardPath(NULL, path, Paths, &BufferSize, flags);
  while(Status == PDH_MORE_DATA){
    if(Paths)
      free(Paths);
    Paths = (PSTR)malloc(BufferSize);
    if(!Paths)
      return lpdh_error_pdh(L, PDH_MEMORY_ALLOCATION_FAILURE);
    Status = PdhExpandWildCardPath(NULL, path, Paths, &BufferSize, flags);
  }

  if(Status != ERROR_SUCCESS)
    return lpdh_error_pdh(L, Status);
 
  lua_newtable(L);
  if(Paths){
    PSTR eop = Paths + BufferSize, p = Paths; 
    int i = 0;
    for(;((p != eop) && (*p != '\0')); p += strlen(p) + 1){
      lua_pushstring(L, p);
      lua_rawseti(L, -2, ++i);
    }
  }
  return 1;
}

static int lpdh_translate_element(lua_State *L){
  DWORD Index = 0;
  const char *name = 0;
  char *result = 0;
  int iName = (lua_gettop(L) == 1)?1:2;
  PDH_STATUS Status;
  const char *machineName = (iName == 1)?NULL:luaL_checkstring(L, 1);
 
  if(lua_type(L, iName) == LUA_TNUMBER) Index = (DWORD)lua_tonumber(L, iName);
  else{
    int n;
    name = luaL_checkstring(L, iName);
    n = lpdh_load_en_counter_names(L);
    if(n != 1) return n;
  }
  
  Status = lpdh_translate_name(L, machineName, name, &Index, &result);

  if(ERROR_SUCCESS != Status){
    if(result) free(result);
    return lpdh_error_pdh(L, Status);
  }

  lua_pushstring(L, result);
  if(result) free(result);
  lua_pushnumber(L, Index);
  return 2;
}

//}

//{ PDH Counter

static lpdh_counter_t *lpdh_getcounter_at (lua_State *L, int i) {
  lpdh_counter_t *counter = (lpdh_counter_t *)lutil_checkudatap (L, i, LPDH_COUNTER);
  luaL_argcheck (L, counter != NULL, i, "PDH Counter expected");
  luaL_argcheck (L, !(counter->flags & FLAG_DESTROYED), i, "PDH Counter is destroyed");
  return counter;
}

static int lpdh_counter_push_new(lua_State *L, const char *path, size_t path_len){
  lpdh_counter_t *counter = lutil_newudatap(L, lpdh_counter_t, LPDH_COUNTER);
  memset(counter, 0, sizeof(lpdh_counter_t));
  if(path){
    DWORD Status = PdhValidatePath(path);
    if (Status != ERROR_SUCCESS){
      return lpdh_error_pdh(L, Status);
    }
    if(path_len > PDH_MAX_COUNTER_PATH){
      path_len = PDH_MAX_COUNTER_PATH;
    }
    memcpy(counter->path, path, path_len);
  }
  return 1;
}

static int lpdh_counter_new(lua_State *L){
  size_t path_len = 0;
  const char *path = (lua_gettop(L) == 0)?0:luaL_checklstring(L, 1, &path_len);
  return lpdh_counter_push_new(L, path, path_len);
}

static int lpdh_counter_path(lua_State *L){
  lpdh_counter_t *counter = lpdh_getcounter_at(L, 1);
  lua_pushstring(L, counter->path);
  return 1;
}

static int lpdh_counter_remove(lua_State *L){
  lpdh_counter_t *counter = lpdh_getcounter_at(L, 1);
  if(counter->flags & FLAG_OPEN){
    PDH_STATUS Status = PdhRemoveCounter(counter->handle);
    if(ERROR_SUCCESS != Status){
      return lpdh_error_pdh(L, Status);
    }
    counter->flags &= ~FLAG_OPEN;
  }
  return lpdh_pass(L);
}

#define LPDH_IS_CSTATUS_VALID(S) (((S)==PDH_CSTATUS_VALID_DATA)||((S)==PDH_CSTATUS_NEW_DATA))

#define define_counter_as_XXX(CNAME, TNAME, MNAME)                                          \
static int lpdh_counter_as_##TNAME(lua_State *L){                                           \
  lpdh_counter_t *counter = lpdh_getcounter_at(L, 1);                                       \
  PDH_FMT_COUNTERVALUE value;                                                               \
  DWORD CounterType;                                                                        \
  DWORD scale = luaL_optinteger(L, 2, 0);                                                   \
  PDH_STATUS Status = PdhGetFormattedCounterValue(                                          \
    counter->handle, scale | PDH_FMT_##CNAME, &CounterType,                                 \
    &value                                                                                  \
  );                                                                                        \
                                                                                            \
  if(Status != ERROR_SUCCESS){                                                              \
    return lpdh_error_pdh(L, Status);                                                       \
  }                                                                                         \
                                                                                            \
  if(LPDH_IS_CSTATUS_VALID(value.CStatus)){                                                 \
    lua_pushnumber(L, value.MNAME);                                                         \
    return 1;                                                                               \
  }                                                                                         \
  return lpdh_error_pdh(L, value.CStatus);                                                  \
}

define_counter_as_XXX(DOUBLE, double, doubleValue );
define_counter_as_XXX(LONG,   long  , longValue   );
define_counter_as_XXX(LARGE,  large , largeValue  );

#undef define_counter_as_XXX

#define define_counter_as_XXX_array(CNAME, TNAME, MNAME)                                    \
static int lpdh_counter_as_##TNAME##_array(lua_State *L){                                   \
  lpdh_counter_t *counter = lpdh_getcounter_at(L, 1);                                       \
                                                                                            \
  DWORD bufferSize = 0;                                                                     \
  DWORD itemCount = 0;                                                                      \
  PDH_FMT_COUNTERVALUE_ITEM *items = NULL;                                                  \
  DWORD scale, i = 0;                                                                       \
  PDH_STATUS Status;                                                                        \
  int cbIndex;                                                                              \
  lua_settop(L, 3);                                                                         \
  if(!lua_isnil(L, 3)){                                                                     \
    cbIndex = 3;                                                                            \
    scale = (DWORD)luaL_optinteger(L, 2, 0);                                                \
  }                                                                                         \
  else if(!lua_isnil(L, 2)){                                                                \
    lua_pop(L, 1);                                                                          \
    if(lua_type(L,2) == LUA_TNUMBER){                                                       \
      cbIndex = 0;                                                                          \
      scale = (DWORD)lua_tonumber(L, 2);                                                    \
    }                                                                                       \
    else{                                                                                   \
      cbIndex = 2;                                                                          \
      scale = 0;                                                                            \
    }                                                                                       \
  }                                                                                         \
  else{                                                                                     \
      cbIndex = 0;                                                                          \
      scale = 0;                                                                            \
  }                                                                                         \
                                                                                            \
  Status = PdhGetFormattedCounterArray(                                                     \
    counter->handle, scale | PDH_FMT_##CNAME,                                               \
    &bufferSize, &itemCount, items                                                          \
  );                                                                                        \
                                                                                            \
  if (PDH_MORE_DATA != Status){                                                             \
    return lpdh_error_pdh(L, Status);                                                       \
  }                                                                                         \
                                                                                            \
  items = (PDH_FMT_COUNTERVALUE_ITEM *) malloc(bufferSize);                                 \
  if(!items){                                                                               \
    return lpdh_error_pdh(L, PDH_MEMORY_ALLOCATION_FAILURE);                                \
  }                                                                                         \
                                                                                            \
  Status = PdhGetFormattedCounterArray(                                                     \
    counter->handle, scale | PDH_FMT_##CNAME,                                               \
    &bufferSize, &itemCount, items                                                          \
  );                                                                                        \
                                                                                            \
  if (ERROR_SUCCESS != Status){                                                             \
    free(items);                                                                            \
    return lpdh_error_pdh(L, Status);                                                       \
  }                                                                                         \
                                                                                            \
  lua_newtable(L);                                                                          \
  for(i = 0; i < itemCount; ++i){                                                           \
    PDH_FMT_COUNTERVALUE *value = &items[i].FmtValue;                                       \
    if(!cbIndex){                                                                           \
      lua_newtable(L);                                                                      \
      lua_pushstring(L, items[i].szName);                                                   \
      lua_rawseti(L, -2, 1);                                                                \
      if(LPDH_IS_CSTATUS_VALID(value->CStatus)){                                            \
        lua_pushnumber(L, value->MNAME);                                                    \
      }                                                                                     \
      else{                                                                                 \
        lpdh_error_push(L, LPDH_ERROR_PDH, value->CStatus);                                 \
      }                                                                                     \
      lua_rawseti(L, -2, 2);                                                                \
      lua_rawseti(L, -2, i+1);                                                              \
    }                                                                                       \
    else{                                                                                   \
      int ret, top = lua_gettop(L);                                                         \
      int narg;                                                                             \
      lua_pushvalue(L, cbIndex);                                                            \
      lua_pushnumber(L, i + 1);                                                             \
      lua_pushstring(L, items[i].szName);                                                   \
      if(LPDH_IS_CSTATUS_VALID(value->CStatus)){                                            \
        lua_pushnumber(L, value->MNAME);                                                    \
        narg = 3;                                                                           \
      }                                                                                     \
      else{                                                                                 \
        lua_pushnil(L);                                                                     \
        lpdh_error_push(L, LPDH_ERROR_PDH, value->CStatus);                                 \
        narg = 4;                                                                           \
      }                                                                                     \
      ret = lua_pcall(L, narg, LUA_MULTRET, 0);                                             \
      if(ret){                                                                              \
        free(items);                                                                        \
        return lua_error(L);                                                                \
      }                                                                                     \
      else if(lua_gettop(L) > top){                                                         \
        free(items);                                                                        \
        return lua_gettop(L) - top;                                                         \
      }                                                                                     \
    }                                                                                       \
  }                                                                                         \
  free(items);                                                                              \
  return 1;                                                                                 \
}                                                                                           \

define_counter_as_XXX_array(DOUBLE, double, doubleValue );
define_counter_as_XXX_array(LONG,   long  , longValue   );
define_counter_as_XXX_array(LARGE,  large , largeValue  );

#undef define_counter_as_XXX_array

//}

//{ PDH Query

static lpdh_query_t *lpdh_getquery_at (lua_State *L, int i) {
  lpdh_query_t *qry = (lpdh_query_t *)lutil_checkudatap (L, i, LPDH_QUERY);
  luaL_argcheck (L, qry != NULL, 1, "PDH Query expected");
  luaL_argcheck (L, !(qry->flags & FLAG_DESTROYED), 1, "PDH Query is destroyed");
  return qry;
}

static int lpdh_query_new(lua_State *L){
  lpdh_query_t *qry = lutil_newudatap(L, lpdh_query_t, LPDH_QUERY);
  PDH_STATUS Status;
  memset(qry, 0, sizeof(lpdh_query_t));
  Status = PdhOpenQuery(0, 0, &qry->handle);
  if (Status != ERROR_SUCCESS){
    return lpdh_error_pdh(L, Status);
  }
  return 1;
}

static int lpdh_query_destroy(lua_State *L){
  lpdh_query_t *qry = (lpdh_query_t *)lutil_checkudatap (L, 1, LPDH_QUERY);
  luaL_argcheck (L, qry != NULL, 1, "PDH Query expected");

  if(qry->flags & FLAG_DESTROYED) return 0;

  PdhCloseQuery(qry->handle);

  qry->flags |= FLAG_DESTROYED;
  
  return lpdh_pass(L);
}

static int lpdh_query_destroyed(lua_State *L){
  lpdh_query_t *qry = (lpdh_query_t *)lutil_checkudatap (L, 1, LPDH_QUERY);
  luaL_argcheck (L, qry != NULL, 1, "PDH Query expected");
  lua_pushboolean(L, qry->flags & FLAG_DESTROYED);
  return 1;
}

static int lpdh_query_add_counter(lua_State *L){
  lpdh_query_t *qry = lpdh_getquery_at(L, 1);
  lpdh_counter_t *counter;
  PDH_STATUS Status;
  if(lua_type(L,2) == LUA_TSTRING){
    // create new counter and replace with it string 
    size_t len; const char *path = lua_tolstring(L, 2, &len);
    int n = lpdh_counter_push_new(L, path, len);
    if(!lutil_isudatap(L, -1, LPDH_COUNTER)) return n;
    assert(n == 1);
    lua_remove(L, -2);
  }
  counter = lpdh_getcounter_at(L, 2);
  // if(counter->flags & FLAG_OPEN) return lpdh_error_library(L, LPDH_COUNTER_ALREADY_OPEN); 
  Status = PdhAddCounter(qry->handle, counter->path, 0, &counter->handle);
  if (Status != ERROR_SUCCESS){
    return lpdh_error_pdh(L, Status);
  }
  counter->flags |= FLAG_OPEN;
  lua_settop(L, 2); // leave counter on top
  return 1;
}

static int lpdh_query_collect(lua_State *L){
  lpdh_query_t *qry = lpdh_getquery_at(L, 1);
  PDH_STATUS Status = PdhCollectQueryData(qry->handle);
  if (Status != ERROR_SUCCESS){
    return lpdh_error_pdh(L, Status);
  }
  lua_settop(L, 1);
  return 1;
}

//}

static int lpdh_sleep(lua_State *L){
  DWORD n = luaL_checkinteger(L, 1);
  Sleep(n);
  return 0;
}

//{ psapi (XP+)

#define SET_FIELD(L, INFO, N) {lua_pushnumber(L, ((INFO).N)); lua_setfield(L, -2, #N);}

#include <Psapi.h>

#ifdef _MSC_VER
#  include <Winternl.h>
#else

#include <ntdef.h>

typedef enum _PROCESSINFOCLASS {
    ProcessBasicInformation = 0,
    ProcessWow64Information = 26
} PROCESSINFOCLASS;

typedef VOID (NTAPI *PPS_POST_PROCESS_INIT_ROUTINE) ( VOID );

typedef struct _PEB_LDR_DATA {
    BYTE Reserved1[8];
    PVOID Reserved2[3];
    LIST_ENTRY InMemoryOrderModuleList;
} PEB_LDR_DATA, *PPEB_LDR_DATA;

typedef struct _RTL_USER_PROCESS_PARAMETERS {
    BYTE Reserved1[16];
    PVOID Reserved2[10];
    UNICODE_STRING ImagePathName;
    UNICODE_STRING CommandLine;
} RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;

typedef struct _PEB {
    BYTE Reserved1[2];
    BYTE BeingDebugged;
    BYTE Reserved2[1];
    PVOID Reserved3[2];
    PPEB_LDR_DATA Ldr;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
    BYTE Reserved4[104];
    PVOID Reserved5[52];
    PPS_POST_PROCESS_INIT_ROUTINE PostProcessInitRoutine;
    BYTE Reserved6[128];
    PVOID Reserved7[1];
    ULONG SessionId;
} PEB, *PPEB;

typedef struct _PROCESS_BASIC_INFORMATION {
    PVOID Reserved1;
    PPEB PebBaseAddress;
    PVOID Reserved2[2];
    ULONG_PTR UniqueProcessId;
    PVOID Reserved3;
} PROCESS_BASIC_INFORMATION;

typedef PROCESS_BASIC_INFORMATION *PPROCESS_BASIC_INFORMATION;

#endif 

typedef struct _PROCESS_BASIC_INFORMATION_DECODE{
  ULONG_PTR ExitStatus;
  PPEB PebBaseAddress;
  ULONG_PTR AffinityMask;
  ULONG_PTR BasePriority;
  ULONG_PTR UniqueProcessId;
  ULONG_PTR InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION_DECODE;
typedef PROCESS_BASIC_INFORMATION_DECODE *PPROCESS_BASIC_INFORMATION_DECODE;

ASSERT_SAME_SIZE(PROCESS_BASIC_INFORMATION_DECODE, PROCESS_BASIC_INFORMATION);
ASSERT_SAME_OFFSET(PROCESS_BASIC_INFORMATION_DECODE, ExitStatus,                   PROCESS_BASIC_INFORMATION, Reserved1       );
ASSERT_SAME_OFFSET(PROCESS_BASIC_INFORMATION_DECODE, PebBaseAddress,               PROCESS_BASIC_INFORMATION, PebBaseAddress  );
ASSERT_SAME_OFFSET(PROCESS_BASIC_INFORMATION_DECODE, AffinityMask,                 PROCESS_BASIC_INFORMATION, Reserved2       );
ASSERT_SAME_OFFSET(PROCESS_BASIC_INFORMATION_DECODE, UniqueProcessId,              PROCESS_BASIC_INFORMATION, UniqueProcessId );
ASSERT_SAME_OFFSET(PROCESS_BASIC_INFORMATION_DECODE, InheritedFromUniqueProcessId, PROCESS_BASIC_INFORMATION, Reserved3       );

typedef NTSTATUS (NTAPI *pfnNtQueryInformationProcess)(
  IN  HANDLE ProcessHandle,
  IN  PROCESSINFOCLASS ProcessInformationClass,
  OUT PVOID ProcessInformation,
  IN  ULONG ProcessInformationLength,
  OUT PULONG ReturnLength    OPTIONAL
);

static HMODULE hNtDll = NULL;

static pfnNtQueryInformationProcess pNtQueryInformationProcess = NULL;

//{ Utils

static DWORD lpsapi_set_process_privilege(HANDLE hProcess, LPCTSTR Privilege, BOOL bEnable){
  HANDLE hToken;
  TOKEN_PRIVILEGES tkp;
  DWORD status;

  // �������� ����� ��������
  if(!OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    return GetLastError();

  // �������� LUID ��� �������� ����������
  if(!LookupPrivilegeValue(NULL, Privilege, &tkp.Privileges[0].Luid)){
    status = GetLastError();
    CloseHandle(hProcess);
    return status;
  }

  tkp.PrivilegeCount = 1;  //���-�� ����������
  tkp.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0; //�������� ����������

  // ��������� �������� ���������� ��� ��������
  if(!AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0)){
    status = GetLastError();
    CloseHandle(hProcess);
    return status;
  }

  CloseHandle(hToken);
  return ERROR_SUCCESS;
}

static HMODULE lpsapi_load_ntdll_(){
  HMODULE h;
  if(hNtDll) return hNtDll;

  h = LoadLibrary("ntdll.dll");
  if(!h) return NULL;

  pNtQueryInformationProcess = (pfnNtQueryInformationProcess)GetProcAddress(h, "NtQueryInformationProcess");
  if(!pNtQueryInformationProcess){
    FreeLibrary(h);
    return NULL;
  }
  return hNtDll=h;
}

static time_t filetime_to_timet(const FILETIME *ft){
  ULARGE_INTEGER ull;
  ull.LowPart = ft->dwLowDateTime;
  ull.HighPart = ft->dwHighDateTime;
  return ull.QuadPart / 10000000ULL - 11644473600ULL;
}

static ULONGLONG filetime_to_msec(const FILETIME *ft){
  ULARGE_INTEGER ull;
  ull.LowPart = ft->dwLowDateTime;
  ull.HighPart = ft->dwHighDateTime;
  return ull.QuadPart / 10000ULL;
}

//}

static int lpsapi_get_performance_info(lua_State *L){
  PERFORMANCE_INFORMATION info;
  BOOL ret = GetPerformanceInfo(&info,sizeof(info));
  if(!ret){
    return lpdh_error_system(L, GetLastError());
  }
  lua_newtable(L);
  SET_FIELD(L, info, CommitTotal       );
  SET_FIELD(L, info, CommitLimit       );
  SET_FIELD(L, info, CommitPeak        );
  SET_FIELD(L, info, PhysicalTotal     );
  SET_FIELD(L, info, PhysicalAvailable );
  SET_FIELD(L, info, SystemCache       );
  SET_FIELD(L, info, KernelTotal       );
  SET_FIELD(L, info, KernelPaged       );
  SET_FIELD(L, info, KernelNonpaged    );
  SET_FIELD(L, info, PageSize          );
  SET_FIELD(L, info, HandleCount       );
  SET_FIELD(L, info, ProcessCount      );
  SET_FIELD(L, info, ThreadCount       );
  return 1;
}

static int lpsapi_enum_processes(lua_State *L){
  //! @todo implement callback
  DWORD *pids, bytes, pids_size = 128 * sizeof(DWORD);
  int i, n;
  int cbIndex = lua_isnone(L, 1)?0:1;

  while(1){
    pids = (DWORD*)malloc(pids_size);
    if(!pids){
      return lpdh_error_system(L, ERROR_NOT_ENOUGH_MEMORY);
    }

    if(!EnumProcesses(pids, pids_size, &bytes)){
      DWORD status = GetLastError();
      free(pids);
      return lpdh_error_system(L, status);
    }

    if(bytes >= pids_size){
      free(pids);
      pids_size = ((pids_size / sizeof(DWORD)) * 1.5) * sizeof(DWORD);
      continue;
    }

    n = bytes / sizeof(DWORD);

    if(cbIndex){
      int ret; int top = lua_gettop(L);
      for(i = 0;i<n;++i){
        assert(top == lua_gettop(L));
        lua_pushvalue(L, cbIndex);
        lua_pushnumber(L, i);
        lua_pushnumber(L, pids[i]);
        ret = lua_pcall(L, 2, LUA_MULTRET, 0);
        if(ret){
          free(pids);
          return lua_error(L);
        }
        if(lua_gettop(L) > top){
          free(pids);
          return lua_gettop(L) - top;
        }
      }
      return lpdh_pass(L);
    }

    lua_newtable(L);
    for(i = 0;i<n;++i){
      lua_pushnumber(L, pids[i]);
      lua_rawseti(L, -2, i + 1);
    }
    free(pids);
    return 1;
  }
}

//{ psapi process

static const char *LPSAPI_PROCESS   = "PSAPI Process";

typedef struct lpsapi_process_tag{
  FLAG_TYPE flags;
  HANDLE    handle;
} lpsapi_process_t;

static lpsapi_process_t *lpsapi_getprocess_at (lua_State *L, int i, int checkOpen){
  lpsapi_process_t *process = (lpsapi_process_t *)lutil_checkudatap (L, i, LPSAPI_PROCESS);
  luaL_argcheck (L, process != NULL, 1, "PSAPI Process expected");
  luaL_argcheck (L, !(process->flags & FLAG_DESTROYED), 1, "PSAPI Process is destroyed");
  if(checkOpen == 0)luaL_argcheck (L, !(process->flags & FLAG_OPEN), 1, "PSAPI Process is opened");
  if(checkOpen == 1)luaL_argcheck (L,  (process->flags & FLAG_OPEN), 1, "PSAPI Process is closed");
  return process;
}

static lpsapi_process_t *lpsapi_process_alloc(lua_State *L){
  lpsapi_process_t *process = lutil_newudatap(L, lpsapi_process_t, LPSAPI_PROCESS);
  memset(process, 0, sizeof(lpsapi_process_t));
  return process;
}

static int lpsapi_process_open(lua_State *L){
  lpsapi_process_t *process = lpsapi_getprocess_at(L, 1, -1);
  DWORD pid, accessFlags = PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | SYNCHRONIZE;
  int top = lua_gettop(L);

  if(process->flags & FLAG_OPEN){
    assert(!!process->handle);
    CloseHandle(process->handle);
    process->handle = 0;
    process->flags &= ~FLAG_OPEN;
  }

  if(top == 1){
    process->handle = GetCurrentProcess();
  }
  else{
    pid = (DWORD)luaL_checkinteger(L, 2);
    process->handle = OpenProcess(accessFlags, FALSE, pid);
  }

  if(!process->handle) return lpdh_error_system(L, GetLastError());
  process->flags |= FLAG_OPEN;

  lua_settop(L, 1);
  return 1;
}

static int lpsapi_process_new(lua_State *L){
  lpsapi_process_t *process = lpsapi_process_alloc(L);
  (void)process;
  lua_insert(L, 1);
  return lpsapi_process_open(L);
}

static int lpsapi_process_destroy(lua_State *L){
  lpsapi_process_t *process = (lpsapi_process_t *)lutil_checkudatap (L, 1, LPSAPI_PROCESS);
  luaL_argcheck (L, process != NULL, 1, "PSAPI Process expected");

  if(process->flags & FLAG_DESTROYED) return 0;

  if(process->flags & FLAG_OPEN){
    assert(!!process->handle);
    CloseHandle(process->handle);
    process->handle = 0;
    process->flags &= ~FLAG_OPEN;
  }

  process->flags |= FLAG_DESTROYED;

  return lpdh_pass(L);
}

static int lpsapi_process_destroyed(lua_State *L){
  lpsapi_process_t *process = (lpsapi_process_t *)lutil_checkudatap (L, 1, LPSAPI_PROCESS);
  luaL_argcheck (L, process != NULL, 1, "PSAPI Process expected");
  lua_pushboolean(L, process->flags & FLAG_DESTROYED);
  return 1;
}

static int lpsapi_process_close(lua_State *L){
  lpsapi_process_t *process = lpsapi_getprocess_at(L, 1, -1);
  lua_settop(L, 1);
  if(process->flags & FLAG_OPEN){
    assert(!!process->handle);
    CloseHandle(process->handle);
    process->handle = 0;
    process->flags &= ~FLAG_OPEN;
  }
  return 1;
}

static int lpsapi_process_closed(lua_State *L){
  lpsapi_process_t *process = lpsapi_getprocess_at(L, 1, -1);
  lua_pushboolean(L, !(process->flags & FLAG_OPEN));
  return 1;
}

static int lpsapi_process_memory_info(lua_State *L){
  lpsapi_process_t *process = lpsapi_getprocess_at(L, 1, 1);
  PROCESS_MEMORY_COUNTERS_EX counters;
  DWORD size = sizeof(counters);

  memset(&counters, 0, sizeof(counters));
  counters.cb = size;

  if(!GetProcessMemoryInfo(process->handle, (PROCESS_MEMORY_COUNTERS*)&counters, size)){
    return lpdh_error_system(L, GetLastError());
  }

  lua_newtable(L);
  SET_FIELD(L, counters, PageFaultCount);
  SET_FIELD(L, counters, PeakWorkingSetSize);
  SET_FIELD(L, counters, WorkingSetSize);
  SET_FIELD(L, counters, QuotaPeakPagedPoolUsage);
  SET_FIELD(L, counters, QuotaPagedPoolUsage);
  SET_FIELD(L, counters, QuotaPeakNonPagedPoolUsage);
  SET_FIELD(L, counters, QuotaNonPagedPoolUsage);
  SET_FIELD(L, counters, PagefileUsage);
  SET_FIELD(L, counters, PeakPagefileUsage);
  SET_FIELD(L, counters, PrivateUsage);
  return 1;
}

static int lpsapi_process_module_name(lua_State *L){
  lpsapi_process_t *process = lpsapi_getprocess_at(L, 1, 1);
  char path[MAX_PATH + 1];
  DWORD size = sizeof(path);
  size = GetModuleFileNameEx(process->handle, NULL, path, size);
  if(size){
    lua_pushlstring(L, path, size);
    return 1;
  }
  return lpdh_error_system(L, GetLastError());
}

static int lpsapi_process_base_name(lua_State *L){
  lpsapi_process_t *process = lpsapi_getprocess_at(L, 1, 1);
  char path[MAX_PATH + 1];
  DWORD size = sizeof(path);
  size = GetModuleBaseName(process->handle, NULL, path, size);
  if(size){
    lua_pushlstring(L, path, size);
    return 1;
  }
  return lpdh_error_system(L, GetLastError());
}

static int lpsapi_process_command_line(lua_State *L){
  lpsapi_process_t *process = lpsapi_getprocess_at(L, 1, 1);
  NTSTATUS Status;
  SIZE_T size;
  PPEB ppeb;

  {
    PROCESS_BASIC_INFORMATION pbi;
    ULONG size;
    Status = pNtQueryInformationProcess(
      process->handle, ProcessBasicInformation,
      &pbi, sizeof(pbi), &size
    );
    if(Status != ERROR_SUCCESS){
      return lpdh_error_system(L, Status);
    }
    ppeb = pbi.PebBaseAddress;
  }

  if(ppeb){
    PRTL_USER_PROCESS_PARAMETERS pupp;
    {
      PEB peb;
      if(!ReadProcessMemory(process->handle, ppeb, &peb, sizeof(peb), &size)){
        return lpdh_error_system(L, GetLastError());
     }
      pupp = peb.ProcessParameters;
    }
    if(pupp){
      RTL_USER_PROCESS_PARAMETERS upp;
      WCHAR *buffer;
      char *cmd_line;
      if(!ReadProcessMemory(process->handle, pupp, &upp, sizeof(upp), &size)){
        return lpdh_error_system(L, GetLastError());
      }
      size = upp.CommandLine.Length;
      buffer = (WCHAR*)malloc(size);
      if(!buffer){
        return lpdh_error_system(L, ERROR_NOT_ENOUGH_MEMORY);
      }
      cmd_line = (char*)malloc(size / sizeof(WCHAR));
      if(!cmd_line){
        free(buffer);
        return lpdh_error_system(L, ERROR_NOT_ENOUGH_MEMORY);
      }
      if(!ReadProcessMemory(process->handle, upp.CommandLine.Buffer, buffer, size, &size)){
        free(cmd_line);
        free(buffer);
        return lpdh_error_system(L, GetLastError());
      }
      size = (size/sizeof(WCHAR));
      WideCharToMultiByte(CP_ACP, 0, buffer, size, cmd_line, size, NULL, NULL);
      free(buffer);
      lua_pushlstring(L, cmd_line, size);
      free(cmd_line);
      return 1;
    }
  }
  return 0;
}

static int lpsapi_process_times(lua_State *L){
  lpsapi_process_t *process = lpsapi_getprocess_at(L, 1, 1);
  FILETIME creationTime, exitTime, kernelTime, userTime;
  BOOL ret = GetProcessTimes(process->handle, &creationTime, &exitTime, &kernelTime, &userTime);
  if(!ret){
    return lpdh_error_system(L, GetLastError());
  }
  lua_pushnumber(L, filetime_to_timet(&creationTime));
  lua_pushnumber(L, filetime_to_timet(&exitTime));
  lua_pushnumber(L, filetime_to_msec(&kernelTime));
  lua_pushnumber(L, filetime_to_msec(&userTime));
  return 4;
}

static int lpsapi_process_pid(lua_State *L){
  lpsapi_process_t *process = lpsapi_getprocess_at(L, 1, 1);
  lua_pushnumber(L, GetProcessId(process->handle));
  return 1;
}

static int lpsapi_process_parent_pid(lua_State *L){
  lpsapi_process_t *process = lpsapi_getprocess_at(L, 1, 1);
  DWORD size;
  PROCESS_BASIC_INFORMATION pbi;
  PPROCESS_BASIC_INFORMATION_DECODE pbid = (PPROCESS_BASIC_INFORMATION_DECODE) &pbi;
  NTSTATUS Status = pNtQueryInformationProcess(
    process->handle, ProcessBasicInformation,
    &pbi, sizeof(pbi), &size
  );
  if(Status != ERROR_SUCCESS){
    return lpdh_error_system(L, Status);
  }
  lua_pushnumber(L, pbid->InheritedFromUniqueProcessId);
  return 1;
}

static int lpsapi_process_exit_code(lua_State *L){
  lpsapi_process_t *process = lpsapi_getprocess_at(L, 1, 1);
  DWORD code;
  BOOL ret = GetExitCodeProcess(process->handle, &code);
  if (!ret) {
    return lpdh_error_system(L, GetLastError());
  }

  lua_pushinteger(L, code);
  return 1;
}

static int lpsapi_process_alive(lua_State *L){
  lpsapi_process_t *process = lpsapi_getprocess_at(L, 1, 1);
  DWORD ret = WaitForSingleObject(process->handle, 0);
  lua_pushboolean(L, (ret == WAIT_TIMEOUT) ? 1 : 0);
  lua_pushinteger(L, ret);
  return 2;
}

//}

#undef SET_FIELD
//}

static int lpdh_counter_dialog(lua_State *L){
  PDH_STATUS Status;
  PDH_BROWSE_DLG_CONFIG BrowseDlgData;
  char CounterPathBuffer[PDH_MAX_COUNTER_PATH];
  size_t CounterPathLength;
  const char *BROWSE_DIALOG_CAPTION = luaL_optstring(L, 1, "Select a counter to monitor.");

  ZeroMemory(&CounterPathBuffer, sizeof(CounterPathBuffer));
  ZeroMemory(&BrowseDlgData, sizeof(PDH_BROWSE_DLG_CONFIG));

  BrowseDlgData.bIncludeInstanceIndex = FALSE;
  BrowseDlgData.bSingleCounterPerAdd = TRUE;
  BrowseDlgData.bSingleCounterPerDialog = TRUE;
  BrowseDlgData.bLocalCountersOnly = FALSE;
  BrowseDlgData.bWildCardInstances = TRUE;
  BrowseDlgData.bHideDetailBox = TRUE;
  BrowseDlgData.bInitializePath = FALSE;
  BrowseDlgData.bDisableMachineSelection = FALSE;
  BrowseDlgData.bIncludeCostlyObjects = FALSE;
  BrowseDlgData.bShowObjectBrowser = FALSE;
  BrowseDlgData.hWndOwner = NULL;
  BrowseDlgData.szReturnPathBuffer = CounterPathBuffer;
  BrowseDlgData.cchReturnPathLength = PDH_MAX_COUNTER_PATH;
  BrowseDlgData.pCallBack = NULL;
  BrowseDlgData.dwCallBackArg = 0;
  BrowseDlgData.CallBackStatus = ERROR_SUCCESS;
  BrowseDlgData.dwDefaultDetailLevel = PERF_DETAIL_WIZARD;
  BrowseDlgData.szDialogBoxCaption = (char*)BROWSE_DIALOG_CAPTION;

  Status = PdhBrowseCounters(&BrowseDlgData);

  if (Status != ERROR_SUCCESS){
      if (Status == PDH_DIALOG_CANCELLED){
        lua_pushboolean(L, 0);
        return 1;
      }
      return lpdh_error_pdh(L, Status);
  }

  CounterPathLength = strlen(CounterPathBuffer);
  if(0 == CounterPathLength){
    lua_pushboolean(L, 0);
    return 1;
  }

  return lpdh_counter_push_new(L, CounterPathBuffer, CounterPathLength);
}

static const struct luaL_Reg lpdh_lib[] = {
  {"disabled",           lpdh_disabled                },
  {"sleep",              lpdh_sleep                   },
  {"query",              lpdh_query_new               },
  {"counter",            lpdh_counter_new             },
  {"counter_dialog",     lpdh_counter_dialog          },
  {"translate_path",     lpdh_path_translate          },
  {"translate_name",     lpdh_translate_element       },
  {"expand_path",        lpdh_path_expand             },
  {"counter_names",      lpdh_load_en_counter_names   },

  {NULL, NULL}
};

static const struct luaL_Reg lpdh_query_meth[] = {
  {"__gc",         lpdh_query_destroy       },
  {"destroy",      lpdh_query_destroy       },
  {"destroyed",    lpdh_query_destroyed     },
  {"add_counter",  lpdh_query_add_counter   },
  {"collect",      lpdh_query_collect       },
  {NULL, NULL}
};

static const struct luaL_Reg lpdh_counter_meth[] = {
  {"__gc",             lpdh_counter_remove             },
  {"path",             lpdh_counter_path               },
  {"translate",        lpdh_path_translate             },
  {"expand",           lpdh_path_expand                },
  {"remove",           lpdh_counter_remove             },
  {"as_double",        lpdh_counter_as_double          },
  {"as_long",          lpdh_counter_as_long            },
  {"as_large",         lpdh_counter_as_large           },
  {"as_double_array",  lpdh_counter_as_double_array    },
  {"as_long_array",    lpdh_counter_as_long_array      },
  {"as_large_array",   lpdh_counter_as_large_array     },
  {NULL, NULL}
};

static const struct luaL_Reg lpdh_error_meth[] = {
  {"no",              lpdh_error_number           },
  {"msg",             lpdh_error_message          },
  {"mnemo",           lpdh_error_mnemo            },
  {"type",            lpdh_error_type             },
  {"__tostring",      lpdh_error_tostring         },

  {NULL, NULL}
};

static const struct luaL_Reg lpsapi_lib[] = {
  {"process",            lpsapi_process_new           },
  {"performance_info",   lpsapi_get_performance_info  },
  {"enum_processes",     lpsapi_enum_processes        },

  {NULL, NULL}
};

static const struct luaL_Reg lpsapi_process_meth[] = {
  {"__gc",         lpsapi_process_destroy       },
  {"destroy",      lpsapi_process_destroy       },
  {"destroyed",    lpsapi_process_destroyed     },
  {"close",        lpsapi_process_close         },
  {"closed",       lpsapi_process_closed        },
  {"open",         lpsapi_process_open          },
  {"module_name",  lpsapi_process_module_name   },
  {"base_name",    lpsapi_process_base_name     },
  {"memory_info",  lpsapi_process_memory_info   },
  {"command_line", lpsapi_process_command_line  },
  {"times",        lpsapi_process_times         },
  {"pid",          lpsapi_process_pid           },
  {"parent_pid",   lpsapi_process_parent_pid    },
  {"exit_code",    lpsapi_process_exit_code     },
  {"alive",        lpsapi_process_alive         },
  {NULL, NULL}
};

LPDH_EXPORT int luaopen_pdh_psapi(lua_State *L){
  int top = lua_gettop(L);
  lpsapi_set_process_privilege(GetCurrentProcess(), "SeDebugPrivilege", TRUE);
  lpsapi_load_ntdll_();

  lutil_createmetap(L, LPSAPI_PROCESS, lpsapi_process_meth,   0);
  lua_settop(L, top);

  lua_newtable(L);
  luaL_setfuncs(L, lpsapi_lib, 0);

  assert(top+1 == lua_gettop(L));
  return 1;
}

LPDH_EXPORT int luaopen_pdh_core(lua_State*L){
  int top = lua_gettop(L);

  lutil_createmetap(L, LPDH_QUERY,   lpdh_query_meth,   0);
  lutil_createmetap(L, LPDH_COUNTER, lpdh_counter_meth, 0);
  lutil_createmetap(L, LPDH_ERROR,   lpdh_error_meth,   0);
  lua_settop(L, top);

  lua_newtable(L);
  luaL_setfuncs(L, lpdh_lib, 0);
#define LPDH_PROCEED_ERROR_NODE(VALUE) lua_pushnumber(L, VALUE); lua_setfield(L, -2, #VALUE);
#define LPDH_PROCEED_CONST_NODE LPDH_PROCEED_ERROR_NODE
  LPDH_PROCEED_ERROR()
  LPDH_PROCEED_CONST()
#undef LPDH_PROCEED_CONST_NODE
#undef LPDH_PROCEED_ERROR_NODE

  assert(top+1 == lua_gettop(L));
  if(1 == luaopen_pdh_psapi(L)){
    assert(top+2 == lua_gettop(L));
    lua_setfield(L, -2, "psapi");
  }
  lua_settop(L, top+1);

  return 1;
}