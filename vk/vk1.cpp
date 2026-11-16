#include <exception>
#include <format>
#include <iostream>
#include <print>
#include <stdexcept>

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

int main()
{
	try
	{
		VULKAN_HPP_DEFAULT_DISPATCHER.init();

		if (!SDL_Init(SDL_INIT_VIDEO))
			throw std::runtime_error{std::format("SDL_Init: {}", SDL_GetError())};
		//if (!SDL_Vulkan_LoadLibrary(nullptr))
		//	throw std::runtime_error{std::format("SDL_Vulkan_LoadLibrary: {}", SDL_GetError())};
		SDL_Window* pWindow{SDL_CreateWindow("vksdldl", 800, 600, SDL_WINDOW_VULKAN)};
		if (!pWindow)
			throw std::runtime_error{std::format("SDL_CreateWindow: {}", SDL_GetError())};

		SDL_DestroyWindow(pWindow);
		SDL_Quit();
	}
	catch (const std::exception& e)
	{
		std::println(std::cerr, "exception caught: {}", e.what());
	}
}
