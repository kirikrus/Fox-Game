varying vec3 Normal;
varying vec3 Position;


uniform vec3 Ia;
uniform vec3 Id;
uniform vec3 Is;

uniform vec3 light_pos;

uniform vec3 ma;
uniform vec3 md;
uniform vec4 ms;

uniform vec3 camera;

void main(void)
{
	gl_FragColor = vec4(0.0,0.0,0.0,1.0);
	
	vec3 color_amb = Ia*ma;
	
	vec3 light_vector = normalize(light_pos-Position);
	vec3 color_dif = Id*md*dot( light_vector,Normal  );
	
	vec3 view_vector = normalize(camera - Position);
	vec3 r = reflect(light_vector,Normal);
	vec3 color_spec = Is*ms.xyz*pow(max(0.0,dot(-r,view_vector)),ms.w);
	
	
	
	gl_FragColor = vec4(color_amb + color_dif + color_spec,1.0);
	
	
}