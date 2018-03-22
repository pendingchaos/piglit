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

void piglit_init(int argc, char **argv)
{
	GLint max_subpixel_bits_bias;

	piglit_require_extension("GL_NV_conservative_raster");

	glGetIntegerv(GL_MAX_SUBPIXEL_PRECISION_BIAS_BITS_NV, &max_subpixel_bits_bias);
	if (max_subpixel_bits_bias<3)
		piglit_report_result(PIGLIT_SKIP);
}

enum piglit_result piglit_display(void)
{
	bool enable_pass, viewport_pass;
	GLint biasx, biasy;
	glEnable(GL_CONSERVATIVE_RASTERIZATION_NV);
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_CONSERVATIVE_RASTERIZATION_NV);
	glPopAttrib();

	enable_pass = glIsEnabled(GL_CONSERVATIVE_RASTERIZATION_NV);
	piglit_report_subtest_result(enable_pass ? PIGLIT_PASS : PIGLIT_FAIL, "enable attributes");

	glSubpixelPrecisionBiasNV(0, 1);
	glPushAttrib(GL_VIEWPORT_BIT);
	glSubpixelPrecisionBiasNV(2, 3);
	glPopAttrib(GL_VIEWPORT_BIT);

	glGetIntegerv(GL_SUBPIXEL_PRECISION_BIAS_X_BITS_NV, &biasx);
	glGetIntegerv(GL_SUBPIXEL_PRECISION_BIAS_Y_BITS_NV, &biasy);
	viewport_pass = biasx==0 && biasy==1;
	piglit_report_subtest_result(viewport_pass ? PIGLIT_PASS : PIGLIT_FAIL, "viewport attributes");

	return (enable_pass && viewport_pass) ? PIGLIT_PASS : PIGLIT_FAIL;
}
