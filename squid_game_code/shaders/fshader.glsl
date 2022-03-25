#version 330 core

// 给光源数据一个结构体
struct Light{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;

	vec3 position;
    // 光源衰减系数的三个参数
    float constant; // 常数项
    float linear;	// 一次项
    float quadratic;// 二次项

};

// 给物体材质数据一个结构体
struct Material{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;

	float shininess;
};

in vec3 position;
in vec3 normal;
in vec2 texCoord;
in vec4 fragPosLight;

// 相机坐标
uniform vec3 eye_position;
// 光源
uniform Light light;
// 物体材质
uniform Material material;

uniform int isShadow;
// 纹理数据
uniform sampler2D texture1;
uniform sampler2D shadowMap;
uniform vec4 camPos;

out vec4 fColor;

vec4 pointLight(){
	
	vec3 lightDec = light.position-position;
	float dist = length(lightDec);
	float attenuation = 1.0f/((light.quadratic*dist+light.linear)*dist+light.constant);

	vec3 N = normalize(normal);
	vec3 V = normalize(eye_position-position);
	vec3 L = normalize(light.position-position);
	vec3 R = reflect(-1*light.position,N);

	vec4 texColor = texture2D(texture1,texCoord);
	
	// 环境光
	vec4 I_a = light.ambient * texColor * material.ambient;

	// 漫反射
	float diffuse_dot = max(dot(L,N),0);
	vec4 I_d = diffuse_dot *  light.diffuse * texColor * material.diffuse;

	// 镜面反射
	float specular_dot_pow = pow(max(dot(R,V),0),material.shininess);
	vec4 I_s = specular_dot_pow * light.specular * texColor * material.specular;

	if( dot(L, N) < 0.0 ) {
		    I_s = vec4(0.0, 0.0, 0.0, 1.0);
		}

	vec4 I = I_a + attenuation*(I_d + I_s);
	return I;
}

void main()
{
	if (isShadow == 1) {
		fColor = vec4(0.45, 0.45, 0.45, 1.0);// shadow's color
	}
	else {
		
		fColor = pointLight();
	}
}
