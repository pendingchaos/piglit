/**
 * Copyright © 2013 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/**
 * Test that transform feedback occurs before flatshading
 *
 * Section 2.16(Transform Feedback) of OpenGL 3.2 Core says:
 * "In transform feedback mode, attributes of the vertices of transformed
 * primitives processed by a vertex shader, or primitives generated by a
 * geometry shader if one is active, are written out to one or more buffer
 * objects. The vertices are fed back after vertex color clamping, but before
 * flatshading and clipping."
 *
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
        config.supports_gl_core_version = 32;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static const char *vstext = 
	"#version 130\n"
	"in vec3 vert1;\n"
	"in vec3 color;\n"
	"flat out vec3 color1;\n"
	"out vec3 vert;\n"
	"void main() {\n"
	"	gl_Position = vec4(vert1, 1.);\n"
	"	vert = vert1;\n"
	"	color1 = color;\n"
	"}\n";

static const char *fstext = 
	"#version 130\n"
	"in vec3 vert;\n"
	"flat in vec3 color1;\n"
	"void main() {\n"
	"	gl_FragColor = vec4(color1, 1.);\n"
	"}\n";

static GLuint vao;
static GLuint vertBuff;
static GLuint indexBuf;
static GLuint vertColorBuf;

static GLfloat vertices[] = {
	-1.0, 1.0, 0.0,
	 1.0, 1.0, 0.0,
	 1.0,-1.0, 0.0,
	-1.0,-1.0, 0.0
};
static GLsizei vertSize = sizeof(vertices);

static GLfloat vertColors[] = {
	1.0, 0.0, 0.0,
	0.0, 1.0, 0.0,
	0.0, 0.0, 1.0,
	0.0, 1.0, 0.0
};
static GLsizei vertColorSize = sizeof(vertColors);

static GLuint indices[] = {
	0, 1, 2, 0, 2, 3
};
static GLsizei indSize = sizeof(indices);

static GLuint xfb_buf;
static GLuint prog;

/* Only capture the color data into the transform feedback buffer */
static const char *varyings[] = { "color1" };

/* Calculate number of floats contained in the transform varyings */
static int numVaryingFloats = ARRAY_SIZE(varyings) * ARRAY_SIZE(indices) * 3;

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLuint vs = 0, fs = 0;
	GLuint vertIndex;
	GLuint vertColorIndex;

	prog = glCreateProgram();
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vstext);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fstext);
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	glTransformFeedbackVaryings(prog, ARRAY_SIZE(varyings), varyings,
					GL_INTERLEAVED_ATTRIBS);
	glLinkProgram(prog);
	if(!piglit_link_check_status(prog)){
		glDeleteProgram(prog);
		piglit_report_result(PIGLIT_FAIL);
	}

	glUseProgram(prog);

	glGenBuffers(1, &vertBuff);
	glBindBuffer(GL_ARRAY_BUFFER, vertBuff);
	glBufferData(GL_ARRAY_BUFFER, vertSize, vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &indexBuf);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indSize,
			indices, GL_STATIC_DRAW);

	glGenBuffers(1, &vertColorBuf);
	glBindBuffer(GL_ARRAY_BUFFER, vertColorBuf);
	glBufferData(GL_ARRAY_BUFFER, vertColorSize,
			vertColors, GL_STATIC_DRAW);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	vertIndex = glGetAttribLocation(prog, "vert1");
	vertColorIndex = glGetAttribLocation(prog, "color");

	glBindBuffer(GL_ARRAY_BUFFER, vertBuff);
	glEnableVertexAttribArray(vertIndex);
	glVertexAttribPointer(vertIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, vertColorBuf);
	glEnableVertexAttribArray(vertColorIndex);
	glVertexAttribPointer(vertColorIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &xfb_buf);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, xfb_buf);
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER,
			numVaryingFloats * sizeof(float),
			NULL, GL_STREAM_READ);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
}

enum piglit_result
piglit_display(void)
{
	float *xBuffer;
	int i;
	bool pass = true;

	glClearColor(0.2, 0.2, 0.2, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);

	glBeginTransformFeedback(GL_TRIANGLES);
	glDrawElements(GL_TRIANGLES, ARRAY_SIZE(indices),
			GL_UNSIGNED_INT, NULL);
	glEndTransformFeedback();

	xBuffer = (float*)glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, GL_READ_ONLY);
	/* Compare transform feedback color data with original color data */
	for(i = 0; i < numVaryingFloats/3; i++) {
		bool unchanged = true;
		if(xBuffer[i*3+0] != vertColors[0+3*indices[i]])
			unchanged = false;
		if(xBuffer[i*3+1] != vertColors[1+3*indices[i]])
			unchanged = false;
		if(xBuffer[i*3+2] != vertColors[2+3*indices[i]])
			unchanged = false;
		if(!unchanged) {
			printf("Incorrect data for vertex %d:\n", i);
			printf("Expected (%f, %f, %f)\n",
				vertColors[0+3*indices[i]],
				vertColors[1+3*indices[i]],
				vertColors[2+3*indices[i]]);
			printf("Actual   (%f, %f, %f)\n",
				xBuffer[i*3+0],
				xBuffer[i*3+1],
				xBuffer[i*3+2]);
			pass = false;
		}
	}
	glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
