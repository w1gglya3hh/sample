// gl11: refactored gl10

#include <cmath>
#include <cstddef>

#include <algorithm>
#include <array>
#include <bit>
#include <exception>
#include <format>
#include <iostream>
#include <memory>
#include <print>
#include <random>
#include <stdexcept>
#include <string_view>

#define GLM_FORCE_INLINE
#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>

#define LOG(...) std::println(std::cerr, __VA_ARGS__)

class Gl final
{
	struct SdlDelete final
	{
		void operator()(SDL_Surface* pSurface) const noexcept
		{
			SDL_DestroySurface(pSurface);
		}
	};

	static constexpr auto VertexData{std::to_array<GLfloat>({
		// front (+Z)
		+0.5f, +0.5f, +0.5f, 1.0f, 1.0f, // top-right
		+0.5f, -0.5f, +0.5f, 1.0f, 0.0f, // bottom-right
		-0.5f, -0.5f, +0.5f, 0.0f, 0.0f, // bottom-left
		-0.5f, +0.5f, +0.5f, 0.0f, 1.0f, // top-left

		// back (-Z)
		-0.5f, +0.5f, -0.5f, 1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
		+0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
		+0.5f, +0.5f, -0.5f, 0.0f, 1.0f,

		// right
		+0.5f, +0.5f, -0.5f, 1.0f, 1.0f,
		+0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
		+0.5f, -0.5f, +0.5f, 0.0f, 0.0f,
		+0.5f, +0.5f, +0.5f, 0.0f, 1.0f,

		// left
		-0.5f, +0.5f, +0.5f, 1.0f, 1.0f,
		-0.5f, -0.5f, +0.5f, 1.0f, 0.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
		-0.5f, +0.5f, -0.5f, 0.0f, 1.0f,

		// top
		-0.5f, +0.5f, -0.5f, 1.0f, 1.0f,
		-0.5f, +0.5f, +0.5f, 1.0f, 0.0f,
		+0.5f, +0.5f, +0.5f, 0.0f, 0.0f,
		+0.5f, +0.5f, -0.5f, 0.0f, 1.0f,

		// bottom
		-0.5f, -0.5f, +0.5f, 1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
		+0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
		+0.5f, -0.5f, +0.5f, 0.0f, 1.0f,
	})};

	static constexpr auto ElementData{std::to_array<GLuint>({
		// front (+Z)
		 0,  3,  1,
		 1,  3,  2,

		// back (-Z)
		 4,  7,  5,
		 5,  7,  6,

		// right (+X)
		 8, 11,  9,
		 9, 11, 10,

		// left (-X)
		12, 15, 13,
		13, 15, 14,

		// top (+Y)
		16, 17, 19,
		17, 18, 19,

		// bottom (-Y)
		20, 21, 23,
		21, 22, 23,
	})};

	static constexpr std::string_view VertexShaderSource{
		R"(
		#version 410 core
		layout (location = 0) in vec3 InPosition;
		layout (location = 1) in vec2 InUv;
		out vec2 Uv;
		uniform mat4 Model;
		uniform mat4 View;
		uniform mat4 Projection;
		void main()
		{
			gl_Position = Projection * View * Model * vec4(InPosition, 1.0);
			Uv = InUv;
		}
		)"
	};

	static constexpr std::string_view FragmentShaderSource{
		R"(
		#version 410 core
		out vec4 OutColor;
		in vec2 Uv;
		uniform sampler2D TextureBrick;
		uniform sampler2D TextureGraffiti;
		void main()
		{
			OutColor = mix(texture(TextureBrick, Uv), texture(TextureGraffiti, Uv), 0.5);
		}
		)"
	};

	SDL_Window* _pWindow{nullptr};
	SDL_GLContext _context{nullptr};
	GLuint _program{};
	GLuint _textureBrick{};
	GLuint _textureGraffiti{};
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

	[[nodiscard]] static std::unique_ptr<SDL_Surface, SdlDelete> LoadBmpImage(const char* pPath)
	{
		SDL_Surface* pImage{SDL_LoadBMP(pPath)};
		if (pImage)
		{
			SDL_Surface* pConverted{SDL_ConvertSurface(pImage, SDL_PIXELFORMAT_RGBA32)};
			SDL_DestroySurface(pImage);
			if (pConverted && SDL_FlipSurface(pConverted, SDL_FLIP_VERTICAL))
				return std::unique_ptr<SDL_Surface, SdlDelete>{pConverted};
		}
		return nullptr;
	}

	[[nodiscard]] static glm::mat4 GetPerspectiveProjection(int width, int height)
	{
		return glm::perspective(glm::radians(45.0f), static_cast<float>(width) / height, 0.1f, 100.0f);
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

		// initialize mostly OpenGL

		glEnable(GL_DEPTH_TEST);

		const auto pBrick{LoadBmpImage("data/brick.bmp")};
		const auto pGraffiti{LoadBmpImage("data/graffiti.bmp")};
		if (!pBrick || !pGraffiti)
		{
			SDL_GL_DestroyContext(_context);
			SDL_DestroyWindow(_pWindow);
			SDL_Quit();
			throw std::runtime_error{std::format("LoadBmpImage - {}", SDL_GetError())};
		}

		glActiveTexture(GL_TEXTURE0);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glGenTextures(1, &_textureBrick);
		glBindTexture(GL_TEXTURE_2D, _textureBrick);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pBrick->w, pBrick->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pBrick->pixels);
		glGenerateMipmap(GL_TEXTURE_2D);

		glActiveTexture(GL_TEXTURE1);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glGenTextures(1, &_textureGraffiti);
		glBindTexture(GL_TEXTURE_2D, _textureGraffiti);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pGraffiti->w, pGraffiti->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pGraffiti->pixels);
		glGenerateMipmap(GL_TEXTURE_2D);

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

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), static_cast<void*>(0));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), std::bit_cast<void*>(3 * sizeof(GLfloat)));
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
		// generate cube positions

		std::array<glm::vec3, 10> cubePositions{};

		std::ranges::generate(cubePositions, []() {
			static std::random_device randomDevice{};
			static std::mt19937 randomGenerator{randomDevice()};
			static std::uniform_real_distribution<float> xDistribution{-5.0f, 5.0f};
			static std::uniform_real_distribution<float> yDistribution{-3.0f, 3.0f};
			static std::uniform_real_distribution<float> zDistribution{-16.0f, 0.0f};

			const float x{xDistribution(randomGenerator)};
			const float y{yDistribution(randomGenerator)};
			const float z{zDistribution(randomGenerator)};

			return glm::vec3{x, y, z};
		});

		// rendering

		glUseProgram(_program);
		glUniform1i(glGetUniformLocation(_program, "TextureBrick"), 0);
		glUniform1i(glGetUniformLocation(_program, "TextureGraffiti"), 1);

		glm::mat4 model{glm::rotate(glm::mat4{1.0f}, glm::radians(-60.0f), glm::vec3{1.0f, 0.0f, 0.0f})};
		glm::mat4 view{glm::translate(glm::mat4{1.0f}, glm::vec3{0.0f, 0.0f, -3.0f})};
		glm::mat4 projection{GetPerspectiveProjection(800, 600)};

		const GLint modelLocation{glGetUniformLocation(_program, "Model")};
		const GLint viewLocation{glGetUniformLocation(_program, "View")};
		const GLint projectionLocation{glGetUniformLocation(_program, "Projection")};

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
					projection = GetPerspectiveProjection(w, h);
					break;
				}
			}

			// some calls only need to be done once; but it is often good practice to call them every frame

			glClearColor(0.5, 0.5, 0.5, 1.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, _textureBrick);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, _textureGraffiti);
			glBindVertexArray(_vertexArray);

			const float angle{std::fmodf(static_cast<float>(SDL_GetTicks()) / 1000.0f * 60.0f, 360.0f)};

			for (const auto& position : cubePositions)
			{
				glm::mat4 newModel{1.0f};
				newModel = glm::translate(newModel, position);
				newModel = glm::rotate(newModel, glm::radians(angle), glm::normalize(glm::vec3{0.5f, 1.0f, 1.0f}));
				newModel *= model;

				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(newModel));
				glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
			}

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
