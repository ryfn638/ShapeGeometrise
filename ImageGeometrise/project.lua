-- ImageGeometrise: the ImGui app that approximates images with evolved shapes.
return {
  kind  = "WindowedApp",
  links = { "opencv", "directx11", "cuda" },
  setup = function()
    openmp "On"
    -- Run from the repo root so targets/ and shapes/ resolve
    debugdir "%{wks.location}/.."
  end,
}
