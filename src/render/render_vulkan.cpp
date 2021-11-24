#include "../build.hpp"

#include <Btk/gl/vulkan.hpp>
#include <SDL2/SDL_vulkan.h>

//TODO Vulkan Backend
#define  NANOVG_VULKAN_IMPLEMENTATION
#include "../libs/nanovg_vk.h"

namespace Btk{
    VulkanDevice::~VulkanDevice(){

    }
    void VulkanDevice::swap_buffer(){
        
    }
}