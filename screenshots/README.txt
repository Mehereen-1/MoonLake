================================================================================
MoonLake Project Report - Screenshot Placement Guide
================================================================================

The report file `report_anti.tex` features intelligent auto-detecting
placeholder macros (\IfFileExists).

When you capture screenshots of your running MoonLake project, simply save them
into this folder (`screenshots/`) with the exact filenames listed below.
The next time you compile the LaTeX report, it will automatically embed your
actual screenshots in place of the placeholder boxes!

--------------------------------------------------------------------------------
PART A: Chapter 4 Side-by-Side Object Screenshots (Right of Each Paragraph)
--------------------------------------------------------------------------------
Save these screenshots into `screenshots/` to appear right beside their paragraphs:

1. obj_terrain.png
   - 4.2 Terrain and mountains: Custom terrain mesh, height elevation, distant ridges.
2. obj_lake.png
   - 4.3 Lake and shore: Subdivided water basin, gentle shore waves, deep lake water.
3. obj_sky.png
   - 4.4 Sky, sun, moon, and stars: Atmospheric sky dome, celestial bodies, night stars.
4. obj_clouds.png
   - 4.5 Clouds: Drifting rounded volumetric cloud spheres across the horizon.
5. obj_cabin.png
   - 4.6 Cabin: Timber walls, sloping prism roof, porch supports, door, and windows.
6. obj_campfire.png
   - 4.7 Campfire and firewood: Rock stone ring, crossed firewood logs, dynamic flame layers.
7. obj_cave.png
   - 4.8 Cave and treasure room: Stone tunnel, glowing torch, treasure chest, gold coins.
8. obj_trees.png
   - 4.9 Trees and vegetation: Pine trees, autumn deciduous trees, grass tufts, shoreline bushes.
9. obj_windmill.png
   - 4.10 Windmill: Tower body, conical roof, rotating aerodynamic 4-blade wheel.
10. obj_boat_balloon.png
    - 4.11 Boat and hot-air balloon: Lakeside wooden rowboat with oars and airborne hot-air balloon.
11. obj_train.png
    - 4.12 Tourist train and station: Vintage steam locomotive, passenger coach, rails, station.
12. obj_shoreline.png
    - 4.13 Autumn shoreline: Wooden promenade dock, bench, lanterns, pebbles, fallen logs.
13. obj_rain.png
    - 4.14 Rain: Dynamic semi-transparent falling rain streak particles.

--------------------------------------------------------------------------------
PART B: Full-Width Figures and Result Screenshots
--------------------------------------------------------------------------------
14. overview_environment.png
    - Figure 1.2: Complete panoramic view of the lake, campsite, mountains, and sky.
15. primitives_comparison.png
    - Figure 3.1: Visual gallery of the reusable unit geometric primitives.
16. constructed_objects_overview.png
    - Figure 4.2: Overview of major constructed objects assembled in the environment.
17. lighting_shadows_view.png
    - Figure 5.3: Scene view showing directional sunlight/moonlight, point lights, and cast shadows.
18. textured_vs_basecolor.png
    - Figure 5.4: Visual comparison between textured surfaces and flat base-color materials.
19. motion_comparison.png
    - Figure 6.2: Motion comparison showing train on railway, moving water, rain, and campfire.
20. ray_tracing_coins_comparison.png
    - Figure 7.2: Side-by-side comparison of treasure coins with ray tracing disabled vs enabled.
21. result_night_view.png
    - Figure 9.1: Night view showing stars, moonlight, lanterns, campfire, and reflective lake.
22. result_sunset_view.png
    - Figure 9.2: Sunset view showing warm sky colors, terrain, cabin, and lake.
23. result_cave_view.png
    - Figure 9.3: Cave view showing stone surfaces, torch light, treasure chest, and shiny coins.
24. result_train_view.png
    - Figure 9.4: Train view showing the curved railway track, station, and forward headlight.

================================================================================
How to Compile the Report:
--------------------------------------------------------------------------------
Option A: Overleaf (Recommended)
  1. Open https://www.overleaf.com and create a new project.
  2. Upload `report_anti.tex`.
  3. Create a folder named `screenshots/` and upload your screenshots there (optional).
  4. Click "Recompile" (using standard pdfLaTeX).

Option B: Local LaTeX (TeX Live / MiKTeX)
  Run in terminal:
    pdflatex report_anti.tex
    pdflatex report_anti.tex  (run twice to build table of contents and figure list)
================================================================================
