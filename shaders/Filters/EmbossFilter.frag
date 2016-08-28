uniform sampler2D sourceTex;
uniform float HFrac;
uniform float VFrac;

#define KERNEL_SIZE 9

float kernel[KERNEL_SIZE];
vec2 offset[KERNEL_SIZE];


void main()
{

  int i = 0;
  vec4 sum = vec4(0);

  offset[0] = vec2(-HFrac,-VFrac);
  offset[1] = vec2(0, -VFrac);
  offset[2] = vec2(HFrac, -VFrac);

  offset[3] = vec2(-HFrac, 0);
  offset[4] = vec2(0, 0);
  offset[5] = vec2(HFrac, 0);
  
  offset[6] = vec2(-HFrac, VFrac);
  offset[7] = vec2(0, VFrac);
  offset[8] = vec2(HFrac, VFrac);

  kernel[0] = 2.0; kernel[1] = 0.0; kernel[2] = 0.0;
  kernel[3] = 0.0; kernel[4] = -1.0; kernel[5] = 0.0;
  kernel[6] = 0.0; kernel[7] = 0.0; kernel[8] = -1.0;

  for (i = 0; i < KERNEL_SIZE; i++)
  {
    vec4 tmp = texture2D(sourceTex, gl_TexCoord[0].st + offset[i]);
    sum += tmp * kernel[i];
    
  }
  sum += 0.5;
  gl_FragColor = sum;

}