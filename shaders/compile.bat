@echo off
for %%i in (*.vert *.tesc *.tese *.geom *.frag *.comp) do F:/Vulkan/1.1.92.1/Bin/glslangValidator.exe -V %%i -o %%i.spv
pause