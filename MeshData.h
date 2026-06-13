#ifndef MESHDATA_HPP
#define MESHDATA_HPP

#include <vector>

struct Vert {
	std::vector<float> vertices;
	std::vector<unsigned int> indices;
};


class MeshData {
public:
	static Vert Cube;
	static Vert Plane;
	static Vert Quad;
	static Vert Skybox;
};

#endif // !MESHDATA_HPP
