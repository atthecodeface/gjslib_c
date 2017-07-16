/** Copyright (C) 2016-2017,  Gavin J Stark.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @file          quaternion.cpp
 * @brief         Quaternions and methods thereon
 *
 */

/*a Documentation
Ported from gjslib.math.quaternion python
    # +y to right, +x up, +z in front
    # roll + = roll clkwise inside looking forward
    # pitch + = nose up inside looking forward
    # yaw + = nose left inside looking forward
    # when given roll, pitch, yaw the order applies is roll(pitch(yaw())) - i.e. yaw is applied first

Note that quaternion q = w + xi + yj + zk maps (0,0,1) to (2*(z*x-w*y), 2*(w*x+y*z), w*w-x*x-y*y+z*z)

Note that a quaternion can be represented also as q0 + q_  (i.e. real and 'ijk' parts)
A unit quatnernion has q0 = cos(theta/2), |q_| as sin(theta/2), and the axis of rotation is q_.

Now p * q = (p0 + p_) * (q0 + q_) = p0q0-p_.q_ + p0q_ + q0p_ + p_ x q_

Consider q . (0,0,1) . q* = (-qz + q0.(0,0,1) + q_ x (0,0,1)) . q*
 = (-qz + (0,0,q0) + q_ x (0,0,1) ) . (q0 - q_)
q_ = (qx,qy,qz)
q_ x (0,0,1) = (qy, -qx, 0)
q . (0,0,1) . q* = (-qz + (qy, -qx, q0)) . (q0 - q_)
                 = -q0.qz+q_.(qy, -qx, q0) + "p0q_+q0p_+p+xq_"
                 = (-q0.qz+qx.qy-qx.qy+qz.q0) + "p0q_+q0p_+p+xq_"
                 = "p0q_+q0p_+p+xq_"
                 = qz.q_ + q0.(qy, -qx, q0) - (qy, -qx, q0)x(qx,qy,qz)
(qy, -qx, q0)x(qx,qy,qz) = (-qx.qz-q0.qy, q0.qx-qy.qz, qy.qy+qx.qx)
Hence
q . (0,0,1) . q* = (qx.qz, qy.qz, qz.qz) + (q0.qy, -q0.qx, q0.q0) - (-qx.qz-q0.qy, q0.qx-qy.qz, qy.qy+qx.qx) 
                 = (2qx.qz + 2q0.qy,   2qy.qz - 2q0.qx, q0.q0-qx.qx-qy.qy+qz.qz)
                 = (2qx.qz + 2q0.qy,   2qy.qz - 2q0.qx, 2q0.q0 + 2qz.qz - 1)

Note that if we do the inverse rotation, we have q0,-qx,-qy,-qz and hence
q* . (0,0,1) . q = (2qx.qz - 2q0.qy,   2qy.qz + 2q0.qx, 2q0.q0 + 2qz.qz - 1)


for w in range(4):
    for x in range(4):
        for y in range(4):
           for z in range(4):
               q = quaternion(r=w,i=x,j=y,k=z)
               r = q.rotate_vector((0,0,1))
               print r, (2*(z*x-w*y), 2*(w*x+y*z), w*w-x*x-y*y+z*z)
               pass
           pass
        pass
    pass
pass

 */
/*a Includes
 */
#include <math.h>
#include <stdio.h>
#include "vector.h"
#include "quaternion.h"

/*a Defines
 */
#define EPSILON (1E-20)
#define PI (M_PI)

/*a Infix operator methods for Ts
 */
template <typename T>
c_quaternion<T> &c_quaternion<T>::operator=(T real)
{
    quat.r = real;
    quat.i = quat.j = quat.k = 0.0;
    return *this;
}

template <typename T>
c_quaternion<T> &c_quaternion<T>::operator+=(T real)
{
    quat.r += real;
    return *this;
}

template <typename T>
c_quaternion<T> &c_quaternion<T>::operator-=(T real)
{
    quat.r -= real;
    return *this;
}

template <typename T>
c_quaternion<T> &c_quaternion<T>::operator*=(T real)
{
    this->scale(real);
    return *this;
}

template <typename T>
c_quaternion<T> &c_quaternion<T>::operator/=(T real)
{
    this->scale(1.0/real);
    return *this;
}

/*a Infix operator methods for c_quaternion<T>'s
 */
template <typename T>
c_quaternion<T> &c_quaternion<T>::operator=(const c_quaternion<T> &other)
{
    quat = other.quat;
    return *this;
}

template <typename T>
c_quaternion<T> &c_quaternion<T>::operator+=(const c_quaternion<T> &other)
{
    return this->add_scaled(other,1.0);
}

template <typename T>
c_quaternion<T> &c_quaternion<T>::operator-=(const c_quaternion<T> &other)
{
    return this->add_scaled(other,-1.0);
}

template <typename T>
c_quaternion<T> &c_quaternion<T>::operator*=(const c_quaternion<T> &other)
{
    this->multiply(other);
    return *this;
}

template <typename T>
c_quaternion<T> &c_quaternion<T>::operator/=(const c_quaternion<T> &other)
{
    c_quaternion<T> r;
    r = other;
    this->multiply(r.reciprocal());
    return *this;
}

/*a Constructors
 */
/*f c_quaternion<T>::c_quaternion (void) - null
 */
template <typename T>
c_quaternion<T>::c_quaternion(void)
{
    quat.r = 0.0;
    quat.i = 0.0;
    quat.j = 0.0;
    quat.k = 0.0;
}

/*f c_quaternion<T>::c_quaternion(other) - copy from other
 */
template <typename T>
c_quaternion<T>::c_quaternion(const c_quaternion<T> &other)
{
    quat = other.quat;
}

/*f c_quaternion<T>::c_quaternion(r,j,k,k)
 */
template <typename T>
c_quaternion<T>::c_quaternion(T r, T i, T j, T k)
{
    quat.r = r;
    quat.i = i;
    quat.j = j;
    quat.k = k;
}

/*f c_quaternion<T>::c_quaternion(vector)
 */
template <typename T>
c_quaternion<T>::c_quaternion(const c_vector<T> &vector)
{
    quat.r = 0;
    quat.i = vector.value(0);
    quat.j = vector.value(1);
    quat.k = vector.value(2);
}

/*f c_quaternion<T>::copy
 */
template <typename T>
c_quaternion<T> *c_quaternion<T>::copy(void) const
{
    return new c_quaternion<T>(*this);
}

/*f c_quaternion<T>::__str__
 */
template <typename T>
char *
c_quaternion<T>::__str__(char *buffer, int buf_size) const
{
    snprintf(buffer, buf_size, "(%lf, %lf, %lf, %lf)",
             quat.r, quat.i, quat.j, quat.k );
    buffer[buf_size-1] = 0;
    return buffer;
}

/*f c_quaternion<T>::get_rijk
 */
template <typename T>
void c_quaternion<T>::get_rijk(T rijk[4]) const
{
    rijk[0] = quat.r;
    rijk[1] = quat.i;
    rijk[2] = quat.j;
    rijk[3] = quat.k;
}

/*f c_quaternion<T>::from_euler
 */
template <typename T>
c_quaternion<T> &c_quaternion<T>::from_euler(T roll, T pitch, T yaw, int degrees)
{
    T cr, cp, cy;
    T sr, sp, sy;
    T crcp, srsp;

    if (degrees) {
        roll  *= PI/180;
        yaw   *= PI/180;
        pitch *= PI/180;
    }

    cr = cos(roll/2);
    cp = cos(pitch/2);
    cy = cos(yaw/2);
    sr = sin(roll/2);
    sp = sin(pitch/2);
    sy = sin(yaw/2);

    crcp = cr * cp;
    srsp = sr * sp;
    quat.r = cy * crcp + sy * srsp;
    quat.i = sy * crcp - cy * srsp;
    quat.j = cy * cr * sp + sy * sr * cp;
    quat.k = cy * sr * cp - sy * cr * sp;
    return *this;
}

/*f c_quaternion<T>::lookat_aeronautic
 *
 * Find rotation that makes Z map to xyz axis, with X map to up (as far as possible)
 *
 * Remember the convention: +y to right, +x up, +z in front
 *  roll + = roll clkwise (around Z) inside looking forward
 * pitch + = nose up (around Y) inside looking forward
 *   yaw + = nose left (around X) inside looking forward
 * when given roll, pitch, yaw the order applies is roll(pitch(yaw())) - i.e. yaw is applied first
 *
 * So find yaw that gets axis on to Y=0 plane
 * Then pitch angle that gets axis rotated onto Y=0 rotated on to Z axis
 *
 * Then, find rotate the up vector by pitch and yaw
 * Then find the roll that gets the rotated Up to be parallel to the X axis
 *
 */
template <typename T>
c_quaternion<T> &c_quaternion<T>::lookat_aeronautic(const T xyz[3], const T up[3])
{
    T len_xyz = sqrt(xyz[0]*xyz[0] + xyz[1]*xyz[1] + xyz[2]*xyz[2]);
    T pitch =  asin(xyz[0] / len_xyz); // +ve as ccw around Y to Z
    T yaw   = -atan2(xyz[1], xyz[2]); // -ve as ccw around X to Z

    T cy = cos(yaw);
    T sy = sin(yaw);
    T cp = cos(pitch);
    T sp = sin(pitch);
    T roll = atan2( up[1]*cy    + up[2]*sy, // CCW around X -> Y coord (unchanged by CCW around Y)
                         up[1]*sy*sp - up[2]*cy*sp + up[0]*cp ); // CCW around X -> CCW around Y -> X coord
    return this->from_euler(-roll, -pitch, -yaw, 0).conjugate();
}

/*f c_quaternion<T>::lookat_aeronautic
 */
template <typename T>
c_quaternion<T> &c_quaternion<T>::lookat_aeronautic(const c_vector<T> &at, const c_vector<T> &up)
{
    T at_xyz[3], up_xyz[3];
    for (int i=0; i<3; i++) {
        at_xyz[i] = at.value(i);
        up_xyz[i] = up.value(i);
    }
    return lookat_aeronautic(at_xyz, up_xyz);
}

/*f c_quaternion<T>::lookat_graphics
 *
 * Find rotation that makes at map to -Z, with up mapping to Y (as far as possible)
 *
 * First map to +Z, up mapping to +X; rotate round Z by 90 degrees to
 * get up to +Y, rotate around Y by 180 degrees to get at to -Z
 *
 * Rotate around Z by 90 degrees is premultiply by quaternion of cos(45), 0, 0, sin(45)
 * 
 * Hence (r + rk) * (a + bi + cj + dk) => ra + rbi + rcj + rdk + ark + brj - cri -dr
 * = r(a-d) + r(b-c)i + r(b+c) + r(a+d)k
 *
 * Rotate around Y by 180 degrees is premultiply by quaternion of 0, 0, 1, 0
 *
 * Hence j * (a + bi + cj + dk) => aj - bk - c + di
 * = -c + di + aj - bk
 *
 * 0 r r 0 -> 0 0 1 0 -> -1 0 0 0
 * 
 */
template <typename T>
c_quaternion<T> &c_quaternion<T>::lookat_graphics(const T at[3], const T up[3])
{
    T r, i, j, k, rrt2;
    this->lookat_aeronautic(at, up);
    rrt2 = 1/sqrt(2);
    r = rrt2 * ( quat.r - quat.k );
    i = rrt2 * ( quat.i - quat.j );
    j = rrt2 * ( quat.j + quat.i );
    k = rrt2 * ( quat.k + quat.r );

    quat.r = -j;
    quat.i = k;
    quat.j = r;
    quat.k = -i;

    return *this;
}

/*f c_quaternion<T>::lookat_graphics
 */
template <typename T>
c_quaternion<T> &c_quaternion<T>::lookat_graphics(const c_vector<T> &at, const c_vector<T> &up)
{
    T at_xyz[3], up_xyz[3];
    for (int i=0; i<3; i++) {
        at_xyz[i] = at.value(i);
        up_xyz[i] = up.value(i);
    }
    return lookat_graphics(at_xyz, up_xyz);
}

/*f c_quaternion<T>::as_euler
 */
template <typename T>
void c_quaternion<T>::as_euler(T rpy[3]) const
{
    T l=modulus();
    T r=quat.r;
    T i=quat.i;
    T j=quat.j;
    T k=quat.k;
    T roll, pitch, yaw;
    if (l>1E-9) {
        r/=l; i/=l; j/=l; k/=l;
    }

    yaw = atan2(2*(r*i+j*k), 1-2*(i*i+j*j));
    if ((2*(r*j-i*k)<-1) || (2*(r*j-i*k)>1)) {
        pitch = asin(1.0);
    } else {
        pitch = asin(2*(r*j-i*k));
    }
    roll  = atan2(2*(r*k+i*j), 1-2*(j*j+k*k));
    rpy[0] = roll;
    rpy[1] = pitch;
    rpy[2] = yaw;
}

/*f More
 */
/*
    #f get_matrix_values
    def get_matrix_values( self ):
        if self.matrix is None: self.create_matrix()
        return self.matrix

    #f get_matrix
    def get_matrix( self, order=3 ):
        self.create_matrix()
        m = self.matrix
        if order==3:
            return matrix(data=(m[0][0], m[0][1], m[0][2],
                                     m[1][0], m[1][1], m[1][2],
                                     m[2][0], m[2][1], m[2][2],))
        if order==4:
            return matrix(data=(m[0][0], m[0][1], m[0][2], 0.0,
                                     m[1][0], m[1][1], m[1][2], 0.0,
                                     m[2][0], m[2][1], m[2][2], 0.0,
                                     0.0,0.0,0.0,1.0))
        raise Exception("Get matrix of unsupported order")

    #f create_matrix
    def create_matrix( self ):
        # From http://www.gamasutra.com/view/feature/131686/rotating_objects_using_quaternions.php?page=2
        # calculate coefficients
        l = self.modulus()

        x2 = self.quat["i"] + self.quat["i"]
        y2 = self.quat["j"] + self.quat["j"] 
        z2 = self.quat["k"] + self.quat["k"]
        xx = self.quat["i"] * x2
        xy = self.quat["i"] * y2
        xz = self.quat["i"] * z2
        yy = self.quat["j"] * y2
        yz = self.quat["j"] * z2
        zz = self.quat["k"] * z2
        wx = self.quat["r"] * x2
        wy = self.quat["r"] * y2
        wz = self.quat["r"] * z2
        m = [[0,0,0,0.],[0,0,0,0.],[0,0,0,0.],[0.,0.,0.,1.]]

        m[0][0] = l - (yy + zz)/l
        m[1][0] = (xy - wz)/l
        m[2][0] = (xz + wy)/l

        m[0][1] = (xy + wz)/l
        m[1][1] = l - (xx + zz)/l
        m[2][1] = (yz - wx)/l

        m[0][2] = (xz - wy)/l
        m[1][2] = (yz + wx)/l
        m[2][2] = l - (xx + yy)/l

        self.matrix = m
        pass
    #f from_matrix
    def from_matrix( self, matrix, epsilon=1E-6 ):
        """
        """
        d = matrix.determinant()
        if (d>-epsilon) and (d<epsilon):
            raise Exception("Singular matrix supplied")
        m = matrix.copy()
        if d<0: d=-d
        m.scale(1.0/math.pow(d,1/3.0))

        yaw   = math.atan2(m[1,2],m[2,2])
        roll  = math.atan2(m[0,1],m[0,0])
        if m[0,2]<-1 or m[0,2]>1:
            pitch=-math.asin(1)
        else:
            pitch = -math.asin(m[0,2])
        q0 = quaternion.of_euler(roll=roll, pitch=pitch, yaw=yaw, degrees=False)

        yaw   = math.atan2(m[2,1],m[2,2])
        roll  = math.atan2(m[1,0],m[0,0])
        if m[2,0]<-1 or m[2,0]>1:
            pitch=-math.asin(1)
        else:
            pitch = -math.asin(m[2,0])
        q1 = quaternion.of_euler(roll=roll, pitch=pitch, yaw=yaw, degrees=False)
        self.quat["r"] = (q0.quat["r"] + q1.quat["r"])/2.0
        self.quat["i"] = (q0.quat["i"] - q1.quat["i"])/2.0
        self.quat["j"] = (q0.quat["j"] - q1.quat["j"])/2.0
        self.quat["k"] = (q0.quat["k"] - q1.quat["k"])/2.0
        self.normalize()
        self.matrix = None
        return self
*/

/*f c_quaternion<T>::from_rotation
 */
template <typename T>
c_quaternion<T> &c_quaternion<T>::from_rotation(T angle, const T axis[3], int degrees)
{
    if (degrees) {
        angle *= PI/180;
    }
    T s = sin(angle/2);
    T c = cos(angle/2);
    quat.r = c;
    quat.i = s*axis[0];
    quat.j = s*axis[1];
    quat.k = s*axis[2];
    return *this;
}

/*f c_quaternion<T>::from_rotation
 */
template <typename T>
c_quaternion<T> &c_quaternion<T>::from_rotation(T cos_angle, T sin_angle, const T *axis, int axis_stride)
{
    T c, s;
    // cos(2x) = 2(cos(x)^2)-1 = cos(x)^2 - sin(x)^2
    // sin(2x) = 2sin(x)cos(x)
    // cos(x) = +-sqrt(1+cos(2x))/sqrt(2)
    // sin(x) = +-sin(2x)/sqrt(1+cos(2x))/sqrt(2)
    // chose +ve for cos(x) (-90<x<90), and sin(x) same segment as sin(2x)
    if (cos_angle>=1){
        quat.r = 1;
        quat.i = 0;
        quat.j = 0;
        quat.k = 0;
        return *this;
    }
    if (cos_angle<=-1){
        quat.r = 0; // rotate by 180 degrees around an axis
        quat.i = axis[0*axis_stride];
        quat.j = axis[1*axis_stride];
        quat.k = axis[2*axis_stride];
        return *this;
    }
    c = sqrt((1+cos_angle)/2);
    s = sqrt(1-c*c);
    if (sin_angle<0) {
        s = -s;
    }
    quat.r = c;
    quat.i = s*axis[0*axis_stride];
    quat.j = s*axis[1*axis_stride];
    quat.k = s*axis[2*axis_stride];
    return *this;
}

/*f c_quaternion<T>::from_rotation
 */
template <typename T>
c_quaternion<T> &c_quaternion<T>::from_rotation(T cos_angle, T sin_angle, const c_vector<T> &axis)
{
    return from_rotation(cos_angle, sin_angle, axis.coords(NULL), axis.stride());
}

/*f c_quaternion<T>::as_rotation
 */
template <typename T>
T c_quaternion<T>::as_rotation(T axis[3]) const
{
    T m=this->modulus();
    T angle = 2*acos(quat.r/m);

    T sm = m*sin(angle/2);
    if (fabs(sm)>EPSILON) {
        axis[0] = quat.i/sm;
        axis[1] = quat.j/sm;
        axis[2] = quat.k/sm;
    } else {
        axis[0] = 0;
        axis[1] = 0;
        axis[2] = 0;
    }
    return angle;
}

/*f c_quaternion<T>::as_rotation
 */
template <typename T>
T c_quaternion<T>::as_rotation(c_vector<T> &vector) const
{
    T m=this->modulus();
    T angle = 2*acos(quat.r/m);

    T sm = m*sin(angle/2);
    if (fabs(sm)>EPSILON) {
        vector.set(0, quat.i/sm);
        vector.set(1, quat.j/sm);
        vector.set(2, quat.k/sm);
    } else {
        vector.set(0, 0);
        vector.set(1, 0);
        vector.set(2, 0);
    }
    return angle;
}

/*f c_quaternion<T>::as_rotation
 */
template <typename T>
void c_quaternion<T>::as_rotation(c_vector<T> &vector, T *cos, T *sin) const
{
    T m=this->modulus();
    T cos_half = quat.r/m;
    T sin_half = sqrt(1-cos_half*cos_half);
    *cos = cos_half*cos_half - sin_half*sin_half;
    *sin = 2*sin_half*cos_half;

    T sm = m*sin_half;
    if (fabs(sm)>EPSILON) {
        vector.set(0, quat.i/sm);
        vector.set(1, quat.j/sm);
        vector.set(2, quat.k/sm);
    } else {
        vector.set(0, 0);
        vector.set(1, 0);
        vector.set(2, 0);
    }
}

/*f c_quaternion<T>::conjugate
 */
template <typename T>
c_quaternion<T> &c_quaternion<T>::conjugate(void)
{
    quat.r = quat.r;
    quat.i = -quat.i;
    quat.j = -quat.j;
    quat.k = -quat.k;
    return *this;
}

/*f c_quaternion<T>::reciprocal
 */
template <typename T>
c_quaternion<T> &c_quaternion<T>::reciprocal(void)
{
    this->conjugate();
    this->scale(1.0/this->modulus_squared());
    return *this;
}


/*f c_quaternion<T>::add_scaled
 */
template <typename T>
c_quaternion<T> &c_quaternion<T>::add_scaled(const c_quaternion<T> &other, T scale)
{
    quat.r += other.quat.r*scale;
    quat.i += other.quat.i*scale;
    quat.j += other.quat.j*scale;
    quat.k += other.quat.k*scale;
    return *this;
}

/*f c_quaternion<T>::modulus_squared
 */
template <typename T>
T c_quaternion<T>::modulus_squared(void) const
{
    return (quat.r*quat.r + 
            quat.i*quat.i + 
            quat.j*quat.j + 
            quat.k*quat.k);
}

/*f c_quaternion<T>::modulus
 */
template <typename T>
T c_quaternion<T>::modulus(void) const
{
    return sqrt( quat.r*quat.r + 
                 quat.i*quat.i + 
                 quat.j*quat.j + 
                 quat.k*quat.k);
}

/*f c_quaternion<T>::scale
 */
template <typename T>
c_quaternion<T> &c_quaternion<T>::scale(T scale)
{
    quat.r *= scale;
    quat.i *= scale;
    quat.j *= scale;
    quat.k *= scale;
    return *this;
}

/*f c_quaternion<T>::normalize
 */
template <typename T>
c_quaternion<T> &c_quaternion<T>::normalize(void)
{
    T l = this->modulus();
    if ((l>-EPSILON) && (l<EPSILON))
        return *this;
    return this->scale(1.0/l);
}

/*f c_quaternion<T>::multiply
 */
template <typename T>
c_quaternion<T> &c_quaternion<T>::multiply(const c_quaternion<T> &other, int premultiply)
{
    T r1, i1, j1, k1;
    T r2, i2, j2, k2;
    const c_quaternion<T> *a, *b;
    a=this; b=&other;
    if (premultiply) {
        a=&other; b=this;
    }

    r1 = a->quat.r;
    i1 = a->quat.i;
    j1 = a->quat.j;
    k1 = a->quat.k;

    r2 = b->quat.r;
    i2 = b->quat.i;
    j2 = b->quat.j;
    k2 = b->quat.k;

    this->quat.r = r1*r2 - i1*i2 - j1*j2 - k1*k2;
    this->quat.i = r1*i2 + i1*r2 + j1*k2 - k1*j2;
    this->quat.j = r1*j2 + j1*r2 + k1*i2 - i1*k2;
    this->quat.k = r1*k2 + k1*r2 + i1*j2 - j1*i2;
    return *this;
}

/*f c_quaternion<T>::rotate_vector
 */
template <typename T>
c_quaternion<T> *c_quaternion<T>::rotate_vector(const c_vector<T> &vector) const
{
    c_quaternion<T> *r = new c_quaternion<T>();
    *r = (*this) * c_quaternion<T>(vector) * this->copy()->conjugate();
    return r;
}

/*f c_quaternion<T>::rotate_vector
 */
template <typename T>
void c_quaternion<T>::rotate_vector(c_vector<T> *vector) const
{
    c_quaternion<T> r = c_quaternion<T>(*vector);
    c_quaternion<T> c = c_quaternion<T>(*this);
    r = (*this) * r * c.conjugate();
    vector->set(0,r.quat.i);
    vector->set(1,r.quat.j);
    vector->set(2,r.quat.k);
}

/*f c_quaternion<T>::angle_axis
 */
template <typename T>
c_quaternion<T> *c_quaternion<T>::angle_axis(const c_quaternion<T> &other, c_vector<T> &vector) const
{
    c_quaternion<T> *a, *b;
    c_quaternion<T> *r = new c_quaternion<T>();
    T cos_angle, sin_angle;
    //fprintf(stderr,"vector:%lf,%lf,%lf\n",vector.coords()[0],vector.coords()[1],vector.coords()[2]);
    a = this->rotate_vector(vector);
    //fprintf(stderr,"a:%lf,%lf,%lf,%lf\n",a.r(),a.i(),a.j(),a.k());
    b = other.rotate_vector(vector);
    //fprintf(stderr,"b:%lf,%lf,%lf,%lf\n",b.r(),b.i(),b.j(),b.k());
    c_vector<T> *axis = c_vector<T>(*a).angle_axis_to_v3(c_vector<T>(*b), &cos_angle, &sin_angle);
    //fprintf(stderr,"axis:%lf,%lf,%lf - %lf,%lf\n",axis.coords()[0],axis.coords()[1],axis.coords()[2],cos_angle,sin_angle);
    r->from_rotation(cos_angle, sin_angle, *axis);
    delete a;
    delete b;
    delete axis;
    return r;
}

/*f c_quaternion<T>::distance_to
 */
template <typename T>
T c_quaternion<T>::distance_to(const c_quaternion<T> &other) const
{
    c_quaternion<T> q;
    q = (*this / other);
    return 1-((q*q).r());
}

/*a Others
 */

/*
    #f rotation_multiply
    def rotation_multiply( self, other ):
        A = (self.quat["r"] + self.quat["i"])*(other.quat["r"] + other.quat["i"])
        B = (self.quat["k"] - self.quat["j"])*(other.quat["j"] - other.quat["k"])
        C = (self.quat["r"] - self.quat["i"])*(other.quat["j"] + other.quat["k"]) 
        D = (self.quat["j"] + self.quat["k"])*(other.quat["r"] - other.quat["i"])
        E = (self.quat["i"] + self.quat["k"])*(other.quat["i"] + other.quat["j"])
        F = (self.quat["i"] - self.quat["k"])*(other.quat["i"] - other.quat["j"])
        G = (self.quat["r"] + self.quat["j"])*(other.quat["r"] - other.quat["k"])
        H = (self.quat["r"] - self.quat["j"])*(other.quat["r"] + other.quat["k"])
        r = B + (-E - F + G + H) /2
        i = A - (E + F + G + H)/2 
        j = C + (E - F + G - H)/2 
        k = D + (E - F - G + H)/2
        return quaternion( quat={"r":r, "i":i, "j":j, "k":k } )
    #f interpolate
    def interpolate( self, other, t ):
        cosom = ( self.quat["i"] * other.quat["i"] +
                  self.quat["j"] * other.quat["j"] +
                  self.quat["k"] * other.quat["k"] +
                  self.quat["r"] * other.quat["r"] )
        abs_cosom = cosom
        sgn_cosom = 1
        if (cosom <0.0): 
            abs_cosom = -cosom
            sgn_cosom = -1
            pass

        # calculate coefficients
        if ( (1.0-abs_cosom) > epsilon ):
            #  standard case (slerp)
            omega = math.acos(abs_cosom);
            sinom = math.sin(omega);
            scale0 = math.sin((1.0 - t) * omega) / sinom;
            scale1 = math.sin(t * omega) / sinom;
            pass
        else:
            # "from" and "to" quaternions are very close 
            #  ... so we can do a linear interpolation
            scale0 = 1.0 - t;
            scale1 = t;
            pass

        # calculate final values
        i = scale0 * self.quat["i"] + scale1 * sgn_cosom * other.quat["i"]
        j = scale0 * self.quat["j"] + scale1 * sgn_cosom * other.quat["j"]
        k = scale0 * self.quat["k"] + scale1 * sgn_cosom * other.quat["k"]
        r = scale0 * self.quat["r"] + scale1 * sgn_cosom * other.quat["r"]
        return quaternion( quat={"r":r, "i":i, "j":j, "k":k } )

                }
}
*/


/*a Explicit instantiations of the template
 */
template class c_quaternion<double>;
template class c_quaternion<float>;
