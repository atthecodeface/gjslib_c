#define STEP (1.0/1024)

#ifndef CIRCLE_COMPONENT
#define CIRCLE_COMPONENT r
#endif

#ifndef NUM_WEIGHTS
#define NUM_WEIGHTS 9
#endif

#ifndef NUM_OFFSETS
#define NUM_OFFSETS 9
#endif

#ifndef NUM_CIRCLE_STEPS
#define NUM_CIRCLE_STEPS 8
#endif

#ifndef CIRCLE_RADIUS
#define CIRCLE_RADIUS 1
#endif

#ifndef CORR_CIRCLE_RADIUS
#define CORR_CIRCLE_RADIUS 8
#endif

#ifndef INTENSITY_XSCALE
#define INTENSITY_XSCALE 1.0
#define INTENSITY_YSCALE 1.0
#define INTENSITY_XOFS   0.0
#define INTENSITY_YOFS   0.0
#endif

const vec2 gauss_offset_weights9[9] = vec2[](
vec2(-4*STEP, 0.000229f),
vec2(-3*STEP, 0.005977f),
vec2(-2*STEP, 0.060598f),
vec2(-1*STEP, 0.241732f),
vec2( 0*STEP, 0.382928f),
vec2( 1*STEP, 0.241732f),
vec2( 2*STEP, 0.060598f),
vec2( 3*STEP, 0.005977f),
vec2( 4*STEP, 0.000229f)
);

const vec3 sobel_weights[9] = vec3[](
       vec3(-1*STEP, -1*STEP, -1.0f),
       vec3(-1*STEP,  0*STEP, -1.0f),
       vec3(-1*STEP,  1*STEP, -1.0f),
       vec3( 0*STEP, -1*STEP, -1.0f),
       vec3( 0*STEP,  0*STEP,  8.0f),
       vec3( 0*STEP,  1*STEP, -1.0f),
       vec3( 1*STEP, -1*STEP, -1.0f),
       vec3( 1*STEP,  0*STEP, -1.0f),
       vec3( 1*STEP,  1*STEP, -1.0f)
);
     
const vec2 circle_offsets_8[8] = vec2[](
    STEP*CIRCLE_RADIUS*vec2(1.0, 0.0),
    STEP*CIRCLE_RADIUS*vec2(0.980785280447, 0.195090321796),
    STEP*CIRCLE_RADIUS*vec2(0.923879532683, 0.382683431951),
    STEP*CIRCLE_RADIUS*vec2(0.831469612676, 0.55557023246),
    STEP*CIRCLE_RADIUS*vec2(0.707106781821, 0.707106780552),
    STEP*CIRCLE_RADIUS*vec2(0.555570233952, 0.831469611679),
    STEP*CIRCLE_RADIUS*vec2(0.382683433609, 0.923879531996),
    STEP*CIRCLE_RADIUS*vec2(0.195090323556, 0.980785280097)
    );

const vec2 offsets_2d_25[25] = vec2[](
    vec2(-2*STEP, -2*STEP),
    vec2(-2*STEP, -1*STEP),
    vec2(-2*STEP,    STEP),
    vec2(-2*STEP,  1*STEP),
    vec2(-2*STEP,  2*STEP),

    vec2(-1*STEP, -2*STEP),
    vec2(-1*STEP, -1*STEP),
    vec2(-1*STEP,    STEP),
    vec2(-1*STEP,  1*STEP),
    vec2(-1*STEP,  2*STEP),

    vec2( 0*STEP, -2*STEP),
    vec2( 0*STEP, -1*STEP),
    vec2( 0*STEP,    STEP),
    vec2( 0*STEP,  1*STEP),
    vec2( 0*STEP,  2*STEP),

    vec2( 1*STEP, -2*STEP),
    vec2( 1*STEP, -1*STEP),
    vec2( 1*STEP,    STEP),
    vec2( 1*STEP,  1*STEP),
    vec2( 1*STEP,  2*STEP),

    vec2( 2*STEP, -2*STEP),
    vec2( 2*STEP, -1*STEP),
    vec2( 2*STEP,    STEP),
    vec2( 2*STEP,  1*STEP),
    vec2( 2*STEP,  2*STEP)
);

const vec2 offsets_2d_81[81] = vec2[](
       vec2(-4*STEP, -4*STEP),
       vec2(-4*STEP, -3*STEP),
       vec2(-4*STEP, -2*STEP),
       vec2(-4*STEP, -1*STEP),
       vec2(-4*STEP,  0*STEP),
       vec2(-4*STEP,  1*STEP),
       vec2(-4*STEP,  2*STEP),
       vec2(-4*STEP,  3*STEP),
       vec2(-4*STEP,  4*STEP),

       vec2(-3*STEP, -4*STEP),
       vec2(-3*STEP, -3*STEP),
       vec2(-3*STEP, -2*STEP),
       vec2(-3*STEP, -1*STEP),
       vec2(-3*STEP,  0*STEP),
       vec2(-3*STEP,  1*STEP),
       vec2(-3*STEP,  2*STEP),
       vec2(-3*STEP,  3*STEP),
       vec2(-3*STEP,  4*STEP),

       vec2(-2*STEP, -4*STEP),
       vec2(-2*STEP, -3*STEP),
       vec2(-2*STEP, -2*STEP),
       vec2(-2*STEP, -1*STEP),
       vec2(-2*STEP,  0*STEP),
       vec2(-2*STEP,  1*STEP),
       vec2(-2*STEP,  2*STEP),
       vec2(-2*STEP,  3*STEP),
       vec2(-2*STEP,  4*STEP),

       vec2(-1*STEP, -4*STEP),
       vec2(-1*STEP, -3*STEP),
       vec2(-1*STEP, -2*STEP),
       vec2(-1*STEP, -1*STEP),
       vec2(-1*STEP,  0*STEP),
       vec2(-1*STEP,  1*STEP),
       vec2(-1*STEP,  2*STEP),
       vec2(-1*STEP,  3*STEP),
       vec2(-1*STEP,  4*STEP),

       vec2( 0*STEP, -4*STEP),
       vec2( 0*STEP, -3*STEP),
       vec2( 0*STEP, -2*STEP),
       vec2( 0*STEP, -1*STEP),
       vec2( 0*STEP,  0*STEP),
       vec2( 0*STEP,  1*STEP),
       vec2( 0*STEP,  2*STEP),
       vec2( 0*STEP,  3*STEP),
       vec2( 0*STEP,  4*STEP),

       vec2( 1*STEP, -4*STEP),
       vec2( 1*STEP, -3*STEP),
       vec2( 1*STEP, -2*STEP),
       vec2( 1*STEP, -1*STEP),
       vec2( 1*STEP,  0*STEP),
       vec2( 1*STEP,  1*STEP),
       vec2( 1*STEP,  2*STEP),
       vec2( 1*STEP,  3*STEP),
       vec2( 1*STEP,  4*STEP),

       vec2( 2*STEP, -4*STEP),
       vec2( 2*STEP, -3*STEP),
       vec2( 2*STEP, -2*STEP),
       vec2( 2*STEP, -1*STEP),
       vec2( 2*STEP,  0*STEP),
       vec2( 2*STEP,  1*STEP),
       vec2( 2*STEP,  2*STEP),
       vec2( 2*STEP,  3*STEP),
       vec2( 2*STEP,  4*STEP),

       vec2( 3*STEP, -4*STEP),
       vec2( 3*STEP, -3*STEP),
       vec2( 3*STEP, -2*STEP),
       vec2( 3*STEP, -1*STEP),
       vec2( 3*STEP,  0*STEP),
       vec2( 3*STEP,  1*STEP),
       vec2( 3*STEP,  2*STEP),
       vec2( 3*STEP,  3*STEP),
       vec2( 3*STEP,  4*STEP),

       vec2( 4*STEP, -4*STEP),
       vec2( 4*STEP, -3*STEP),
       vec2( 4*STEP, -2*STEP),
       vec2( 4*STEP, -1*STEP),
       vec2( 4*STEP,  0*STEP),
       vec2( 4*STEP,  1*STEP),
       vec2( 4*STEP,  2*STEP),
       vec2( 4*STEP,  3*STEP),
       vec2( 4*STEP,  4*STEP)
);

// from wikipedia, ITU-R YUV from RGB
vec3 yuv_at(in sampler2D texture_in, in float x, in float y)
{
    vec2 texture_xy;
    texture_xy = vec2( (x*INTENSITY_XSCALE)+INTENSITY_XOFS, (y*INTENSITY_YSCALE)+INTENSITY_YOFS);
    vec3 t=texture(texture_in, texture_xy).rgb;
    vec3 yuv;
    yuv.r = 0.299*t.r + 0.587*t.g + 0.114*t.b;
    yuv.g = (-0.169*t.r -0.331*t.g + 0.499*t.b)+0.5;
    yuv.b = ( 0.499*t.r -0.418*t.g - 0.0813*t.b)+0.5;
    return yuv;
}

void texture_circle(in sampler2D texture_in, in vec2 centre_xy, in float radius, in vec2[NUM_CIRCLE_STEPS] circle_offsets, out float[NUM_CIRCLE_STEPS*4] colors)
{
    for (int i=0; i<NUM_CIRCLE_STEPS; i++) {
        colors[NUM_CIRCLE_STEPS*0+i] = texture(texture_in, centre_xy+radius*circle_offsets[i]).CIRCLE_COMPONENT;
        colors[NUM_CIRCLE_STEPS*1+i] = texture(texture_in, vec2(centre_xy.x-radius*circle_offsets[i].y,
                                                                centre_xy.y+radius*circle_offsets[i].x)
                                                   ).CIRCLE_COMPONENT;;
        colors[NUM_CIRCLE_STEPS*2+i] = texture(texture_in, centre_xy-radius*circle_offsets[i]).CIRCLE_COMPONENT;;
        colors[NUM_CIRCLE_STEPS*3+i] = texture(texture_in, vec2(centre_xy.x+radius*circle_offsets[i].y,
                                                                centre_xy.y-radius*circle_offsets[i].x)
                                                   ).CIRCLE_COMPONENT;;
    }
}

void gauss(in sampler2D texture_in, in vec2 xy, in bool x_not_y, in vec2[NUM_WEIGHTS] offset_weights, out vec4[NUM_WEIGHTS] colors)
{
    float mx, my;
    mx = 0.0; my=1.0;
    if (x_not_y) {
        mx = 1.0; my=0.0;
    }
    for (int i=0; i<NUM_WEIGHTS; i++) {
        colors[i] = offset_weights[i].y * texture( texture_in,
                                                   vec2(xy.x+offset_weights[i].x*mx, xy.y+offset_weights[i].x*my));
                                                   }
}

vec2 complex_mult(vec2 a, vec2 b)
{
    return vec2(a.r*b.r-a.g*b.g, a.r*b.g+a.g*b.r);
}

float flt_angle(in vec2 cs)
{
    // Octants   0   1   2   3   4   5   6   7
    //     cos   +>  +<  -<  ->  ->  -<  +<  +>
    //     sin   +<  +>  +>  +<  -<  ->  ->  -<
    // (ac>as)?(as/ac):(2.0-ac/as)
    //           0-1 1-2 2-1 1-0 0-1 1-2 2-1 1-0 
    // ac<0?4-:  0-1 1-2 2-3 3-4 4-3 3-2 2-1 1-0
    // as<0?8-:  0-1 1-2 2-3 3-4 4-5 5-6 6-7 7-8
    float ac, as;
    float angle;
    ac = abs(cs.x);
    as = abs(cs.y);
    angle = (ac>as)?(as/ac):(2.0-ac/as);
    angle = (cs.x<0)?4-angle:angle;
    angle = (cs.y<0)?8-angle:angle;
    return angle;
}

vec2 flt_angle_cs(in float a)
{
    // fract(angle) is s/c = tan
    // tan^2 = s^2/c^2 = 1/c^2-1
    // c^2 = 1/(1+t^2)
    // Octants   0   1   2   3   4   5   6   7
    //     cos  +ac  +<  -<  ->  ->  -<  +<  +>
    //     sin  +as  +>  +>  +<  -<  ->  ->  -<
    // (ac>as)?(as/ac):(2.0-ac/as)
    //           0-1 1-2 2-1 1-0 0-1 1-2 2-1 1-0 
    // ac<0?4-:  0-1 1-2 2-3 3-4 4-3 3-2 2-1 1-0
    // as<0?8-:  0-1 1-2 2-3 3-4 4-5 5-6 6-7 7-8
    float ac, as;
    float a_i, a_f;
    float ac2;
    int octant;
    vec2 cs;

    a_i = floor(a);
    octant = int(a_i);
    a_f = a - a_i;
    a_f = ((octant&1)!=0) ? (1-a_f) : a_f;
    ac2 = 1/(1+a_f*a_f);
    as = sqrt(1-ac2);
    ac = sqrt(ac2);

    if ((octant&4)!=0) { as=-as; ac=-ac; }
    cs = vec2(ac, as);
    if ((octant&1)!=0) { cs = vec2(cs.y,cs.x); }
    if ((octant&2)!=0) { cs = vec2(-cs.y,cs.x); }

    return cs;
}

float flt_angle_diff(in float a, in float b)
{
    a = a-b;
    a = (a<0)?(a+8):a;
    return a;
}

float flt_angle_diff_scaled(in float a, in float b, in float scale)
{
    a = (a-b)*scale;
    if (a<0) a+=32.0;
    if (a>=32.0) a-=32.0;
    if (a>=24.0) a-=24.0;
    if (a>=16.0) a-=16.0;
    if (a>=8.0) a-=8.0;
    return a;
}

float flt_angle_diff_abs(in float a, in float b)
{
    // difference is 0 <= a-b <=4
    // can do abs(a-b)
    // but if abs(a-b)>4, then use 8.0-abs(a-b) as a=7.99, b=0.01 should give 0.02
    float r;
    r = abs(a-b);
    r = (r>=4)?(8-r):r;
    return r;
}

float flt_angle_diff_scale_abs(in float a, in float scale, in float b)
{
    // difference is 0 <= a-b <=4
    // can do abs(a-b)
    // but if abs(a-b)>4, then use 8.0-abs(a-b) as a=7.99, b=0.01 should give 0.02
    float r;
    r = abs(a*scale-b);
    r = (r>=32)?(r-32):r;
    r = (r>=16)?(r-16):r;
    r = (r>=8)?(r-8):r;
    r = (r>=4)?(8-r):r;
    return r;
}

#define PI 3.1415926535897932384626433832795
// fd[k] = Sum(n=0..N-1)(td[n]*e^(2i.pi.k.n/N))
void dft32(in float[32] time_domain, out vec2[32]freq_domain)
{
    vec2[32]   fd_1;
    vec2[32]   fd_2;
    vec2[32]   fd_3;
    vec2[32]   fd_4;
    vec2 wk;
    // handle fft.py:fft2 when j=1 and 2 by simple mapping (dft4's, effectively)
    for (int i=0; i<8; i++) {
        int bri; // 0 4 2 6 1 5 3 7
        bri = i+(((i&1)!=0)?3:0) - (((i&4)!=0)?3:0);
        fd_1[4*i+0]  = vec2(time_domain[bri+0]+time_domain[bri+8]+time_domain[bri+16]+time_domain[bri+24],0.0);
        fd_1[4*i+1]  = vec2(time_domain[bri+0]-time_domain[bri+16],-time_domain[bri+8]+time_domain[bri+24]);
        fd_1[4*i+2]  = vec2(time_domain[bri+0]-time_domain[bri+8]+time_domain[bri+16]-time_domain[bri+24],0.0);
        fd_1[4*i+3]  = vec2(time_domain[bri+0]-time_domain[bri+16],+time_domain[bri+8]-time_domain[bri+24]);
    }
    // handle fft.py:fft2 when j=4
    wk = vec2(1.0,0.0);
    for (int k=0; k<4; k++) {
        const vec2 wm = vec2(cos(-2*PI/8.0),sin(-2*PI/8.0));
        for (int i=0; i<4; i++) {
            vec2 fdm;
            fdm = complex_mult(wk,fd_1[4+k+i*8]);
            fd_2[  k+i*8] = fd_1[k+i*8]+fdm;
            fd_2[4+k+i*8] = fd_1[k+i*8]-fdm;
        }
        wk = complex_mult(wk,wm);
    }
    // handle fft.py:fft2 when j=8
    wk = vec2(1.0,0.0);
    for (int k=0; k<8; k++) {
        const vec2 wm = vec2(cos(-2*PI/16.0),sin(-2*PI/16.0));
        for (int i=0; i<2; i++) {
            vec2 fdm;
            fdm = complex_mult(wk, fd_2[8+k+i*16]);
            fd_3[  k+i*16] = fd_2[k+i*16]+fdm;
            fd_3[8+k+i*16] = fd_2[k+i*16]-fdm;
        }
        wk = complex_mult(wk,wm);
    }
    // handle fft.py:fft2 when j=16
    wk = vec2(1.0,0.0);
    for (int k=0; k<16; k++) {
        const vec2 wm = vec2(cos(-2*PI/32.0),sin(-2*PI/32.0));
        vec2 fdm;
        fdm = complex_mult(wk, fd_3[16+k]);
        fd_4[k]    = fd_3[k]+fdm;
        fd_4[k+16] = fd_3[k]-fdm;
        wk = complex_mult(wk,wm);
    }
    for (int j=0; j<32; j++) {
        freq_domain[j] = fd_4[j];
        //freq_domain[j] = vec2(-fd_4[j].r,-fd_4[j].g);
    }
    freq_domain[0] = vec2(0.0,0.0);
}

void dft32_8(in float[32] time_domain, out vec2[8]freq_domain)
{
    vec2[32]   fd_1;
    vec2[32]   fd_2;
    vec2[32]   fd_3;
    vec2[32]   fd_4;
    vec2 wk;
    // handle fft.py:fft2 when j=1 and 2 by simple mapping (dft4's, effectively)
    for (int i=0; i<8; i++) {
        int bri; // 0 4 2 6 1 5 3 7
        bri = i+(((i&1)!=0)?3:0) - (((i&4)!=0)?3:0);
        fd_1[4*i+0]  = vec2(time_domain[bri+0]+time_domain[bri+8]+time_domain[bri+16]+time_domain[bri+24],0.0);
        fd_1[4*i+1]  = vec2(time_domain[bri+0]-time_domain[bri+16],-time_domain[bri+8]+time_domain[bri+24]);
        fd_1[4*i+2]  = vec2(time_domain[bri+0]-time_domain[bri+8]+time_domain[bri+16]-time_domain[bri+24],0.0);
        fd_1[4*i+3]  = vec2(time_domain[bri+0]-time_domain[bri+16],+time_domain[bri+8]-time_domain[bri+24]);
    }
    // handle fft.py:fft2 when j=4
    wk = vec2(1.0,0.0);
    for (int k=0; k<4; k++) {
        const vec2 wm = vec2(cos(-2*PI/8.0),sin(-2*PI/8.0));
        for (int i=0; i<4; i++) {
            vec2 fdm;
            fdm = complex_mult(wk,fd_1[4+k+i*8]);
            fd_2[  k+i*8] = fd_1[k+i*8]+fdm;
            fd_2[4+k+i*8] = fd_1[k+i*8]-fdm;
        }
        wk = complex_mult(wk,wm);
    }
    // handle fft.py:fft2 when j=8
    wk = vec2(1.0,0.0);
    for (int k=0; k<8; k++) {
        const vec2 wm = vec2(cos(-2*PI/16.0),sin(-2*PI/16.0));
        for (int i=0; i<2; i++) {
            vec2 fdm;
            fdm = complex_mult(wk, fd_2[8+k+i*16]);
            fd_3[  k+i*16] = fd_2[k+i*16]+fdm;
            fd_3[8+k+i*16] = fd_2[k+i*16]-fdm;
        }
        wk = complex_mult(wk,wm);
    }
    // handle fft.py:fft2 when j=16
    wk = vec2(1.0,0.0);
    for (int k=0; k<8; k++) {
        const vec2 wm = vec2(cos(-2*PI/32.0),sin(-2*PI/32.0));
        vec2 fdm;
        fdm = complex_mult(wk, fd_3[16+k]);
        fd_4[k]    = fd_3[k]+fdm;
        wk = complex_mult(wk,wm);
    }
    for (int j=0; j<8; j++) {
        freq_domain[j] = fd_4[j];
    }
    freq_domain[0] = fd_4[0]/32.0;
}

void dft32_normalize(in vec2[32] dft, out float[32] dft_power, out vec2[16]dft_normalized)
{
    float[32] dft_power_raw;
    float max, sqrt_max;
    max = 1.0;
    for (int i=1; i<8; i++) {
        dft_power_raw[i] = dft[i].x*dft[i].x+dft[i].y*dft[i].y;
        if (dft_power_raw[i]>max) {
            max=dft_power_raw[i];
        }
    }
    dft_power_raw[0] = 0;
    max = 0.1;
    max = 32.0;
    sqrt_max = sqrt(max);
    for (int i=0; i<32; i++) {
        dft_power[i] = dft_power_raw[i] / max;
        //dft_normalized[i] = dft[i] / sqrt_max;
        dft_normalized[i] = vec2(dft[i].x / sqrt_max, dft[i].y / sqrt_max);
    }
}

void dft32_normalize_8(in vec2[8] dft, out float[8] dft_power, out vec2[8]dft_normalized)
{
    float[8] dft_power_raw;
    float max, sqrt_max;
    max = 1.0;
    for (int i=1; i<8; i++) {
        dft_power_raw[i] = dft[i].x*dft[i].x+dft[i].y*dft[i].y;
        if (dft_power_raw[i]>max) {
            max=dft_power_raw[i];
        }
    }
    dft_power_raw[0] = 0;
    max = 8.0;
    max = 32.0;
    sqrt_max = sqrt(max);
    for (int i=0; i<8; i++) {
        dft_power[i] = dft_power_raw[i] / max;
        dft_normalized[i] = vec2(dft[i].x / sqrt_max, dft[i].y / sqrt_max);
    }
}

void dft8_power_angle(in vec2[8] dft, out float[8] dft_power, out float[8]dft_angle)
{
    for (int i=0; i<8; i++) {
        dft_power[i] = sqrt(dft[i].x*dft[i].x+dft[i].y*dft[i].y);
        dft_angle[i] = flt_angle(dft[i]);
    }
}

uint pack_power_angle(float power, float angle)
{
    // power is 0 -> 1
    // angle is 0 -> 8
    // output is 24 bits power, 8 bits angle, packed as 32 bits
    uint p, a;
    p = uint(power*4096*4096);
    a = uint(angle*(256/8));
    if (power>=1.0) { p=uint((1<<24)-1); }
    return (p<<8)|(a&uint(0xff));
}

void unpack_power_angle(uint pa, out float power, out float angle)
{
    uint p, a;
    a = pa&uint(0xff);
    p = pa>>8;
    power = (float(p))/(4096.0*4096.0);
    angle = float(a)/32.0;
}

float power_diff(float p0, float p1)
{
    // make pu,pl = max(p0,p1),min(p0,p1)
    // Then 0<pl/pu<1, and indicates the match
    // And 0.5 < (1+pu)/2  < 1.0
    // hence 0 < (1+pu)/2 * pl/pu <1,
    // and pu=1.0 => result = pl
    // pu=0.1, pl=0.1  => result = 0.55
    // pu=0.1, pl=0.02 => result = 0.11
    // pu=0.02, pl=0.01 => result = 0.11
    float pl, pu;
    pl = p0; pu = p1;
    if (p1<p0) { pl=p1; pu=p0; }
    pu = (pu==0)?1:pu;
    return (1+pu)/2*pl/pu;
    return (1+1*sqrt(pu))/2*pl/pu;
    return (1+20*sqrt(pu))/21*pl/pu;
}

float dft_rotation_error(in vec2 X_a1,in vec2 X_a2,in vec2 X_a3, in vec2 X_b1, in vec2 X_b2, in vec2 X_b3)
{
    vec2 Xa1_Xa1, Xb1_Xb1;
    vec2 Xa1_Xa1_Xb2, Xb1_Xb1_Xa2, X12_err;
    vec2 Xa1_Xa1_Xa1, Xb1_Xb1_Xb1;
    vec2 Xa1_Xa1_Xa1_Xb3, Xb1_Xb1_Xb1_Xa3, X13_err;
    float diff;
    float Xa1_m, Xb1_m;
    diff = 0.0;
    Xa1_Xa1 = complex_mult(X_a1,X_a1);
    Xb1_Xb1 = complex_mult(X_b1,X_b1);
//#if 0
    Xa1_Xa1_Xb2 = complex_mult(Xa1_Xa1,X_b2);
    Xb1_Xb1_Xa2 = complex_mult(Xb1_Xb1,X_a2);
    X12_err = Xa1_Xa1_Xb2 - Xb1_Xb1_Xa2;
    diff += (X12_err.x*X12_err.x + X12_err.y*X12_err.y);
//#else
    Xa1_Xa1_Xa1 = complex_mult(Xa1_Xa1,X_a1);
    Xb1_Xb1_Xb1 = complex_mult(Xb1_Xb1,X_b1);
    Xa1_Xa1_Xa1_Xb3 = complex_mult(Xa1_Xa1_Xa1,X_b3);
    Xb1_Xb1_Xb1_Xa3 = complex_mult(Xb1_Xb1_Xb1,X_a3);
    X13_err = Xa1_Xa1_Xa1_Xb3 - Xb1_Xb1_Xb1_Xa3;
    diff += (X13_err.x*X13_err.x + X13_err.y*X13_err.y);
//#endif
    return diff;
}


