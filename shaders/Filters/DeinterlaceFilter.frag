uniform sampler2D sourceTex;
uniform float HFrac;
uniform float VFrac;

void main()
{
  vec4 c0 = texture2D(sourceTex, gl_TexCoord[0].st);
  vec2 h = vec2(0, 1*VFrac);
  vec4 c1 = texture2D(sourceTex, gl_TexCoord[0].st-h);
  vec4 c2 = texture2D(sourceTex, gl_TexCoord[0].st+h);
  gl_FragColor = (c0*2+c1+c2)/4;
}