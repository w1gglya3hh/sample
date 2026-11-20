// gl1: generic application template

#include <exception>
#include <format>
#include <iostream>
#include <print>
#include <stdexcept>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>

#define LOG(...) std::println(std::cerr, __VA_ARGS__)

class Gl final
{
	SDL_Window* _pWindow{nullptr};
	SDL_GLContext _glContext{nullptr};

public:
	Gl()
	{
		if (!SDL_Init(SDL_INIT_VIDEO))
			throw std::runtime_error{std::format("SDL_Init - {}", SDL_GetError())};

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		_pWindow = SDL_CreateWindow("GL", 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
		if (!_pWindow)
		{
			SDL_Quit();
			throw std::runtime_error{std::format("SDL_CreateWindow - {}", SDL_GetError())};
		}

		_glContext = SDL_GL_CreateContext(_pWindow);
		if (!_glContext)
		{
			SDL_DestroyWindow(_pWindow);
			SDL_Quit();
			throw std::runtime_error{std::format("SDL_GL_CreateContext - {}", SDL_GetError())};
		}

		SDL_GL_MakeCurrent(_pWindow, _glContext);
		SDL_GL_SetSwapInterval(1);
	}

	~Gl()
	{
		SDL_GL_DestroyContext(_glContext);
		SDL_DestroyWindow(_pWindow);
		SDL_Quit();
	}


	void Run()
	{
		bool running{true};
		while (running)
		{
			SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				if (event.type == SDL_EVENT_QUIT)
					running = false;
			}

			glClearColor(0.5, 0.5, 0.5, 1.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			SDL_GL_SwapWindow(_pWindow);
		}
	}
};

int main()
{
	try
	{
		Gl gl;
		gl.Run();
	}
	catch (const std::exception& exception)
	{
		LOG("[F] exception caught - {}", exception.what());
		return 1;
	}
	catch (...)
	{
		LOG("[F] exception caught");
		return 1;
	}
}
