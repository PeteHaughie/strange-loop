OF_GLSL_SHADER_HEADER

uniform sampler2D tex0;

uniform float in1;

uniform float in2;

uniform float dispX;

uniform float dispY;

uniform vec2 resolution;

vec3 hsv2rgb(vec3 c) {
  vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
  vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
  return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec3 rgb2hsv(vec3 c) {
  vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
  vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
  vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

  float d = q.x - min(q.w, q.y);
  float e = 1.0e-10;
  return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

void main() {
  vec2 uv = gl_FragCoord.xy / resolution.xy;
  uv = clamp(uv, 0.0, 1.0);
  // NEGATIVE
  vec4 color = texture2D(tex0, uv + vec2(dispX, dispY));

  vec3 invert = rgb2hsv(color.rgb);
  invert.z = 1.0 - invert.z + in1;
  invert.x = 1.0 - invert.x + in2;

  // color.rgb = hsv2rgb(invert.xyz);

  gl_FragColor = color;
}
