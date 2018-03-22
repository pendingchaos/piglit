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

PIGLIT_GL_TEST_CONFIG_BEGIN

#if PIGLIT_USE_OPENGL
	config.supports_gl_compat_version = 21;
#else
	config.supports_gl_es_version = 20;
#endif

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

static const float white[3] = {1.0f, 1.0f, 1.0f};
static const float black[3] = {0.0f, 0.0f, 0.0f};

void piglit_init(int argc, char **argv)
{
	GLint max_subpixel_bits_bias;

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
}

static void draw(GLenum prim)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
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

static bool test_triangle(bool smooth, bool conservative, bool xbias, bool ybias) {
	bool c1 = conservative ? 1 : 0;
	if (smooth) {
		glEnable(GL_BLEND);
		glEnable(GL_POLYGON_SMOOTH);
	}
	if (conservative)
		glEnable(GL_CONSERVATIVE_RASTERIZATION_NV);
	glSubpixelPrecisionBiasNV(xbias?8:0, ybias?8:0);
	draw(GL_TRIANGLES);
	int c = 0;
	if (ybias) {
		c += piglit_probe_pixel_rgb(58, 25, white);
		c += piglit_probe_pixel_rgb(58, 24, black);
	} else {
		c += piglit_probe_pixel_rgb(58, 25-c1, white);
		c += piglit_probe_pixel_rgb(58, 24-c1, black);
	}
	c += piglit_probe_pixel_rgb(180, 180+c1, white);
	c += piglit_probe_pixel_rgb(180, 181+c1, black);
	if (xbias) {
		c += piglit_probe_pixel_rgb(224, 189, white);
		c += piglit_probe_pixel_rgb(225, 189, black);
	} else {
		c += piglit_probe_pixel_rgb(224+c1, 189, white);
		c += piglit_probe_pixel_rgb(225+c1, 189, black);
	}
	if (xbias && ybias)
		c += piglit_probe_pixel_rgb(224, 24, black);
	else if (xbias)
		c += piglit_probe_pixel_rgb(224, 225, white);
	else if (ybias)
		c += piglit_probe_pixel_rgb(225, 225, black);
	else
		c += piglit_probe_pixel_rgb(225, 225, conservative?white:black);
	return c == 7;
}

static bool test_lines(bool smooth, bool conservative, bool xbias, bool ybias) {
	int c1 = conservative ? 1 : 0;
	if (smooth) {
		glEnable(GL_BLEND);
		glEnable(GL_LINE_SMOOTH);
	}
	if (conservative)
		glEnable(GL_CONSERVATIVE_RASTERIZATION_NV);
	glSubpixelPrecisionBiasNV(xbias?8:0, ybias?8:0);
	draw(GL_LINE_STRIP);
	int c = piglit_probe_pixel_rgb(224+c1, 149, white);
	c += piglit_probe_pixel_rgb(225+c1, 149, black);
	c += piglit_probe_pixel_rgb(100, 25-c1, white);
	c += piglit_probe_pixel_rgb(100, 24-c1, black);
	if (conservative) {
		c += piglit_probe_pixel_rgb(24, 24, xbias?black:white);
		c += piglit_probe_pixel_rgb(225, 225, ybias?black:white);
		c += piglit_probe_pixel_rgb(225, 24, (xbias&&ybias)?black:white);
	} else {
		c += 3;
	}
	return c == 7;
}

static bool test_line_stipple(bool _0, bool _1, bool _2, bool _3) {
	glEnable(GL_CONSERVATIVE_RASTERIZATION_NV);
	glEnable(GL_LINE_STIPPLE);
	glLineStipple(3, 0xaaaa);
	draw(GL_LINE_STRIP);
	return piglit_probe_pixel_rgb(26, 25, white);
}

static bool test_point(bool smooth, bool conservative, bool _0, bool _1) {
	if (smooth) {
		glEnable(GL_BLEND);
		glEnable(GL_POINT_SMOOTH);
	}
	if (conservative)
		glEnable(GL_CONSERVATIVE_RASTERIZATION_NV);
	draw(GL_POINTS);
	int c = 0;
	if (conservative) {
		c += piglit_probe_pixel_rgb(124, 128, white);
		c += piglit_probe_pixel_rgb(124, 129, black);
		c += piglit_probe_pixel_rgb(128, 124, white);
		c += piglit_probe_pixel_rgb(129, 124, black);
	} else {
		c += piglit_probe_pixel_rgb(123, 123, white);
		c += piglit_probe_pixel_rgb(122, 123, black);
		c += piglit_probe_pixel_rgb(125, 127, white);
		c += piglit_probe_pixel_rgb(125, 128, black);
	}
	return c == 4;
}

static bool run_test(bool (*func)(bool, bool, bool, bool), const char* name,
					 bool smooth, bool conservative, bool xbias, bool ybias) {
	glDisable(GL_CONSERVATIVE_RASTERIZATION_NV);
	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#if PIGLIT_USE_OPENGL
	glDisable(GL_POLYGON_SMOOTH);
	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_LINE_STIPPLE);
	glPointSize(5.0);
#endif
	glSubpixelPrecisionBiasNV(0, 0);
	
	bool pass = func(smooth, conservative, xbias, ybias);
	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL, name);
	
	return pass;
}

static bool degenerate_test() {
	glEnable(GL_CONSERVATIVE_RASTERIZATION_NV);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindBuffer(GL_ARRAY_BUFFER, buf_degenerate);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)0);

	glUseProgram(program);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	bool pass = piglit_probe_pixel_rgb(125, 125, black);
	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL, "degenerate triangle");

	return pass;
}

enum piglit_result piglit_display(void)
{
	bool pass[17];
	for (int i = 0; i < 17; i++) pass[i] = true;

	pass[0] = run_test(&test_triangle, "normal triangle", false, false, false, false);
	pass[1] = run_test(&test_triangle, "bias 0 0 triangle", false, true, false, false);
	pass[2] = run_test(&test_triangle, "bias 0 8 triangle", false, true, false, true);
	pass[3] = run_test(&test_triangle, "bias 8 0 triangle", false, true, true, false);
	pass[4] = run_test(&test_triangle, "bias 8 8 triangle", false, true, true, true);
#if PIGLIT_USE_OPENGL
	pass[5] = run_test(&test_triangle, "smooth triangle", true, true, false, false);
#endif
	pass[6] = run_test(&test_lines, "normal line", false, false, false, false);
	pass[7] = run_test(&test_lines, "bias 0 0 line", false, true, false, false);
	pass[8] = run_test(&test_lines, "bias 0 8 line", false, true, false, true);
	pass[9] = run_test(&test_lines, "bias 8 0 line", false, true, true, false);
	pass[10] = run_test(&test_lines, "bias 8 8 line", false, true, true, true);
#if PIGLIT_USE_OPENGL
	pass[11] = run_test(&test_lines, "smooth line", true, true, false, false);
	pass[12] = run_test(&test_line_stipple, "line stipple", false, false, false, false);
	pass[13] = run_test(&test_point, "normal point", false, false, false, false);
	pass[14] = run_test(&test_point, "bias 0 0 point", false, true, false, false);
	pass[15] = run_test(&test_point, "smooth point", true, true, false, false);
#endif
	pass[16] = degenerate_test();
	piglit_present_results();

	for (int i = 0; i < 17; i++) {
		if (!pass[i]) return PIGLIT_FAIL;
	}
	return PIGLIT_PASS;
}
