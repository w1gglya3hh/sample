// gl5: color gradient using vertex buffer

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
		+0.5f, +0.5f, 0.0f, 1.0f, 0.0f, 0.0f, // top-right
		+0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, // bottom-right
		-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
		-0.5f, +0.5f, 0.0f, 0.0f, 0.0f, 0.0f, // top-left
	})};

	static constexpr auto ElementData{std::to_array<GLuint>({
		0, 1, 3, // 1st
		1, 2, 3, // 2nd
	})};

	static constexpr std::string_view VertexShaderSource{
		R"(
		#version 410 core
		layout (location = 0) in vec3 InPosition;
		layout (location = 1) in vec3 InColor;
		out vec3 Color;
		void main()
		{
			gl_Position = vec4(InPosition, 1.0);
			Color = InColor;
		}
		)"
	};

	static constexpr std::string_view FragmentShaderSource{
		R"(
		#version 410 core
		out vec4 OutColor;
		in vec3 Color;
		void main()
		{
			OutColor = vec4(Color, 1.0);
		}
		)"
	};

	SDL_Window* _pWindow{nullptr};
	SDL_GLContext _context{nullptr};
	GLuint _program{};
	GLuint _vertexArray{};
	GLuint _vertexBuffer{};
	GLuint _elementBuffer{};

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

		glGenBuffers(1, &_elementBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _elementBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, ElementData.size() * sizeof(GLuint), ElementData.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), static_cast<void*>(0));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);
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
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

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
