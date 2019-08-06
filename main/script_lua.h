#ifndef SCRIPT_LUA_H_
#define SCRIPT_LUA_H_

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <memory>

class Lua {
public:
	Lua() = default;
	~Lua() = default;
	bool init();

	lua_State *get() { return m_lua.get(); }
	bool eval_file(const char *path);

private:
	struct LuaDeleter {
		void operator()(lua_State *L);
	};

	std::unique_ptr<lua_State, LuaDeleter> m_lua;
};

#endif // SCRIPT_LUA_H_
