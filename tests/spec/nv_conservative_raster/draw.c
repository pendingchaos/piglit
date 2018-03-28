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
 * This test verifies that primitives are rasterized correctly with conservative
 * rasterization enabled.
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

GLuint program, buf_triangle, buf_lines, buf_point, buf_degenerate;

static const float verts_triangle[6] = {
	-0.799998779, -0.799998779,
	 0.799998779,  0.799998779,
	 0.799998779, -0.799998779};

static const float verts_lines[6] = {
	 0.799998779,  0.799998779,
	 0.799998779, -0.799998779,
	-0.799998779, -0.799998779};

static const float verts_point[2] = {
	 0.001601953, 0.001601953};

static const float verts_degenerate[6] = {
	 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

static const float white[4] = {1.0f, 1.0f, 1.0f, 1.0f};
static const float black[4] = {0.0f, 0.0f, 0.0f, 1.0f};

static void draw(GLenum prim)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	switch (prim) {
	case GL_TRIANGLES:
		glBindBuffer(GL_ARRAY_BUFFER, buf_triangle);
	break;
	case GL_LINE_STRIP:
		glBindBuffer(GL_ARRAY_BUFFER, buf_lines);
	break;
	case GL_POINTS:
		glBindBuffer(GL_ARRAY_BUFFER, buf_point);
	break;
	}
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)0);

	glUseProgram(program);
	glDrawArrays(prim, 0, prim==GL_POINTS?1:3);
}

static bool test_triangle(bool conservative, bool xbias, bool ybias)
{
	bool c1 = conservative ? 1 : 0;
	draw(GL_TRIANGLES);
	int c = 0;
	if (ybias) {
		c += piglit_probe_pixel_rgba(58, 25, white);
		c += piglit_probe_pixel_rgba(58, 24, black);
	} else {
		c += piglit_probe_pixel_rgba(58, 25-c1, white);
		c += piglit_probe_pixel_rgba(58, 24-c1, black);
	}
	if (xbias != ybias) { //precision problems mess up these probes when xbias and ybias differ
		c += 2;
	} else {
		c += piglit_probe_pixel_rgba(180, 180+c1, white);
		c += piglit_probe_pixel_rgba(180, 181+c1, black);
	}
	if (xbias) {
		c += piglit_probe_pixel_rgba(224, 189, white);
		c += piglit_probe_pixel_rgba(225, 189, black);
	} else {
		c += piglit_probe_pixel_rgba(224+c1, 189, white);
		c += piglit_probe_pixel_rgba(225+c1, 189, black);
	}
	if (xbias && ybias)
		c += piglit_probe_pixel_rgba(224, 24, black);
	else if (xbias)
		c += piglit_probe_pixel_rgba(224, 225, white);
	else if (ybias)
		c += piglit_probe_pixel_rgba(225, 225, black);
	else
		c += piglit_probe_pixel_rgba(225, 225, conservative?white:black);
	return c == 7;
}

static bool test_lines(bool conservative, bool xbias, bool ybias)
{
	int c1 = conservative ? 1 : 0;
	draw(GL_LINE_STRIP);
	int c = piglit_probe_pixel_rgba(224+c1, 149, white);
	c += piglit_probe_pixel_rgba(225+c1, 149, black);
	c += piglit_probe_pixel_rgba(100, 25-c1, white);
	c += piglit_probe_pixel_rgba(100, 24-c1, black);
	if (conservative) {
		c += piglit_probe_pixel_rgba(24, 24, xbias?black:white);
		c += piglit_probe_pixel_rgba(225, 225, ybias?black:white);
		c += piglit_probe_pixel_rgba(225, 24, (xbias&&ybias)?black:white);
	} else {
		c += 3;
	}
	return c == 7;
}

static bool test_line_stipple()
{
	draw(GL_LINE_STRIP);
	return piglit_probe_pixel_rgba(26, 25, white);
}

static bool test_point(bool conservative, bool _0, bool _1)
{
	draw(GL_POINTS);
	int c = 0;
	if (conservative) {
		c += piglit_probe_pixel_rgba(124, 128, white);
		c += piglit_probe_pixel_rgba(124, 129, black);
		c += piglit_probe_pixel_rgba(128, 124, white);
		c += piglit_probe_pixel_rgba(129, 124, black);
	} else {
		c += piglit_probe_pixel_rgba(123, 123, white);
		c += piglit_probe_pixel_rgba(122, 123, black);
		c += piglit_probe_pixel_rgba(125, 127, white);
		c += piglit_probe_pixel_rgba(125, 128, black);
	}
	return c == 4;
}

static enum piglit_result test(const char* data) {
	bool bias[2] = {strstr(data, "x"), strstr(data, "y")};
	bool conservative = strstr(data, "c");
	bool pass = false;

    //Workaround for when normal_line renders slightly differently with -fbo
    //(like with test_triangle()'s precision problems when xbias!=ybias)
	if (strcmp(data, "l")==0 && piglit_use_fbo)
		return PIGLIT_SKIP;

	if (conservative)
		glEnable(GL_CONSERVATIVE_RASTERIZATION_NV);
	else
		glDisable(GL_CONSERVATIVE_RASTERIZATION_NV);
	#if PIGLIT_USE_OPENGL
	if (strstr(data, "s")) {
		glEnable(GL_BLEND);
		glEnable(GL_POLYGON_SMOOTH);
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_POINT_SMOOTH);
	} else {
		glDisable(GL_BLEND);
		glDisable(GL_POLYGON_SMOOTH);
		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_POINT_SMOOTH);
	}
	if (strstr(data, "i"))
		glEnable(GL_LINE_STIPPLE);
	else
		glDisable(GL_LINE_STIPPLE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glLineStipple(3, 0xaaaa);
	glPointSize(5.0);
	#endif

	glSubpixelPrecisionBiasNV(bias[0]?8:0, bias[1]?8:0);

	if (strstr(data, "t"))
		pass = test_triangle(conservative, bias[0], bias[1]);
	else if (strstr(data, "l"))
		pass = test_lines(conservative, bias[0], bias[1]);
	else if (strstr(data, "p"))
		pass = test_point(conservative, bias[0], bias[1]);
	else if (strstr(data, "i"))
		pass = test_line_stipple();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result test_degenerate(void* data) {
	glEnable(GL_CONSERVATIVE_RASTERIZATION_NV);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindBuffer(GL_ARRAY_BUFFER, buf_degenerate);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)0);

	glUseProgram(program);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	return piglit_probe_pixel_rgba(125, 125, black) ? PIGLIT_PASS : PIGLIT_FAIL;
}

//test data:
//t=triangle, l=line, p=point i=line stipple
//s=smooth, c=conservative, x=xbias, y=ybias
static const struct piglit_subtest subtests[] = {
#define ST(name, data) {\
		"nv_conservative_raster-"name,\
		"nv_conservative_raster-"name,\
		(enum piglit_result(*)(void*))&test,\
		data\
	},
	ST("normal_triangle", "t")
	ST("bias00_triangle", "tc")
	ST("bias80_triangle", "tcx")
	ST("bias08_triangle", "tcy")
	ST("bias88_triangle", "tcxy")
#if PIGLIT_USE_OPENGL
	ST("smooth_triangle", "tsc")
#endif
	ST("normal_line", "l")
	ST("bias00_line", "lc")
	ST("bias80_line", "lcx")
	ST("bias08_line", "lcy")
	ST("bias88_line", "lcxy")
#if PIGLIT_USE_OPENGL
	ST("smooth_line", "lsc")
	ST("stipple_line", "ic")
	ST("normal_point", "p")
	ST("bias00_point", "pc")
	ST("smooth_point", "psc")
#endif
#undef ST
	{
		"nv_conservative_raster-degenerate_triangle",
		"nv_conservative_raster-degenerate_triangle",
		&test_degenerate,
		NULL
	},
	{0},
};

void piglit_init(int argc, char **argv)
{
	GLint max_subpixel_bits_bias;
	const char **selected_subtests = NULL;
	size_t num_selected_subtests = 0;

	piglit_require_extension("GL_NV_conservative_raster");

	glGetIntegerv(GL_MAX_SUBPIXEL_PRECISION_BIAS_BITS_NV, &max_subpixel_bits_bias);
	if (max_subpixel_bits_bias<8)
		piglit_report_result(PIGLIT_SKIP);

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
	glGenBuffers(1, &buf_lines);
	glGenBuffers(1, &buf_point);
	glGenBuffers(1, &buf_degenerate);
	glBindBuffer(GL_ARRAY_BUFFER, buf_triangle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts_triangle), verts_triangle, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, buf_lines);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts_lines), verts_lines, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, buf_point);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts_point), verts_point, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, buf_degenerate);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts_degenerate), verts_degenerate, GL_STATIC_DRAW);

	num_selected_subtests = piglit_get_selected_tests(&selected_subtests);
	piglit_report_result(piglit_run_selected_subtests(subtests, selected_subtests,
	                                                  num_selected_subtests, PIGLIT_SKIP));
}

enum piglit_result piglit_display(void)
{
	return PIGLIT_FAIL;
}
