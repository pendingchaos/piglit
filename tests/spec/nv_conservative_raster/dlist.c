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
 * @file dlist.c
 *
 * This test verifies that display lists are implemented correctly for
 * GL_NV_conservative_raster.
 */

static const struct piglit_subtest subtests[];

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.subtests = subtests;

PIGLIT_GL_TEST_CONFIG_END

static GLuint compiled_list;

static enum piglit_result test_compiled(void* data)
{
	GLint biasx, biasy;
	glCallList(compiled_list);
	glGetIntegerv(GL_SUBPIXEL_PRECISION_BIAS_X_BITS_NV, &biasx);
	glGetIntegerv(GL_SUBPIXEL_PRECISION_BIAS_Y_BITS_NV, &biasy);
	return (biasx==0 && biasy==1) ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result test_exec(void* data)
{
	GLint biasx, biasy;
	glNewList(glGenLists(1), GL_COMPILE_AND_EXECUTE);
	glSubpixelPrecisionBiasNV(2, 3);
	glEndList();
	glGetIntegerv(GL_SUBPIXEL_PRECISION_BIAS_X_BITS_NV, &biasx);
	glGetIntegerv(GL_SUBPIXEL_PRECISION_BIAS_Y_BITS_NV, &biasy);
	return (biasx==2 && biasy==3) ? PIGLIT_PASS : PIGLIT_FAIL;
}

static const struct piglit_subtest subtests[] = {
	{
		"nv_conservative_raster-compiled_dlist",
		"nv_conservative_raster-compiled_dlist",
		test_compiled,
		NULL
	},
	{
		"nv_conservative_raster-exec_dlist",
		"nv_conservative_raster-exec_dlist",
		test_exec,
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
	if (max_subpixel_bits_bias<3)
		piglit_report_result(PIGLIT_SKIP);

	compiled_list = glGenLists(1);
	glNewList(compiled_list, GL_COMPILE);
	glSubpixelPrecisionBiasNV(0, 1);
	glEndList();

	num_selected_subtests = piglit_get_selected_tests(&selected_subtests);
	piglit_report_result(piglit_run_selected_subtests(subtests, selected_subtests,
                                                      num_selected_subtests, PIGLIT_SKIP));
}

enum piglit_result piglit_display(void)
{
	return PIGLIT_FAIL;
}
