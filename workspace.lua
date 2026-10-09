-- workspace.lua (Geometrise)
-- Repo specific settings for premake5.lua. See the top of premake5.lua for every option.

local OPENCV_VER = "4.12.0"

-- The OpenCV release unpacks to opencv/build; OPENCV_DIR can point at either level
local function OpenCVRoot(dir)
  for _, root in ipairs({ dir .. "/opencv/build", dir .. "/build", dir }) do
    if os.isdir(root .. "/include/opencv2") then
      return root
    end
  end
  error("No OpenCV in " .. dir .. ". Delete packages/opencv and re-run createproject.bat, or set OPENCV_DIR.", 0)
end

return {
  name = "Geometrise",
  startproject = "ImageGeometrise",

  packages = {
    opencv = {
      url = "https://github.com/opencv/opencv/releases/download/" .. OPENCV_VER .. "/opencv-" .. OPENCV_VER .. "-windows.exe",
      dir = "opencv-" .. OPENCV_VER,
      -- opencv_world is the whole library in one lib/dll. The DLL is copied next to the exe after a build.
      use = function(dir)
        local root = OpenCVRoot(dir)
        local libdir = path.getdirectory(os.matchfiles(root .. "/x64/*/lib/opencv_world*.lib")[1] or (root .. "/x64/vc16/lib/x"))
        local bindir = path.join(path.getdirectory(libdir), "bin")
        local world = "opencv_world" .. OPENCV_VER:gsub("%.", "")

        externalincludedirs { root .. "/include" }
        libdirs { libdir }
        filter "configurations:Debug"
          links { world .. "d" }
          postbuildcommands { '{COPYFILE} "' .. bindir .. "/" .. world .. 'd.dll" "%{cfg.targetdir}"' }
        filter "configurations:Release"
          links { world }
          postbuildcommands { '{COPYFILE} "' .. bindir .. "/" .. world .. '.dll" "%{cfg.targetdir}"' }
        filter {}
      end,
    },

    -- Comes with the Windows SDK, d3dcompiler is linked by imgui_impl_dx11 itself
    directx11 = function()
      links { "d3d11", "dxgi" }
    end,
  },
}
