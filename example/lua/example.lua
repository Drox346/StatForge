local ffi = require("ffi")

ffi.cdef([[
typedef int SF_ErrorCode;
typedef struct SF_Engine SF_Engine;

SF_Engine* sf_create_engine(void);
void sf_destroy_engine(SF_Engine* engine);

SF_ErrorCode sf_create_value_node(SF_Engine* engine, const char* name, double value);
SF_ErrorCode sf_get_node_value(SF_Engine* engine, const char* name, double* out_value);

const char* sf_last_error(void);
]])

local lib = ffi.load("build/src/libStatForge.so")

local SF_OK = 0

local engine = lib.sf_create_engine()
if engine == nil then
    io.write("Error: could not create engine\n")
    os.exit(1)
end

local lifeNodeName = "life"
lib.sf_create_value_node(engine, lifeNodeName, 105.0)

local nodeValue = ffi.new("double[1]")
local err = lib.sf_get_node_value(engine, lifeNodeName, nodeValue)

if tonumber(err) ~= SF_OK then
    io.write(string.format("Error: %s\n", ffi.string(lib.sf_last_error())))
else
    io.write(string.format("Val: %.0f\n", tonumber(nodeValue[0])))
end

lib.sf_create_value_node(engine, lifeNodeName, 100.0)
io.write(string.format("Error: %s\n", ffi.string(lib.sf_last_error())))

lib.sf_destroy_engine(engine)
