uniform sampler2D sourceTex;
uniform float HFrac;
uniform float VFrac;

void main(void)
{
  vec4 c9 = texture2D(sourceTex, gl_TexCoord[0].st) * 2.0;

  vec4 c1 = texture2D(sourceTex, gl_TexCoord[0].st + vec2(-HFrac,-VFrac))*0.1;
  c1 += texture2D(sourceTex, gl_TexCoord[0].st + vec2(0, -VFrac)) * 0.15;
  c1 += texture2D(sourceTex, gl_TexCoord[0].st + vec2(HFrac, -VFrac)) * 0.1;
  c1 += texture2D(sourceTex, gl_TexCoord[0].st + vec2(-HFrac,0)) * 0.15;
  c1 += texture2D(sourceTex, gl_TexCoord[0].st + vec2(HFrac,0)) * 0.15;
  c1 += texture2D(sourceTex, gl_TexCoord[0].st + vec2(-HFrac,VFrac))*0.1;
  c1 += texture2D(sourceTex, gl_TexCoord[0].st + vec2(0, VFrac)) * 0.15;
  c1 += texture2D(sourceTex, gl_TexCoord[0].st + vec2(HFrac,VFrac)) * 0.1;

  gl_FragColor = c9 - c1;
}