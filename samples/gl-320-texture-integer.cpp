#include "test.hpp"

namespace
{
	char const* VERT_SHADER_SOURCE("gl-320/texture-integer.vert");
	char const* FRAG_SHADER_SOURCE("gl-320/texture-integer.frag");
	char const* TEXTURE_DIFFUSE("kueken7_rgb8_unorm.dds");

	// With DDS textures, v texture coordinate are reversed, from top to bottom
	GLsizei const VertexCount(6);
	GLsizeiptr const VertexSize = VertexCount * sizeof(glf::vertex_v2fv2f);
	glf::vertex_v2fv2f const VertexData[VertexCount] =
	{
		glf::vertex_v2fv2f(glm::vec2(-1.0f,-1.0f), glm::vec2(0.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f,-1.0f), glm::vec2(1.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2(-1.0f, 1.0f), glm::vec2(0.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2(-1.0f,-1.0f), glm::vec2(0.0f, 1.0f))
	};

	GLuint BufferName(0);
	GLuint TextureName(0);
	GLuint VertexArrayName(0);
	GLuint ProgramName(0);
	GLint UniformMVP(0);
	GLint UniformDiffuse(0);
}//namespace

class sample : public framework
{
public:
	sample(int argc, char* argv[]) :
		framework(argc, argv, "gl-320-texture-integer", framework::CORE, 3, 2)
	{}

private:
	bool initProgram()
	{
		bool Validated = true;
	
		if(Validated)
		{
			compiler Compiler;
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE, "--version 150 --profile core");
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE, "--version 150 --profile core");
			Validated = Validated && Compiler.check();

			ProgramName = glCreateProgram();
			glAttachShader(ProgramName, VertShaderName);
			glAttachShader(ProgramName, FragShaderName);

			glBindAttribLocation(ProgramName, semantic::attr::POSITION, "Position");
			glBindAttribLocation(ProgramName, semantic::attr::TEXCOORD, "Texcoord");
			glBindFragDataLocation(ProgramName, semantic::frag::COLOR, "Color");
			glLinkProgram(ProgramName);
			Validated = Validated && Compiler.check_program(ProgramName);
		}

		if(Validated)
		{
			UniformMVP = glGetUniformLocation(ProgramName, "MVP");
			UniformDiffuse = glGetUniformLocation(ProgramName, "Diffuse");
		}

		return Validated && this->checkError("initProgram");
	}

	bool initBuffer()
	{
		glGenBuffers(1, &BufferName);

		glBindBuffer(GL_ARRAY_BUFFER, BufferName);
		glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		return this->checkError("initBuffer");;
	}

	bool initTexture()
	{
		gli::texture2d Texture(gli::load_dds((getDataDirectory() + TEXTURE_DIFFUSE).c_str()));

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glGenTextures(1, &TextureName);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureName);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(Texture.levels()) - 1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		for(std::size_t Level = 0; Level < Texture.levels(); ++Level)
		{
			glTexImage2D(GL_TEXTURE_2D, GLint(Level),
				GL_RGBA8UI,
				GLsizei(Texture[Level].extent().x), GLsizei(Texture[Level].extent().y),
				0,
				GL_RGB_INTEGER, GL_UNSIGNED_BYTE,
				Texture[Level].data());
		}
		glGenerateMipmap(GL_TEXTURE_2D);
	
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

		return this->checkError("initTexture");
	}

	bool initVertexArray()
	{
		glGenVertexArrays(1, &VertexArrayName);
		glBindVertexArray(VertexArrayName);
			glBindBuffer(GL_ARRAY_BUFFER, BufferName);
			glVertexAttribPointer(semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), BUFFER_OFFSET(0));
			glVertexAttribPointer(semantic::attr::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), BUFFER_OFFSET(sizeof(glm::vec2)));
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glEnableVertexAttribArray(semantic::attr::POSITION);
			glEnableVertexAttribArray(semantic::attr::TEXCOORD);
		glBindVertexArray(0);

		return this->checkError("initVertexArray");
	}

	bool begin()
	{
		bool Validated = true;

		if(Validated)
			Validated = initTexture();
		if(Validated)
			Validated = initProgram();
		if(Validated)
			Validated = initBuffer();
		if(Validated)
			Validated = initVertexArray();

		return Validated && this->checkError("begin");
	}

	bool end()
	{
		glDeleteBuffers(1, &BufferName);
		glDeleteProgram(ProgramName);
		glDeleteTextures(1, &TextureName);
		glDeleteVertexArrays(1, &VertexArrayName);

		return this->checkError("end");
	}

	bool render()
	{
		glm::ivec2 WindowSize(this->getWindowSize());

		glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, 4.0f / 3.0f, 0.1f, 100.0f);
		glm::mat4 Model = glm::mat4(1.0f);
		glm::mat4 MVP = Projection * this->view() * Model;

		glViewport(0, 0, WindowSize.x, WindowSize.y);
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)[0]);

		glUseProgram(ProgramName);
		glUniform1i(UniformDiffuse, 0);
		glUniformMatrix4fv(UniformMVP, 1, GL_FALSE, &MVP[0][0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureName);
		glBindVertexArray(VertexArrayName);

		glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 1);

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error = 0;

	sample Sample(argc, argv);
	Error += Sample();

	return Error;
}

