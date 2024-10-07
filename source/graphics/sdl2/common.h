#pragma once
// TODO:CODE:HIGH rename this to not be confusing with the other file called common

#include <GLES2/gl2.h>

#include <resources.h>

enum VERTEX_FORMAT {
	VERTEX_X,
	VERTEX_Y,
	VERTEX_PALETTE,
	VERTEX_BANK,
	VERTEX_U,
	VERTEX_V,
	VERTEX_UNIT_COUNT
};

/// The type used for vertices.
#define VERTEX_UNIT_TYPE GLfloat

typedef VERTEX_UNIT_TYPE vertex[VERTEX_UNIT_COUNT];
typedef vertex* vertex_array;


/// Number of vertices needed to create a single tile
#define VERTICES_PER_TILE 4
/// Number of vertex indices needed for a single tile
/// Because GLES2 doesn't support GL_QUADS, we use two triangles instead.
#define INDICES_PER_TILE 6

typedef GLuint quad[INDICES_PER_TILE];
typedef quad* quad_array;
// TODO:PERF:NONE This could probably be optimized using GL_TRIANGLE_STRIPs,
// but I'd be surprised if the necessary 2*[33*24]=1650 polygons per tilemap
// caused an issue for a modern PC.
// Even the DS itself supported 2K polygons with its 3D hardware.

/// Scale factor for normalizing coordinates from [0, SCREEN_WIDTH] to [-1,1]
#define FLOAT_PIXEL_WIDTH (2.0 / SCREEN_WIDTH)
/// Scale factor for normalizing coordinates from [0, SCREEN_HEIGHT] to [-1,1]
///
/// The negation handles flipping from GL coordinates system to screen coordinatess
#define FLOAT_PIXEL_HEIGHT (2.0 / SCREEN_HEIGHT)

static inline float norm_x(int x){
	return x * FLOAT_PIXEL_WIDTH - 1.0;
}

static inline float norm_y(int y){
	return -(y * FLOAT_PIXEL_HEIGHT - 1.0);
}
