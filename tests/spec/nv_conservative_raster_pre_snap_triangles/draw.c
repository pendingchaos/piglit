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
 * This test verifies that GL_CONSERVATIVE_RASTER_MODE_PRE_SNAP_TRIANGLES_NV is
 * implemented correctly for GL_NV_conservative_raster_pre_snap_triangles. It
 * was created with a window size of 250x250 in mind and may not work with
 * other sizes.
 *
 * The test works by probing various pixels of a rendered primitive that differ
 * between the image with the tested feature enabled and one with it disabled.
 */

static const struct piglit_subtest subtests[];

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 21;
	config.supports_gl_es_version = 20;
	config.subtests = subtests;
	config.window_width = 250;
	config.window_height = 250;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

GLuint program, buf_triangle, buf_zero;

static const float verts_triangle[6] = {
	-0.399991212, -0.799982424,
	 0.799982424,  0.399991212,
	 0.399991212, -0.799982424};

static const float verts_zero[6] = {
	0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

static const float white[3] = {1.0f, 1.0f, 1.0f};
static const float black[3] = {0.0f, 0.0f, 0.0f};

static enum piglit_result
test(const char* data)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindBuffer(GL_ARRAY_BUFFER, strcmp(data, "0")?buf_triangle:buf_zero);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)0);

	glUseProgram(program);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	if (strcmp(data, "0") == 0) {
		bool pass = piglit_probe_pixel_rgb(125, 125, white);
		return pass ? PIGLIT_PASS : PIGLIT_FAIL;
	} else {
		bool pass = piglit_probe_pixel_rgb(177, 30, white);
		pass = pass && piglit_probe_pixel_rgb(177, 29, black);
		return pass ? PIGLIT_PASS : PIGLIT_FAIL;
	}
}

static const struct piglit_subtest subtests[] = {
	{
		"nv_conservative_raster_pre_snap_triangles-degenerate",
		"nv_conservative_raster_pre_snap_triangles-degenerate",
		(enum piglit_result(*)(void*))&test,
		"0"
	},
	{
		"nv_conservative_raster_pre_snap_triangles-normal",
		"nv_conservative_raster_pre_snap_triangles-normal",
		(enum piglit_result(*)(void*))&test,
		""
	},
	{0},
};

void
piglit_init(int argc, char **argv)
{
	const char **selected_subtests = NULL;
	size_t num_selected_subtests = 0;

	piglit_require_extension("GL_NV_conservative_raster_pre_snap_triangles");

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
	glGenBuffers(1, &buf_zero);
	glBindBuffer(GL_ARRAY_BUFFER, buf_zero);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts_zero), verts_zero, GL_STATIC_DRAW);

	glEnable(GL_CONSERVATIVE_RASTERIZATION_NV);
	glConservativeRasterParameteriNV(GL_CONSERVATIVE_RASTER_MODE_NV, GL_CONSERVATIVE_RASTER_MODE_PRE_SNAP_TRIANGLES_NV);

	num_selected_subtests = piglit_get_selected_tests(&selected_subtests);
	piglit_report_result(piglit_run_selected_subtests(subtests, selected_subtests,
	                                                  num_selected_subtests, PIGLIT_SKIP));
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}
