target("lmdb")
_config_project({
    project_kind = "shared"
})
add_files("*.c", "*.cpp")
if is_plat("windows") then
    add_syslinks("Advapi32", {public = true})    
end
target_end()