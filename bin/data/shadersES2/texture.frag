OF_GLSL_SHADER_HEADER

uniform sampler2D tex0;

uniform vec2 resolution;

void main(){
	vec2 uv = gl_FragCoord.xy / resolution.xy;
	uv = clamp(uv, 0.0, 1.0);
	gl_FragColor = vec4(texture2D(tex0, uv));
}
