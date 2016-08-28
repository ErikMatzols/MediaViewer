uniform sampler2D sourceTex;
uniform float brightness;
uniform float contrast;

void main()
{
  vec4 pixel = texture2D(sourceTex, gl_TexCoord[0].st);
  pixel += brightness;
  pixel = pixel * (1.0f+contrast)/1.0f;
  gl_FragColor = pixel;
}