#include "script_lua.h"
#include <sys/stat.h>

namespace {

int osex_mkdir(lua_State *L)
{
	const char *name = luaL_checkstring(L, 1);
	int status = mkdir(name, 0777);
	return luaL_fileresult(L, status == 0, name);
}

const luaL_Reg osexlib[] = {
	{"mkdir", osex_mkdir},
	{nullptr, nullptr}
};

}	// namespace

void Lua::LuaDeleter::operator()(lua_State *lua)
{
	lua_close(lua);
}

bool Lua::init()
{
	m_lua.reset();
	// TODO: memory alloc hook
	lua_State *raw_lua = luaL_newstate();
	if (raw_lua == nullptr) {
		return false;
	}
	m_lua.reset(raw_lua);


	lua_State *L = m_lua.get();
	luaL_openlibs(L);
	luaL_newlib(L, osexlib);
	lua_setglobal(L, "osex");

	return true;
}


bool Lua::eval_file(const char *path)
{
	lua_State *L = m_lua.get();

	int ret = luaL_dofile(L, path);
	if (ret == LUA_OK) {
		return true;
	}
	else {
		printf("luaL_dofile: %d\n", ret);
		if (lua_gettop(L) >= 1) {
			printf("%s\n", lua_tostring(L, -1));
		}
		return false;
	}
}
