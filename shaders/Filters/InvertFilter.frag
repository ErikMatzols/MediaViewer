uniform sampler2D sourceTex;
uniform float HFrac;
uniform float VFrac;

void main()
{
  gl_FragColor = vec4(1, 1, 1, 1) - texture2D(sourceTex, gl_TexCoord[0].st);
}