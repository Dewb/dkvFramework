#version 110
varying float depth;
uniform float clipDistance;

varying float baseline;

uniform sampler2D colorTex;
uniform vec2 offset;
uniform vec2 scale;
uniform float depthBrightness;
uniform float colorize;


float Hue_2_RGB_helper(float v1, float v2, float vH )
{
	float ret;
   if ( vH < 0.0 )
     vH += 1.0;
   if ( vH > 1.0 )
     vH -= 1.0;
   if ( ( 6.0 * vH ) < 1.0 )
     ret = ( v1 + ( v2 - v1 ) * 6.0 * vH );
   else if ( ( 2.0 * vH ) < 1.0 )
     ret = ( v2 );
   else if ( ( 3.0 * vH ) < 2.0 )
     ret = ( v1 + ( v2 - v1 ) * ( ( 2.0 / 3.0 ) - vH ) * 6.0 );
   else
     ret = v1;
   return ret;
}

vec3 Hue_2_RGB(vec3 hsl)
{
	float H = hsl[0], S = hsl[1], L = hsl[2];
	float var_2, var_1;
	float R, G, B;
	
	if (S == 0.0)
	{
		R = L;
		G = L;
		B = L;
	}
	else
	{
		if ( L < 0.5 )
		{
			var_2 = L * ( 1.0 + S );
		}
		else
		{
			var_2 = ( L + S ) - ( S * L );
		}

		var_1 = 2.0 * L - var_2;

		R = Hue_2_RGB_helper( var_1, var_2, H + ( 1.0 / 3.0 ) );
		G = Hue_2_RGB_helper( var_1, var_2, H );
		B = Hue_2_RGB_helper( var_1, var_2, H - ( 1.0 / 3.0 ) );
	}

	vec3 color;
	color.r = R;
	color.g = G;
	color.b = B;

	return color;
}


vec3 RGB_2_Hue(vec3 color)
{
	float R, G, B, H, S, L;
    R = color.r;
    G = color.g;
    B = color.b;

	// convert to HSL

    float Cmax = max (R, max (G, B));
    float Cmin = min (R, min (G, B));

// calculate lightness
    L = (Cmax + Cmin) / 2.0;

	if (Cmax == Cmin) // it's grey
	{
		H = 0.0; // it's actually undefined
		S = 0.0;
	}
	else
	{
		float D = Cmax - Cmin; // we know D != 0 so we cas safely divide by it

		// calculate Saturation
		if (L < 0.5)
		{
			S = D / (Cmax + Cmin);
		}
		else
		{
			S = D / (2.0 - (Cmax + Cmin));
		}

		// calculate Hue
		if (R == Cmax)
		{
			H = (G - B) / D;
		} else {
			if (G == Cmax)
			{
				H = 2.0 + (B - R) /D;
			}
			else
			{
				H = 4.0 + (R - G) / D;
			}
		}

		H = H / 6.0;
	}
	vec3 ret;
	ret[0] = H;
	ret[1] = S;
	ret[2] = L;
	return ret;
}


void main()
{

	if( depth < clipDistance ) discard;
	
	float db = depthBrightness;

	if (baseline != 0.0)
	{
	  db = db/2.0 - 0.2;
	}
	  	
    vec2 coord = gl_TexCoord[0].st + offset;
	coord[0] *= scale[0];
	coord[1] *= scale[1];

	vec3 color = texture2D( colorTex, coord ).rgb;
	
	if (colorize > 0.0)
	{
		vec3 hsl = RGB_2_Hue(color);
		hsl[1] = 1.0;
		hsl[2] = 0.6;
		color = Hue_2_RGB(hsl);
	}
	
    gl_FragColor.rgb =  color + vec3(depth*db);

}
