#include "luaEngine.h"

namespace Flux {
    void LuaEngine::init() {
        lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::math);

        lua["print"] = [](sol::variadic_args args) {
            std::string full_msg = "";
            for (auto v : args) {
                std::string s = v.as<std::string>();
                full_msg += s + " ";
            }

            Output::addLog(full_msg);
	    };
    }

   void LuaEngine::runScript(const std::string& code) {
        auto load_result = lua.load(code);

        if (!load_result.valid()) {
            sol::error err = load_result;
            Output::addLog("LUA SYNTAX ERROR: " + std::string(err.what()));
            isRunning = false;
            stop();
            return;
        }

        sol::protected_function_result exec_result = load_result();

        if (exec_result.valid()) {
            luaOnStart = lua["onStart"];
            luaOnUpdate = lua["onUpdate"];
            luaOnEnd = lua["onEnd"];

            if (luaOnStart.valid()) {
                sol::protected_function_result start_res = luaOnStart();
                if (!start_res.valid()) {
                    sol::error err = start_res;
                    Output::addLog("LUA START ERROR: " + std::string(err.what()));
                }
            }
        } else {
            sol::error err = exec_result;
            Output::addLog("LUA EXECUTION ERROR: " + std::string(err.what()));
            isRunning = false;
            stop();
        }
    }

   void LuaEngine::step() {
        if (!luaOnUpdate.valid()) return;

        sol::protected_function_result result = luaOnUpdate();
    
        if (!result.valid()) {
            sol::error err = result;
            Output::addLog("LUA RUNTIME ERROR (Update): " + std::string(err.what()));
            
            luaOnUpdate = sol::lua_nil; 
            isRunning = false; 
            stop();
        }
    }

    void LuaEngine::stop() {
        if (luaOnEnd.valid()) {
            luaOnEnd();

            sol::protected_function_result result = luaOnEnd();
        
            if (!result.valid()) {
                sol::error err = result;
                Output::addLog("LUA RUNTIME ERROR (End): " + std::string(err.what()));

                isRunning = false;
                stop();
            }
        }
    }

    void LuaEngine::runAllScriptsInFolder(const std::string& folderPath) {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(folderPath)) {
            if (entry.path().extension() == ".lua") {
                std::ifstream ifs(entry.path());
                if (ifs.is_open()) {
                    std::string code{(std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>())};
                    runScript(code);
                    ifs.close();
                }
            }
        }
    }
}