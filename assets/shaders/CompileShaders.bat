for %%f in (*.vert) do (
	%VULKAN_SDK%/bin/glslangValidator.exe -V %%f -o %%~nf.spv
)
for %%f in (*.frag) do (
	%VULKAN_SDK%/bin/glslangValidator.exe -V %%f -o %%~nf.spv
)

pause
