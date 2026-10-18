-- ImageGeometrise: the ImGui app that approximates images with evolved shapes.
return {
  kind  = "WindowedApp",
  links = { "opencv", "directx11", "cuda" },
  setup = function()
    openmp "On"
    -- shapes/ and targets/ go next to the exe; main() switches to the exe's folder on startup
    postbuildcommands {
      '{COPYDIR} "%{wks.location}/../shapes" "%{cfg.targetdir}/shapes"',
      '{COPYDIR} "%{wks.location}/../targets" "%{cfg.targetdir}/targets"',
    }
  end,
}
