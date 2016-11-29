#version 120
uniform sampler3D grid;
uniform sampler3D grid_data;
uniform sampler1D color_scale;

uniform bool hide_inactive_cells;
uniform bool lighting;
uniform bool region_scaling;
uniform float data_range;

// From python code: self.__shader.setUniformi("grid_size", texture.getWidth(), texture.getHeight(), texture.getDepth())


varying vec3 normal;


void main() {
    vec4 cell = texture3D(grid, gl_TexCoord[0].xyz);

    if(hide_inactive_cells && cell.w < 0.5) {
        discard;
    }

    float color_pos = texture3D(grid_data, gl_TexCoord[0].xyz).a;

    if (color_pos < 0.001) {
        discard;
    }

    if(region_scaling) {
        // data_range + 1 because [0-9] count = 10
        color_pos = color_pos * (data_range + 1) / 16.0;
    }

    vec3 color = texture1D(color_scale, color_pos).rgb;

    //color = gl_TexCoord[0].yxz;

    if(lighting) {
        float d = dot(normalize(normal), normalize(vec3(0.0, 0.0, 0.5)));
        //d = clamp(d, 0.0, 1.0);
        color = color + ((0.5 * color * d) - 0.25);
    }



    gl_FragColor.rgb = color;
    gl_FragColor.a = 1.0;
}
