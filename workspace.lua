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

    -- Installed CUDA toolkit (CUDA_PATH is set by the installer).
    -- VS doesn't compile .cu files from premake on its own, so each one gets an nvcc build step.
    -- Without the toolkit it still builds, CPU only: the .cu files are left out and
    -- igGenerationBatch_nocuda.cpp stands in (the GPU checkbox shows as unavailable).
    cuda = function()
      local root = os.getenv("CUDA_PATH")
      if root == nil then
        print("[warning] CUDA_PATH isn't set, building without CUDA (CPU grading only)")
        defines { "IG_NO_CUDA" }
        removefiles { "**.cu" }
        return
      end
      root = path.normalize(root)

      externalincludedirs { root .. "/include" }
      libdirs { root .. "/lib/x64" }
      links { "cudart_static" }

      -- Every architecture CUDA 13 supports (Turing to Blackwell), plus PTX so newer GPUs can
      -- JIT it. -arch=native would only run on the GPU of the machine that built it.
      local archs = "-gencode arch=compute_75,code=sm_75 -gencode arch=compute_80,code=sm_80"
        .. " -gencode arch=compute_86,code=sm_86 -gencode arch=compute_89,code=sm_89"
        .. " -gencode arch=compute_90,code=sm_90 -gencode arch=compute_120,code=[sm_120,compute_120]"

      -- Same include paths as the C++ side
      local includes = '%{table.implode(table.join(cfg.includedirs, cfg.externalincludedirs), \'-I"\', \'"\', " ")}'
      local nvcc = '"' .. root .. '/bin/nvcc.exe" -c -std=c++20 ' .. archs .. ' ' .. includes
        .. ' -o "%{cfg.objdir}/%{file.basename}.cu.obj" "%{file.abspath}"'

      filter "files:**.cu"
        buildmessage "nvcc %{file.name}"
        buildoutputs { "%{cfg.objdir}/%{file.basename}.cu.obj" }
      filter { "files:**.cu", "configurations:Debug" }
        buildcommands { nvcc .. " -g -Xcompiler /MDd -D_DEBUG" }
      filter { "files:**.cu", "configurations:Release" }
        buildcommands { nvcc .. " -O3 -Xcompiler /MD -DNDEBUG" }
      filter {}
    end,
  },
}
