// The fragment shader gets an (x,y) output texture frame buffer pixel, since it is invoked for each such pixel
//
// Requires
// -DNUM_CIRCLE_STEPS=8
// -DDFT_CIRCLE_RADIUS=6

out vec4 color;
in vec2 uv_to_frag;
uniform sampler2D texture_src;

void main()
{
     float[NUM_CIRCLE_STEPS*4] colors;
     vec2[8] dft;
     float[8] dft_power;
     float[8] dft_angle;

     texture_circle(texture_src, uv_to_frag, DFT_CIRCLE_RADIUS, circle_offsets_8, colors);
     dft32_8(colors, dft);
     dft8_power_angle(dft, dft_power, dft_angle);

     // x comes as raw_img[0]
     color.x = uintBitsToFloat( pack_power_angle(dft_power[0], 0) );
     color.y = uintBitsToFloat( pack_power_angle(dft_power[1], dft_angle[1]) );
     color.z = uintBitsToFloat( pack_power_angle(dft_power[2], dft_angle[2]) );
     color.w = uintBitsToFloat( pack_power_angle(dft_power[3], dft_angle[3]) );

     //color.x = float(int((pack_power_angle(dft_power[3], dft_angle[3]))) &0xff)/255.0;

     //float a,b;
     //unpack_power_angle(pack_power_angle(dft_power[1], dft_angle[1]),a,b);
     //color.x = b/8;
}
