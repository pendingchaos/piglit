/*
 * Copyright (c) 2018 Rhys Perry
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL AUTHORS AND/OR THEIR SUPPLIERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "piglit-util-gl.h"

/**
 * @file draw.c
 *
 * This test verifies that primitive dialation is implemented correctly
 * for GL_NV_conservative_raster_dilate.
 */

static const struct piglit_subtest subtests[];

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 21;
	config.subtests = subtests;
	config.window_width = 250;
	config.window_height = 250;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

GLuint program, buf_triangle;

static const float verts_triangle[6] = {
	-0.799998779, -0.799998779,
	 0.799998779,  0.799998779,
	 0.799998779, -0.799998779};

static const float white[4] = {1.0f, 1.0f, 1.0f, 1.0f};
static const float black[4] = {0.0f, 0.0f, 0.0f, 1.0f};

static void draw(GLenum prim)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(program);
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

static enum piglit_result test(const char* data)
{
	bool dilate = strcmp(data, "dilate") == 0;
	int d1 = dilate ? 1 : 0;
	glEnable(GL_CONSERVATIVE_RASTERIZATION_NV);
	glConservativeRasterParameterfNV(GL_CONSERVATIVE_RASTER_DILATE_NV, dilate?0.75:0.0);
	draw(GL_TRIANGLES);
	int c = 0;

	c += piglit_probe_pixel_rgba(90-d1, 92, black);
	c += piglit_probe_pixel_rgba(90-d1, 91, white);

	c += piglit_probe_pixel_rgba(179, 181+d1, black);
	c += piglit_probe_pixel_rgba(180, 181+d1, white);

	c += piglit_probe_pixel_rgba(226, 119, black);
	c += piglit_probe_pixel_rgba(225, 119, white);

	c += piglit_probe_pixel_rgba(130, 23, black);
	c += piglit_probe_pixel_rgba(130, 24, white);

	return c==8 ? PIGLIT_PASS : PIGLIT_FAIL;
}

static const struct piglit_subtest subtests[] = {
	{
		"nv_conservative_raster_dilate-normal",
		"nv_conservative_raster_dilate-normal",
		(enum piglit_result(*)(void*))&test,
		""
	},
	{
		"nv_conservative_raster_dilate-dilate",
		"nv_conservative_raster_dilate-dilate",
		(enum piglit_result(*)(void*))&test,
		"dilate"
	},
	{0},
};

void piglit_init(int argc, char **argv)
{
	const char **selected_subtests = NULL;
	size_t num_selected_subtests = 0;

	piglit_require_extension("GL_NV_conservative_raster_dilate");

	program = piglit_build_simple_program(
#ifdef PIGLIT_USE_OPENGL
			"#version 120\n"
#else
			"#version 100\n"
			"precision highp float;\n"
#endif
			"attribute vec2 piglit_vertex;"
			"void main() { gl_Position = vec4(piglit_vertex, 0.0, 1.0); }\n",
#ifdef PIGLIT_USE_OPENGL
			"#version 120\n"
#else
			"#version 100\n"
			"precision highp float;\n"
#endif
			"void main() { gl_FragColor = vec4(1.0); }\n");

	glGenBuffers(1, &buf_triangle);
	glBindBuffer(GL_ARRAY_BUFFER, buf_triangle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts_triangle), verts_triangle, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)0);

	num_selected_subtests = piglit_get_selected_tests(&selected_subtests);
	piglit_report_result(piglit_run_selected_subtests(subtests, selected_subtests,
                                                      num_selected_subtests, PIGLIT_SKIP));
}

enum piglit_result piglit_display(void)
{
	return PIGLIT_FAIL;
}
