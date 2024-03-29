#if !defined(_BTK_VULKAN_HPP_)
#define _BTK_VULKAN_HPP_

#include "../defs.hpp"
#include "../render.hpp"

#if __has_include(<vulkan/vulkan.h>)
#define BTK_HAS_VULKAN 1
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

            Context create_context() override;
            void    destroy_context(Context) override;

            void swap_buffer() override;
        private:
            SDL_Window *win;
            VkSurfaceKHR surface;
            VkPhysicalDevice gpu;
            VkDevice   dev;
            VkInstance instance;
            VkQueue    queue;
    };
}

#else
#define BTK_HAS_VULKAN 0
#endif

#endif // _BTK_VULKAN_HPP_
