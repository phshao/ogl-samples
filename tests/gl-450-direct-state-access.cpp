///////////////////////////////////////////////////////////////////////////////////
/// OpenGL Samples Pack (ogl-samples.g-truc.net)
///
/// Copyright (c) 2004 - 2014 G-Truc Creation (www.g-truc.net)
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
/// THE SOFTWARE.
///////////////////////////////////////////////////////////////////////////////////

#include "test.hpp"
//#include "dsa.hpp"

namespace
{
	char const * VERT_SHADER_SOURCE("gl-450/direct-state-access.vert");
	char const * FRAG_SHADER_SOURCE("gl-450/direct-state-access.frag");
	char const * TEXTURE_DIFFUSE("kueken3-bgr8.dds");
	glm::ivec2 const FRAMEBUFFER_SIZE(80, 60);

	GLsizei const VertexCount(4);
	GLsizeiptr const VertexSize = VertexCount * sizeof(glf::vertex_v2fv2f);
	glf::vertex_v2fv2f const VertexData[VertexCount] =
	{
		glf::vertex_v2fv2f(glm::vec2(-2.0f,-1.5f), glm::vec2(0.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2( 2.0f,-1.5f), glm::vec2(1.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2( 2.0f, 1.5f), glm::vec2(1.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2(-2.0f, 1.5f), glm::vec2(0.0f, 1.0f))
	};

	GLsizei const ElementCount(6);
	GLsizeiptr const ElementSize = ElementCount * sizeof(GLushort);
	GLushort const ElementData[ElementCount] =
	{
		0, 1, 2,
		2, 3, 0
	};

	namespace program
	{
		enum type
		{
			VERTEX,
			FRAGMENT,
			MAX
		};
	}//namespace program

	namespace framebuffer
	{
		enum type
		{
			RENDER,
			RESOLVE,
			MAX
		};
	}//namespace framebuffer

	namespace buffer
	{
		enum type
		{
			VERTEX,
			ELEMENT,
			TRANSFORM,
			MAX
		};
	}//namespace buffer

	namespace texture
	{
		enum type
		{
			TEXTURE,
			MULTISAMPLE,
			COLORBUFFER,
			MAX
		};
	}//namespace texture
}//namespace

class gl_450_direct_state_access : public test
{
public:
	gl_450_direct_state_access(int argc, char* argv[]) :
		test(argc, argv, "gl-450-direct-state-access", test::CORE, 4, 5, glm::ivec2(640, 480), glm::vec2(glm::pi<float>() * 0.0f)),
		VertexArrayName(0),
		PipelineName(0),
		ProgramName(0),
		SamplerName(0),
		UniformBlockSize(0)
	{}

private:
	std::array<GLuint, buffer::MAX> BufferName;
	std::array<GLuint, texture::MAX> TextureName;
	std::array<GLuint, framebuffer::MAX> FramebufferName;
	GLuint VertexArrayName;
	GLuint PipelineName;
	GLuint ProgramName;
	GLuint SamplerName;
	GLint UniformBlockSize;

	bool initProgram()
	{
		bool Validated = true;

		if(Validated)
		{
			compiler Compiler;
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE, "--version 430 --profile core");
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE, "--version 430 --profile core");
			Validated = Validated && Compiler.check();

			ProgramName = glCreateProgram();
			glProgramParameteri(ProgramName, GL_PROGRAM_SEPARABLE, GL_TRUE);
			glAttachShader(ProgramName, VertShaderName);
			glAttachShader(ProgramName, FragShaderName);
			glLinkProgram(ProgramName);

			Validated = Validated && Compiler.checkProgram(ProgramName);
		}

		if(Validated)
		{
			glGenProgramPipelines(1, &PipelineName);
			glUseProgramStages(PipelineName, GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, ProgramName);
		}

		return Validated && this->checkError("initProgram");
	}

	bool initBuffer()
	{
		GLint UniformBufferOffset(0);
		glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &UniformBufferOffset);
		UniformBlockSize = glm::max(GLint(sizeof(glm::mat4)), UniformBufferOffset);

		glCreateBuffers(buffer::MAX, &BufferName[0]);
		glNamedBufferStorage(BufferName[buffer::ELEMENT], ElementSize, ElementData, 0);
		glNamedBufferStorage(BufferName[buffer::VERTEX], VertexSize, VertexData, 0);
		glNamedBufferStorage(BufferName[buffer::TRANSFORM], UniformBlockSize * 2, nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);

		return true;
	}

	bool initSampler()
	{
		glCreateSamplers(1, &SamplerName);
		glSamplerParameteri(SamplerName, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glSamplerParameteri(SamplerName, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glSamplerParameteri(SamplerName, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glSamplerParameteri(SamplerName, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glSamplerParameteri(SamplerName, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glSamplerParameterfv(SamplerName, GL_TEXTURE_BORDER_COLOR, &glm::vec4(0.0f)[0]);
		glSamplerParameterf(SamplerName, GL_TEXTURE_MIN_LOD, -1000.f);
		glSamplerParameterf(SamplerName, GL_TEXTURE_MAX_LOD, 1000.f);
		glSamplerParameterf(SamplerName, GL_TEXTURE_LOD_BIAS, 0.0f);
		glSamplerParameteri(SamplerName, GL_TEXTURE_COMPARE_MODE, GL_NONE);
		glSamplerParameteri(SamplerName, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

		return this->checkError("initSampler");
	}

	bool initTexture()
	{
		gli::texture2D Texture(gli::load_dds((getDataDirectory() + TEXTURE_DIFFUSE).c_str()));

		glCreateTextures(GL_TEXTURE_2D, 1, &TextureName[texture::TEXTURE]);
		glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &TextureName[texture::MULTISAMPLE]);
		glCreateTextures(GL_TEXTURE_2D, 1, &TextureName[texture::COLORBUFFER]);

		glTextureParameteri(TextureName[texture::TEXTURE], GL_TEXTURE_BASE_LEVEL, 0);
		glTextureParameteri(TextureName[texture::TEXTURE], GL_TEXTURE_MAX_LEVEL, 0);
		glTextureParameteri(TextureName[texture::TEXTURE], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(TextureName[texture::TEXTURE], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureStorage2D(TextureName[texture::TEXTURE], GLint(Texture.levels()), gli::internal_format(Texture.format()), GLsizei(Texture[0].dimensions().x), GLsizei(Texture[0].dimensions().y));
		for(std::size_t Level = 0; Level < Texture.levels(); ++Level)
		{
			glTextureSubImage2D(TextureName[texture::TEXTURE], GLint(Level),
				0, 0, 
				GLsizei(Texture[Level].dimensions().x), GLsizei(Texture[Level].dimensions().y), 
				gli::external_format(Texture.format()), gli::type_format(Texture.format()),
				Texture[Level].data());
		}

		glTextureParameteri(TextureName[texture::MULTISAMPLE], GL_TEXTURE_BASE_LEVEL, 0);
		glTextureParameteri(TextureName[texture::MULTISAMPLE], GL_TEXTURE_MAX_LEVEL, 0);
		glTextureStorage2DMultisample(TextureName[texture::MULTISAMPLE], 4, GL_RGBA8, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y, GL_FALSE);

		glTextureParameteri(TextureName[texture::COLORBUFFER], GL_TEXTURE_BASE_LEVEL, 0);
		glTextureParameteri(TextureName[texture::COLORBUFFER], GL_TEXTURE_MAX_LEVEL, 0);
		glTextureStorage2D(TextureName[texture::COLORBUFFER], 1, GL_RGBA8, GLsizei(FRAMEBUFFER_SIZE.x), GLsizei(FRAMEBUFFER_SIZE.y));

		return this->checkError("initTexture");
	}

	bool initFramebuffer()
	{
		glCreateFramebuffers(framebuffer::MAX, &FramebufferName[0]);
		glNamedFramebufferTexture(FramebufferName[framebuffer::RENDER], GL_COLOR_ATTACHMENT0, TextureName[texture::MULTISAMPLE], 0);
		glNamedFramebufferTexture(FramebufferName[framebuffer::RESOLVE], GL_COLOR_ATTACHMENT0, TextureName[texture::COLORBUFFER], 0);

		if(glCheckNamedFramebufferStatus(FramebufferName[framebuffer::RENDER], GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			return false;
		if(glCheckNamedFramebufferStatus(FramebufferName[framebuffer::RESOLVE], GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			return false;

		return this->checkError("initFramebuffer");
	}

	bool initVertexArray()
	{
		bool Validated(true);

		glCreateVertexArrays(1, &VertexArrayName);
		glBindVertexArray(VertexArrayName);
			glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
			glVertexAttribPointer(semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), BUFFER_OFFSET(0));
			glVertexAttribPointer(semantic::attr::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), BUFFER_OFFSET(sizeof(glm::vec2)));
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glEnableVertexAttribArray(semantic::attr::POSITION);
			glEnableVertexAttribArray(semantic::attr::TEXCOORD);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);
		glBindVertexArray(0);

		return Validated;
	}
/*
	bool initVertexArray()
	{
		glGenVertexArrays(1, &VertexArrayName);
		glVertexArrayElementBuffer(VertexArrayName, BufferName[buffer::ELEMENT]);
		glVertexArrayVertexAttribOffset(VertexArrayName, BufferName[buffer::VERTEX], semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), 0);
		glVertexArrayVertexAttribOffset(VertexArrayName, BufferName[buffer::VERTEX], semantic::attr::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), sizeof(glm::vec2));
		glEnableVertexArrayAttrib(VertexArrayName, semantic::attr::POSITION);
		glEnableVertexArrayAttrib(VertexArrayName, semantic::attr::TEXCOORD);

		return this->checkError("initVertexArray");
	}
*/

	bool begin()
	{
		bool Validated = true;

		if(Validated)
			Validated = initProgram();
		if(Validated)
			Validated = initSampler();
		if(Validated)
			Validated = initBuffer();
		if(Validated)
			Validated = initVertexArray();
		if(Validated)
			Validated = initTexture();
		if(Validated)
			Validated = initFramebuffer();

		//glEnable(GL_SAMPLE_MASK);
		//glSampleMaski(0, 0xFF);

		return Validated && this->checkError("begin");
	}

	bool end()
	{
		glDeleteBuffers(buffer::MAX, &BufferName[0]);
		glDeleteProgram(ProgramName);
		glDeleteTextures(texture::MAX, &TextureName[0]);
		glDeleteFramebuffers(framebuffer::MAX, &FramebufferName[0]);
		glDeleteVertexArrays(1, &VertexArrayName);
		glDeleteSamplers(1, &SamplerName);

		return true;
	}

	void renderFBO()
	{
		//glEnable(GL_SAMPLE_MASK);
		//glSampleMaski(0, 0xFF);

		glEnable(GL_MULTISAMPLE);
		glEnable(GL_SAMPLE_SHADING);
		glMinSampleShading(4.0f);

		glViewportIndexedf(0, 0, 0, static_cast<float>(FRAMEBUFFER_SIZE.x), static_cast<float>(FRAMEBUFFER_SIZE.y));
		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName[framebuffer::RENDER]);
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.0f, 0.5f, 1.0f, 1.0f)[0]);

		GLintptr Offset(0);
		//glBindBuffersRange(GL_UNIFORM_BUFFER, semantic::uniform::TRANSFORM0, 1, &BufferName[buffer::TRANSFORM], &Offset, reinterpret_cast<GLsizeiptr*>(&this->UniformBlockSize));
		glBindBufferRange(GL_UNIFORM_BUFFER, semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM], this->UniformBlockSize, this->UniformBlockSize);
		glBindSamplers(0, 1, &SamplerName);
		glBindTextures(0, 1, &TextureName[texture::TEXTURE]);
		glBindVertexArray(VertexArrayName);

		glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, nullptr, 1, 0, 0);

		glDisable(GL_MULTISAMPLE);

		this->checkError("renderFBO");
	}

	void renderFB()
	{
		glm::vec2 WindowSize(this->getWindowSize());

		glViewportIndexedf(0, 0, 0, WindowSize.x, WindowSize.y);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)[0]);

		glBindBufferRange(GL_UNIFORM_BUFFER, semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM], this->UniformBlockSize, this->UniformBlockSize);
		glBindTextures(0, 1, &TextureName[texture::COLORBUFFER]);
		glBindVertexArray(VertexArrayName);

		glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, nullptr, 1, 0, 0);

		this->checkError("renderFB");
	}

	bool render()
	{
		glm::vec2 WindowSize(this->getWindowSize());

		{
			glm::uint8* Pointer = reinterpret_cast<glm::uint8*>(glMapNamedBufferRange(
				BufferName[buffer::TRANSFORM], 0, this->UniformBlockSize * 2, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));

			glm::mat4 ProjectionA = glm::scale(glm::perspective(glm::pi<float>() * 0.25f, float(FRAMEBUFFER_SIZE.x) / FRAMEBUFFER_SIZE.y, 0.1f, 100.0f), glm::vec3(1, -1, 1));
			*reinterpret_cast<glm::mat4*>(Pointer + 0) = ProjectionA * this->view() * glm::mat4(1);

			glm::mat4 ProjectionB = glm::perspective(glm::pi<float>() * 0.25f, WindowSize.x / WindowSize.y, 0.1f, 100.0f);
			*reinterpret_cast<glm::mat4*>(Pointer + this->UniformBlockSize) = ProjectionB * this->view() * glm::mat4(1);

			// Make sure the uniform buffer is uploaded
			glUnmapNamedBuffer(BufferName[buffer::TRANSFORM]);
		}

		glBindProgramPipeline(PipelineName);

		// Pass 1, render the scene in a multisampled framebuffer
		renderFBO();

		glBlitNamedFramebuffer(
			FramebufferName[framebuffer::RENDER],
			FramebufferName[framebuffer::RESOLVE],
			0, 0, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y,
			0, 0, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y,
			GL_COLOR_BUFFER_BIT, GL_NEAREST);

		GLenum MaxColorAttachment = GL_COLOR_ATTACHMENT0;
		glInvalidateNamedFramebufferData(FramebufferName[framebuffer::RENDER], 1, &MaxColorAttachment);

		// Pass 2, render the colorbuffer from the multisampled framebuffer
		renderFB();

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_450_direct_state_access Test(argc, argv);
	Error += Test();

	return Error;
}

