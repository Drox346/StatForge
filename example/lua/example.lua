local ffi = require("ffi")

ffi.cdef([[
typedef int SF_ErrorCode;
typedef struct SF_Engine SF_Engine;

SF_Engine* sf_create_engine(void);
void sf_destroy_engine(SF_Engine* engine);

SF_ErrorCode sf_create_value_cell(SF_Engine* engine, const char* name, double value);
SF_ErrorCode sf_get_cell_value(SF_Engine* engine, const char* name, double* out_value);

const char* sf_last_error(void);
]])

local lib = ffi.load("build/src/libStatForge.so")

local SF_OK = 0

local engine = lib.sf_create_engine()
if engine == nil then
    io.write("Error: could not create engine\n")
    os.exit(1)
end

local lifeCellName = "life"
lib.sf_create_value_cell(engine, lifeCellName, 105.0)

local cellValue = ffi.new("double[1]")
local err = lib.sf_get_cell_value(engine, lifeCellName, cellValue)

if tonumber(err) ~= SF_OK then
    io.write(string.format("Error: %s\n", ffi.string(lib.sf_last_error())))
else
    io.write(string.format("Val: %.0f\n", tonumber(cellValue[0])))
end

lib.sf_create_value_cell(engine, lifeCellName, 100.0)
io.write(string.format("Error: %s\n", ffi.string(lib.sf_last_error())))

lib.sf_destroy_engine(engine)
