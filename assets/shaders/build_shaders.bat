C:/VulkanSDK/1.3.268.0/Bin/glslc.exe basic.vert -o compiled/vert.spv
C:/VulkanSDK/1.3.268.0/Bin/glslc.exe basic.frag -o compiled/frag.spv

C:/VulkanSDK/1.3.268.0/Bin/spirv-cross --version 310 --es compiled/vert.spv
C:/VulkanSDK/1.3.268.0/Bin/spirv-cross --version 310 --es compiled/frag.spv
