// [config]
// expect_result: pass
// glsl_version: 1.50
// check_link: true
// require_extensions: GL_ARB_enhanced_layouts
// [end config]
//
// From the GL_ARB_enhanced_layouts spec:
//
//    "While *xfb_stride* can be declared multiple times for the same buffer,
//     it is a compile-time or link-time error to have different values
//     specified for the stride for the same buffer."

#version 150
#extension GL_ARB_enhanced_layouts: require

layout(xfb_stride = 20) out block1 {
  vec4 var;
} b1;

layout(xfb_buffer = 0, xfb_stride = 20) out block2 {
  vec4 var2;
} b2;

void main()
{
  b1.var = vec4(1.0);
  b2.var2 = vec4(0.0);
}
