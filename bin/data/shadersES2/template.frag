OF_GLSL_SHADER_HEADER

uniform sampler2D tex0;
//in1 and in2 are linked to the third knob and
// the left/right axis of the analog, they go from
// 0.0 to 1.0
uniform float in1; 

uniform float in2;
//dispX and dispY move the framebuffer, they are linked
//to the left analog two axis.
uniform float dispX;

uniform float dispY;

uniform vec2 resolution;

void main(){
	vec2 uv = gl_FragCoord.xy / resolution.xy;
	uv = clamp(uv, 0.0, 1.0);
	// vec4 color = texture2D(tex0, uv + vec2(dispX, dispY));
	// float in1Scaled = (in1 * /*some value*/ ) + /*some value*/;  
	// float in2Scaled = (in2 * /*some value*/ ) + /*some value*/ ;
	
	// color.rgb = /* do something with the two scaled and biased inputs*/;
	gl_FragColor = vec4(texture2D(tex0, uv));
}
