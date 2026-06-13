#ifndef GLMEMORY_HPP
#define GLMEMORY_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <map>

class BufferObject {
public:
	GLuint val;		// value location
	GLenum type;	// type of buffer (GL_ARRAY_BUFFER | GL_ELEMENT_ARRAY, etc)

	BufferObject(){ }
	BufferObject(GLenum type) : 
		type(type){ 
		generate();
	}

	// generate object
	void generate() {
		glGenBuffers(1, &val);
	}

	// bind object
	void bind() {
		glBindBuffer(type, val);
	}

	// set data (glBufferData)
	template <typename T>
	void setData(GLuint noElements, T* data, GLenum usage) {
		glBufferData(type, noElements * sizeof(T), data, usage);
	}

	template <typename T>
	void updateData(GLintptr offset, GLuint noElements, T* data) {
		glBufferSubData(type, offset, noElements * sizeof(T), data);
	}

	// set attribute pointers
	template <typename T>
	void setAttPointer(GLuint idx, GLint size, GLenum type, GLuint stride, GLuint offset, GLuint divisor = 0) {
		glVertexAttribPointer(idx, size, type, GL_FALSE, stride * sizeof(T), (void*)(offset * sizeof(T)));
		glEnableVertexAttribArray(idx);

		if (divisor > 0) {
			glVertexAttribDivisor(idx, divisor);
		}
	}

	// clear buffer objects (bind 0)
	void clear() {
		glBindBuffer(type, 0);
	}

	// clear
	void cleanup() {
		glDeleteBuffers(1, &val);
	}
};

class ArrayObject {

public:
	GLuint val;			// value / location

	std::map<const char*, BufferObject> buffers;

	// get buffer (override [])
	BufferObject& operator[](const char *key) {
		return buffers[key];
	}

	// generate object
	void generate() {
		glGenVertexArrays(1, &val);
	}

	// bind
	void bind() {
		glBindVertexArray(val);
	}

	// draw
	void draw(GLenum mode, GLuint count, GLenum type, GLint indices, GLuint instanceCount = 1) {
		glDrawElementsInstanced(mode, count, type, (void*)indices, instanceCount);
	}

	void draw(GLenum mode, GLsizei count) {
		glDrawArrays(mode, 0, count);
	}

	void cleanup() {
		glDeleteVertexArrays(1, &val);
		for (auto& pair : buffers) {
			pair.second.cleanup();
		}
	}

	// clear array object (bind 0)
	static void clear() {
		glBindVertexArray(0);
	}
};

#endif