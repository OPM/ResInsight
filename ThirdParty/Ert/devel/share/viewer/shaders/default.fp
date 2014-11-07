void main() {
    gl_FragColor.rgb = gl_TexCoord[0].zyx;
    gl_FragColor.a = 1.0;
}