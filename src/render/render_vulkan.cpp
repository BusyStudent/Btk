#include "../build.hpp"

#include <Btk/detail/scope.hpp>
#include <Btk/detail/core.hpp>
#include <Btk/gl/vulkan.hpp>
#include <SDL2/SDL_vulkan.h>

//TODO Vulkan Backend
#define  NANOVG_VULKAN_IMPLEMENTATION
#include "../libs/nanovg_vk.h"

Btk_CallOnLoad{
    Btk::RegisterDevice([](SDL_Window *win) -> Btk::RendererDevice *{
        if((SDL_GetWindowFlags(win) & SDL_WINDOW_VULKAN) == SDL_WINDOW_VULKAN){
            return new Btk::VulkanDevice(win);
        }
        return nullptr;
    });
};

namespace Btk{
    VulkanDevice::VulkanDevice(SDL_Window *win){
        unsigned int ext_count = 0;
        SDL_Vulkan_GetInstanceExtensions(win,&ext_count,nullptr);
        std::vector<const char *> ext_names(ext_count);
        SDL_Vulkan_GetInstanceExtensions(win,&ext_count,ext_names.data());

        VkInstanceCreateInfo instanceCreateInfo = {};
        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pApplicationInfo = nullptr;
        instanceCreateInfo.enabledLayerCount = 0;
        instanceCreateInfo.ppEnabledLayerNames = nullptr;
        instanceCreateInfo.enabledExtensionCount = ext_names.size();
        instanceCreateInfo.ppEnabledExtensionNames = ext_names.data();
        
        vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
        //Get gpu
        Uint32 devs_count;
        std::vector<VkPhysicalDevice> phydevs(devs_count); 
        vkEnumeratePhysicalDevices(instance,&devs_count,nullptr);
        vkEnumeratePhysicalDevices(instance,&devs_count,phydevs.data());

        gpu = phydevs.at(0);

        //Create Device
        VkDeviceCreateInfo dev_crt_info = {};
        dev_crt_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        dev_crt_info.enabledLayerCount = 0;
        dev_crt_info.ppEnabledLayerNames = nullptr;
        dev_crt_info.enabledExtensionCount = ext_names.size();
        dev_crt_info.ppEnabledExtensionNames = ext_names.data();
        vkCreateDevice(gpu,&dev_crt_info,nullptr,&dev);
        //Get queue
        vkGetDeviceQueue(dev,0,0,&queue);
        //Create done
        SDL_Vulkan_CreateSurface(win,instance,&surface);
    }
    VulkanDevice::~VulkanDevice(){
        vkDestroyDevice(dev,nullptr);
        vkDestroyInstance(instance,nullptr);
    }
    auto VulkanDevice::create_context() -> Context{
        VKNVGCreateInfo info;
        info.allocator = nullptr;
        info.device = dev;
        info.gpu = gpu;
        return nvgCreateVk(
            info,
            NVG_ANTIALIAS,
            queue
        );
    }
    void VulkanDevice::destroy_context(Context ctxt){
        nvgDeleteVk(ctxt);
    }
    void VulkanDevice::swap_buffer(){
        
    }
}