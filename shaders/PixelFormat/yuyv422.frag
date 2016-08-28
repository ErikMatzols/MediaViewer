uniform sampler2D YTex;
uniform float Frac;

void main()
{
  float y = texture2D(YTex, gl_TexCoord[0].st).r;
  float u;
  float v;

  if(mod(floor(gl_FragCoord.x),2.0) > 0.5)
  {
    // odd
    u = texture2D(YTex, vec2(-Frac,0.0)+gl_TexCoord[0].st).a;
    v = texture2D(YTex, gl_TexCoord[0].st).a;
  }
  else
  {
    // even
    u = texture2D(YTex, gl_TexCoord[0].st).a;
    v = texture2D(YTex, vec2(Frac,0.0)+gl_TexCoord[0].st).a;
  }

  y = 1.1643 * (y - 0.0625);
  u = u - 0.5;
  v = v - 0.5;

  float r = y + 1.5958 * v;
  float g = y - 0.39173 * u - 0.81290 * v;
  float b = y + 2.017 * u;

  gl_FragColor = vec4(r,g,b,1.0);
}