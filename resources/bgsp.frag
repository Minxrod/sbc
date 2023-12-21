uniform sampler2D texture;
uniform sampler2D colors;
uniform bool grp_mode;
uniform float colbank; //0-5

void main()
{
	vec4 input_pixel = texture2D(texture, gl_TexCoord[0].xy);
	
	float col = colbank / 6.0;
	
	vec4 c;
	if (!grp_mode){
		//gl_Color is the vertex color. The red/first component of this is the palette
		float col2 = float(input_pixel.r != 0.0) * gl_Color.x;
		
		c = texture2D(colors, vec2(col2 + input_pixel.r, col));
	} else {
		c = texture2D(colors, vec2(input_pixel.r, colbank/6.0));
	}
	
	gl_FragColor = c;
}
