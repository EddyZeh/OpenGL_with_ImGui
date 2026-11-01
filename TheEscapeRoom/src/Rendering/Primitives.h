#ifndef ESCAPE_PRIMITIVES_H
#define ESCAPE_PRIMITIVES_H

#include "graphics/mesh.h"		// engine Mesh / Vertex / VAO wrappers

#include "Prop.h"

/*
	The three placeholder shapes, built once as engine Meshes and shared by every Prop.
	All faces are wound counter-clockwise when seen from outside.
*/
class Primitives {
public:
	void init();
	void draw(Shape shape);
	void cleanup();

private:
	Mesh cube;
	Mesh sphere;
	Mesh cylinder;

	static Mesh buildCube();
	static Mesh buildSphere(unsigned int segments);
	static Mesh buildCylinder(unsigned int segments);
};

#endif // !ESCAPE_PRIMITIVES_H
