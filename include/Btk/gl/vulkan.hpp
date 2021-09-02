#if !defined(_BTK_VULKAN_HPP_)
#define _BTK_VULKAN_HPP_

#include "../defs.hpp"
#include "../render.hpp"

#include <vulkan/vulkan.h>

struct SDL_Window;
namespace Btk{
    /**
     * @brief Vulkan Renderer Device
     * 
     */
    class BTKAPI VulkanDevice:public RendererDevice{
        public:
            VulkanDevice(SDL_Window *win);
            ~VulkanDevice();
        private:
            VkInstance vk;
    };
}

#endif // _BTK_VULKAN_HPP_
