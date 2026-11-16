// gl2: drawing a triangle

#include <cstddef>

#include <array>
#include <exception>
#include <format>
#include <iostream>
#include <memory>
#include <print>
#include <stdexcept>
#include <string>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>

#define LOG(...) std::println(std::cerr, __VA_ARGS__)

class Gl final
{
	static constexpr auto VertexData{std::to_array<GLfloat>({
		-0.5, -0.5, 0.0, // bottom-left
		+0.5, -0.5, 0.0, // bottom-right
		+0.0, +0.5, 0.0, // top-center
	})};

	static constexpr std::string_view VertexShaderSource{
		R"(
		#version 410 core
		layout (location = 0) in vec3 InPosition;
		void main()
		{
			gl_Position = vec4(InPosition, 1.0);
		}
		)"
	};

	static constexpr std::string_view FragmentShaderSource{
		R"(
		#version 410 core
		out vec4 OutColor;
		void main()
		{
			OutColor = vec4(0.0, 0.0, 1.0, 1.0);
		}
		)"
	};

	SDL_Window* _pWindow{nullptr};
	SDL_GLContext _context{nullptr};
	GLuint _program{};
	GLuint _vertexArray{};
	GLuint _vertexBuffer{};

	static void CompileShader(GLuint shader)
	{
		glCompileShader(shader);

		GLint result{};
		glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
		if (!result)
		{
			GLint logLength;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
			if (logLength > 1)
			{
				const auto pLog{std::make_unique_for_overwrite<char[]>(static_cast<std::size_t>(logLength))};
				glGetShaderInfoLog(shader, logLength, nullptr, pLog.get());
				throw std::runtime_error{std::format("glCompileShader - {}", pLog.get())};
			}
			throw std::runtime_error{"glCompileShader - unknown error"};
		}
	}

	static void LinkProgram(GLuint program)
	{
		glLinkProgram(program);

		GLint result{};
		glGetProgramiv(program, GL_LINK_STATUS, &result);
		if (!result)
		{
			GLint logLength;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
			if (logLength > 1)
			{
				const auto pLog{std::make_unique_for_overwrite<char[]>(static_cast<std::size_t>(logLength))};
				glGetProgramInfoLog(program, logLength, nullptr, pLog.get());
				throw std::runtime_error{std::format("glLinkProgram - {}", pLog.get())};
			}
			throw std::runtime_error{"glLinkProgram - unknown error"};
		}
	}

public:
	Gl()
	{
		// initialize SDL

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

		_context = SDL_GL_CreateContext(_pWindow);
		if (!_context)
		{
			SDL_DestroyWindow(_pWindow);
			SDL_Quit();
			throw std::runtime_error{std::format("SDL_GL_CreateContext - {}", SDL_GetError())};
		}

		SDL_GL_MakeCurrent(_pWindow, _context);
		SDL_GL_SetSwapInterval(1);

		// initialize OpenGL

		const GLuint vertexShader{glCreateShader(GL_VERTEX_SHADER)};
		const char* pVertexShaderSource{VertexShaderSource.data()};
		glShaderSource(vertexShader, 1, &pVertexShaderSource, nullptr);
		CompileShader(vertexShader);

		const GLuint fragmentShader{glCreateShader(GL_FRAGMENT_SHADER)};
		const char* pFragmentShaderSource{FragmentShaderSource.data()};
		glShaderSource(fragmentShader, 1, &pFragmentShaderSource, nullptr);
		CompileShader(fragmentShader);

		_program = glCreateProgram();
		glAttachShader(_program, vertexShader);
		glAttachShader(_program, fragmentShader);
		LinkProgram(_program);

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		glGenVertexArrays(1, &_vertexArray);
		glBindVertexArray(_vertexArray);

		glGenBuffers(1, &_vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, VertexData.size() * sizeof(GLfloat), VertexData.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, static_cast<void*>(0)); // tightly-packed
		glEnableVertexAttribArray(0);
	}

	~Gl()
	{
		SDL_GL_DestroyContext(_context);
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
				switch (event.type)
				{
				case SDL_EVENT_QUIT:
					running = false;
					break;
				case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
					const int w{event.window.data1};
					const int h{event.window.data2};
					glViewport(0, 0, w, h);
					break;
				}
			}

			// some calls only need to be done once; but it is often good practice to call them every frame

			glClearColor(0.5, 0.5, 0.5, 1.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUseProgram(_program);
			glBindVertexArray(_vertexArray);
			glDrawArrays(GL_TRIANGLES, 0, 3);

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
