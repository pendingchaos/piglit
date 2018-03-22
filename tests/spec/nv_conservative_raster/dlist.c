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
	config.supports_gl_compat_version = 10;
#endif

PIGLIT_GL_TEST_CONFIG_END

static GLuint list;

void piglit_init(int argc, char **argv)
{
	GLint max_subpixel_bits_bias;

	piglit_require_extension("GL_NV_conservative_raster");

	glGetIntegerv(GL_MAX_SUBPIXEL_PRECISION_BIAS_BITS_NV, &max_subpixel_bits_bias);
	if (max_subpixel_bits_bias<3)
		piglit_report_result(PIGLIT_SKIP);

	list = glGenLists(1);
	glNewList(list, GL_COMPILE);
	glSubpixelPrecisionBiasNV(0, 1);
	glEndList();
}

enum piglit_result piglit_display(void)
{
	bool dlist_pass[2];
	GLint biasx, biasy;

	glCallList(list);
	glGetIntegerv(GL_SUBPIXEL_PRECISION_BIAS_X_BITS_NV, &biasx);
	glGetIntegerv(GL_SUBPIXEL_PRECISION_BIAS_Y_BITS_NV, &biasy);

	dlist_pass[0] = biasx==0 && biasy==1;
	piglit_report_subtest_result(dlist_pass[0] ? PIGLIT_PASS : PIGLIT_FAIL, "compiled display list");

	glNewList(list, GL_COMPILE_AND_EXECUTE);
	glSubpixelPrecisionBiasNV(2, 3);
	glEndList();
	glGetIntegerv(GL_SUBPIXEL_PRECISION_BIAS_X_BITS_NV, &biasx);
	glGetIntegerv(GL_SUBPIXEL_PRECISION_BIAS_Y_BITS_NV, &biasy);

	dlist_pass[1] = biasx==2 && biasy==3;
	piglit_report_subtest_result(dlist_pass[1] ? PIGLIT_PASS : PIGLIT_FAIL, "compile/execute display list");

	return (dlist_pass[0] && dlist_pass[1]) ? PIGLIT_PASS : PIGLIT_FAIL;
}
