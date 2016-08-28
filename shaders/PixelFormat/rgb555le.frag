uniform sampler2D YTex;

void main()
{
  float r = texture2D(YTex, gl_TexCoord[0].xy).r;
  float g = texture2D(YTex, gl_TexCoord[0].xy).g;
  float b = texture2D(YTex, gl_TexCoord[0].xy).b;

  gl_FragColor = vec4(r, g, b, 1.0);
}