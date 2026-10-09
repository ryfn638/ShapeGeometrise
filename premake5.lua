-- premake5.lua (generic)
-- Run createproject.bat to make build/<name>.slnx.
--
-- Nothing in here is repo specific: copy it into any repo as is. The repo's own
-- settings live in workspace.lua next to it, and each project's in <folder>/project.lua.
--
-- Every folder is a project, named after the folder:
--   <folder>/project.lua   one of our projects
--   sub/<repo>             a submodule, built as a static lib
-- Submodules inside submodules are pulled up too, once each (our own sub/ copy wins).
--
-- A project gets every source file in its folder, except sub/, build/ and tests/.
-- Solution Explorer shows the same folders as on disk.
-- Every header folder (and the project folder itself) is on the project's own include path.
-- Headers in any include/ folder are what other projects see when they link it.
--
-- workspace.lua returns a table, and everything in it is optional:
--   return {
--     name         = "MyRepo",        -- default: the repo folder's name
--     startproject = "MyApp",         -- default: the first ConsoleApp/WindowedApp
--     cppdialect   = "C++20",         -- default "C++20"
--     characterset = "Unicode",       -- default "Unicode"
--     packages     = { ... },         -- libraries that aren't a folder here, see below
--     setup        = function() end,  -- any extra workspace-wide premake calls
--   }
--
-- A package is linked by name like any project. It's either a function (system libs,
-- anything already installed), or a table that gets downloaded into packages/<dir>:
--   gtest = {
--     url   = "https://.../gtest.nupkg", -- .zip/.nupkg are unzipped, .exe is run as a 7-Zip SFX
--     dir   = "gtest-1.8.1.8",           -- folder under packages/
--     type  = "zip",                     -- only needed when the url has no extension (zip or exe)
--     use   = function(dir) end,         -- premake calls for whoever links it, dir is absolute
--   }
-- Set <NAME>_DIR (e.g. GTEST_DIR) to use an install you already have instead of downloading.
--
-- project.lua just returns a table, and everything in it is optional:
--   return {
--     kind  = "ConsoleApp",           -- default "StaticLib"
--     links = { "Core", "gtest" },    -- also pulls in whatever those link
--     setup = function() end,         -- any extra premake calls
--   }
-- A submodule can have a project.lua too. Without one, it links its own sub/ repos.
--
-- Source files are globbed, so re-run createproject.bat after adding or removing files.

local SOURCES = { "h", "hpp", "inl", "c", "cpp" }
local SKIP_DIRS = { "sub", "build", "tests", "test", "packages" }

local function Warn(msg)
  term.pushColor(term.warningColor)
  print("[warning] " .. msg)
  term.popColor()
end

local config = {}
if os.isfile(path.join(_SCRIPT_DIR, "workspace.lua")) then
  config = dofile(path.join(_SCRIPT_DIR, "workspace.lua")) or {}
else
  Warn("No workspace.lua, using defaults")
end

-- Packages --------------------------------------------------------------------

-- Only fetch for real generate actions, not clean/help
local FETCH = _ACTION ~= nil and _ACTION ~= "clean"

-- cmd /c strips the outer quotes off a command, so wrap it in one more pair
local function Run(cmd)
  return os.execute('"' .. cmd .. '"')
end

local function Fetch(name, pkg)
  local override = os.getenv(name:upper() .. "_DIR")
  if override then
    return path.getabsolute(override)
  end

  local dir = path.join(_SCRIPT_DIR, "packages", pkg.dir or name)
  local done = path.join(dir, ".fetched")
  if os.isfile(done) or not FETCH then
    return dir
  end

  print("[build] Downloading " .. name .. "...")
  os.mkdir(dir)
  local file = path.join(dir, path.getname(pkg.url:gsub("[?#].*$", "")))
  -- Windows' curl uses the system certificate store; premake's own http often has no CA bundle
  local curl = path.join(os.getenv("SystemRoot") or "C:/Windows", "System32/curl.exe")
  if not os.isfile(curl) then
    curl = "curl"
  end
  if not Run('"' .. path.translate(curl) .. '" -fsSL -o "' .. path.translate(file) .. '" "' .. pkg.url .. '"') then
    error("Couldn't download " .. name .. ". Check your connection and try again.", 0)
  end

  local ext = pkg.type and ("." .. pkg.type) or path.getextension(file):lower()
  if ext == ".zip" or ext == ".nupkg" then
    zip.extract(file, dir)
  elseif ext == ".exe" then
    -- 7-Zip self extracting archive (e.g. the OpenCV Windows release)
    local ok = Run('"' .. path.translate(file) .. '" -o"' .. path.translate(dir) .. '" -y >nul')
    if not ok then
      error("Couldn't extract " .. name, 0)
    end
  else
    error("Don't know how to extract " .. file, 0)
  end
  os.remove(file)

  io.writefile(done, pkg.url)
  return dir
end

local PACKAGES = {}
for name, pkg in pairs(config.packages or {}) do
  if type(pkg) == "function" then
    PACKAGES[name] = pkg
  else
    local dir = Fetch(name, pkg)
    PACKAGES[name] = function() pkg.use(dir) end
  end
end

-- Find projects ---------------------------------------------------------------

local projects = {} -- in the order they were found
local byName = {}

local function IsSkipped(file, dir)
  local rel = path.getrelative(dir, file)
  for _, skip in ipairs(SKIP_DIRS) do
    if rel:find("^" .. skip .. "/") or rel:find("/" .. skip .. "/") then
      return true
    end
  end
  return false
end

-- Folders with headers in them. Public ones are inside an include/ folder;
-- if there isn't one, every header folder is public.
local function HeaderDirs(dir, files)
  local all, public, seen = { dir }, {}, {}
  for _, file in ipairs(files) do
    local ext = path.getextension(file)
    local folder = path.getdirectory(file)
    if (ext == ".h" or ext == ".hpp") and not seen[folder] then
      seen[folder] = true
      table.insert(all, folder)
      local rel = "/" .. path.getrelative(dir, folder) .. "/"
      if rel:find("/include/") then
        table.insert(public, folder)
      end
    end
  end
  if #public == 0 then
    public = all
  end
  return all, public
end

local function AddProject(dir, isSubmodule)
  local name = path.getname(dir)
  if byName[name] then
    return
  end

  local cfg = {}
  if os.isfile(dir .. "/project.lua") then
    cfg = dofile(path.join(_SCRIPT_DIR, dir, "project.lua")) or {}
  end

  local files = {}
  for _, ext in ipairs(SOURCES) do
    for _, file in ipairs(os.matchfiles(dir .. "/**." .. ext)) do
      if not IsSkipped(file, dir) then
        table.insert(files, file)
      end
    end
  end

  local p = {
    name = name,
    dir = dir,
    external = isSubmodule,
    files = files,
    kind = cfg.kind or "StaticLib",
    links = cfg.links or {},
    setup = cfg.setup,
  }
  p.headerDirs, p.publicDirs = HeaderDirs(dir, files)
  byName[name] = p
  table.insert(projects, p)
end

-- Our own projects: any folder with a project.lua
for _, dir in ipairs(os.matchdirs("*")) do
  if os.isfile(dir .. "/project.lua") then
    AddProject(dir, false)
  end
end

-- Submodules, then their submodules, and so on. Found first = used.
local queue = { "." }
while #queue > 0 do
  local parent = table.remove(queue, 1)
  for _, dir in ipairs(os.matchdirs(parent .. "/sub/*")) do
    local name = path.getname(dir)
    if byName[name] == nil then
      if #os.matchfiles(dir .. "/*") == 0 then
        Warn(name .. " is empty. Run: git submodule update --init --recursive")
      else
        -- A submodule with no project.lua links the repos in its own sub/
        local nested = {}
        for _, n in ipairs(os.matchdirs(dir .. "/sub/*")) do
          table.insert(nested, path.getname(n))
        end
        AddProject(dir, true)
        if not os.isfile(dir .. "/project.lua") then
          byName[name].links = nested
        end
        table.insert(queue, dir)
      end
    end
  end
end

if #projects == 0 then
  Warn("No projects found. Add a project.lua to each project folder.")
end

-- Workspace -------------------------------------------------------------------

local startProject = config.startproject
if startProject == nil then
  for _, p in ipairs(projects) do
    if p.kind == "ConsoleApp" or p.kind == "WindowedApp" then
      startProject = p.name
      break
    end
  end
end

workspace(config.name or path.getname(_SCRIPT_DIR))
  location "build"
  configurations { "Debug", "Release" }
  platforms { "x64" }
  architecture "x86_64"
  characterset(config.characterset or "Unicode")
  if startProject then
    startproject(startProject)
  end

  targetdir "build/bin/%{cfg.buildcfg}/%{prj.name}"
  objdir "build/obj/%{cfg.buildcfg}/%{prj.name}"

  -- Warnings from submodule/package headers are theirs to fix, not ours
  externalwarnings "Off"

  filter "configurations:Debug"
    defines { "_DEBUG" }
    symbols "On"
    runtime "Debug"

  filter "configurations:Release"
    defines { "NDEBUG" }
    optimize "Speed"
    runtime "Release"

  filter {}

  if config.setup then
    config.setup()
  end

-- Define projects -------------------------------------------------------------

-- Everything a project links, including what its links link
local function AllLinks(p)
  local out, seen = {}, {}
  local function Visit(names)
    for _, name in ipairs(names) do
      if not seen[name] then
        seen[name] = true
        table.insert(out, name)
        if byName[name] then
          Visit(byName[name].links)
        end
      end
    end
  end
  Visit(p.links)
  return out
end

for _, p in ipairs(projects) do
  -- Solution folders match the disk, e.g. sub/ToolLib goes in a "sub" folder
  local folder = path.getdirectory(p.dir)
  group(folder == "." and "" or folder)

  project(p.name)
    kind(p.kind)
    language "C++"
    cppdialect(config.cppdialect or "C++20")
    files(p.files)
    includedirs(p.headerDirs)
    -- Solution Explorer shows the files as they are inside the project folder
    vpaths { ["*"] = p.dir .. "/**" }

    for _, name in ipairs(AllLinks(p)) do
      local dep = byName[name]
      if PACKAGES[name] then
        PACKAGES[name]()
      elseif dep == nil then
        Warn(p.name .. " links " .. name .. ", but there's no project or package called that")
      else
        if dep.external then
          externalincludedirs(dep.publicDirs)
        else
          includedirs(dep.publicDirs)
        end
        links { name }
      end
    end

    if p.setup then
      p.setup()
    end
end

group ""
