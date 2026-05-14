#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>

Texture::Texture() { }

Texture::Texture(std::string dir, std::string path, bool gammaCorrection) 
				: dir(dir), path(path), gammaCorrection(gammaCorrection){
	generate();
}

void Texture::generate() {
	glGenTextures(1, &id);
}

void Texture::load(bool flip){
	stbi_set_flip_vertically_on_load(flip);

	int width, height, nChannels;
	unsigned char* data = stbi_load((dir + "/" + path).c_str(), &width, &height, &nChannels, 0);

	GLenum internalFormat;
	GLenum colorMode = GL_RGB;
	switch (nChannels) {
	case 1: 
		internalFormat = colorMode = GL_RED;
		break;
	case 3:
		internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
		colorMode = GL_RGB;
		break;
	case 4:
		internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_SRGB;
		colorMode = GL_RGBA;
		break;
	}

	if (data) {
		glBindTexture(GL_TEXTURE_2D, id);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, colorMode, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		if (colorMode == GL_RGBA) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
		else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	else {
		std::cout << "IMAGE NOT LOADED AT: " << path << std::endl;
	}

	stbi_image_free(data);
}

void Texture::bind() {
	glBindTexture(GL_TEXTURE_2D, id);
}