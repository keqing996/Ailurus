#include <SDL3/SDL_main.h>
#include "VulkanContext/VulkanFunctionLoader.h"

extern int Main(int argc, char* argv[]);

int main(int argc, char* argv[])
{
    bool successLoadVulkan = Ailurus::VulkanFunctionLoader::LoadVulkanLibrary();
    if (!successLoadVulkan)
    {
        return -1;
    }

    return Main(argc, argv);
}