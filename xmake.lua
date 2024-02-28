set_xmakever("2.8.6")
add_rules("mode.release", "mode.debug")
includes("scripts/xmake_func.lua")

if is_arch("x64", "x86_64", "arm64") then
	if is_mode("debug") then
		set_targetdir("bin/debug")
	else
		set_targetdir("bin/release")
	end
	includes("src")
else
	target("illegal_env")
	set_kind("phony")
	on_load(function(target)
		utils.error("Illegal environment. Please check your compiler, architecture or platform.")
	end)
	target_end()
end
