/*a Documentation
Ported from gjslib.math.quaternion python
    # +y to right, +x up, +z in front
    # roll + = roll clkwise inside looking forward
    # pitch + = nose up inside looking forward
    # yaw + = nose left inside looking forward
    # when given roll, pitch, yaw the order applies is roll(pitch(yaw())) - i.e. yaw is applied first
 */
/*a Includes
 */
#include <math.h>
#include "quaternion.h"

/*a Defines
 */
#define EPSILON (1E-20)
#define PI (3.141592838)

/*a Infix operator methods for doubles
 */
c_quaternion &c_quaternion::operator=(double real)
{
    quat.r = real;
    quat.i = quat.j = quat.k = 0.0;
    return *this;
}

c_quaternion &c_quaternion::operator+=(double real)
{
    quat.r += real;
    return *this;
}

c_quaternion &c_quaternion::operator-=(double real)
{
    quat.r -= real;
    return *this;
}

c_quaternion &c_quaternion::operator*=(double real)
{
    this->scale(real);
    return *this;
}

c_quaternion &c_quaternion::operator/=(double real)
{
    this->scale(1.0/real);
    return *this;
}

/*a Infix operator methods for c_quaternion's
 */
c_quaternion &c_quaternion::operator=(const c_quaternion &other)
{
    quat = other.quat;
    return *this;
}

c_quaternion &c_quaternion::operator+=(const c_quaternion &other)
{
    this->add_scaled(&other,1.0);
    return *this;
}

c_quaternion &c_quaternion::operator-=(const c_quaternion &other)
{
    this->add_scaled(&other,-1.0);
    return *this;
}

c_quaternion &c_quaternion::operator*=(const c_quaternion &other)
{
    this->multiply(&other);
    return *this;
}

c_quaternion &c_quaternion::operator/=(const c_quaternion &other)
{
    c_quaternion r;
    r = other;
    this->multiply(r.reciprocal());
    return *this;
}

/*a Constructors
 */
c_quaternion::c_quaternion(void)
{
    quat.r = 0.0;
    quat.i = 0.0;
    quat.j = 0.0;
    quat.k = 0.0;
}

c_quaternion::c_quaternion(const c_quaternion *other)
{
    quat = other->quat;
}

c_quaternion::c_quaternion(double r, double i, double j, double k)
{
    quat.r = r;
    quat.i = i;
    quat.j = j;
    quat.k = k;
}

c_quaternion *c_quaternion::copy(void)
{
    c_quaternion *quat;
    quat = new c_quaternion(this);
    return quat;
}

static void __repr__(char *buffer, int buf_size)
{
/*        if self.repr_fmt=="euler":
            result = ("quaternion(euler=("+self.fmt+","+self.fmt+","+self.fmt+"),degrees=True)") % self.to_euler(degrees=True)
            return result
        elif self.repr_fmt=="euler_mod":
            result = ("quaternion(euler=("+self.fmt+","+self.fmt+","+self.fmt+"),length="+self.fmt+",degrees=True)") % self.to_euler(degrees=True,include_modulus=True)
            return result
        result = ("quaternion({'r':"+self.fmt+", 'i':"+self.fmt+", 'j':"+self.fmt+", 'k':"+self.fmt+"})") % (self.quat["r"],
                                                                                       self.quat["i"],
                                                                                       self.quat["j"],
                                                                                       self.quat["k"] )
        return result
*/
}


void c_quaternion::get_rijk(double rijk[4])
{
    rijk[0] = quat.r;
    rijk[1] = quat.i;
    rijk[2] = quat.j;
    rijk[3] = quat.k;
}

c_quaternion *c_quaternion::from_euler(double roll, double pitch, double yaw, int degrees)
{
    double cr, cp, cy;
    double sr, sp, sy;
    double crcp, srsp;

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
    return this;
}

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
    #f to_euler
    def to_euler( self, include_modulus=False, degrees=False ):
        """
        Euler angles are roll, pitch and yaw.
        The rotations are performed in the order 
        """
        r = self.quat["r"]
        i = self.quat["i"]
        j = self.quat["j"]
        k = self.quat["k"]
        l = math.sqrt(r*r+i*i+j*j+k*k)
        if (l>1E-9):
            r=r/l
            i=i/l
            j=j/l
            k=k/l
            pass
        yaw   = math.atan2(2*(r*i+j*k), 1-2*(i*i+j*j))
        if 2*(r*j-i*k)<-1 or 2*(r*j-i*k)>1:
            pitch = math.asin( 1.0 )
            pass
        else:
            pitch = math.asin( 2*(r*j-i*k))
            pass
        roll  = math.atan2(2*(r*k+i*j), 1-2*(j*j+k*k))
        if degrees:
            roll  = 180.0/3.14159265 * roll
            pitch = 180.0/3.14159265 * pitch
            yaw   = 180.0/3.14159265 * yaw
            pass
        if include_modulus:
            return (roll, pitch, yaw, self.modulus())
        return (roll, pitch, yaw)
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

c_quaternion *c_quaternion::from_rotation(double angle, double axis[3], int degrees)
{
    if (degrees) {
        angle *= PI/180;
    }
    double s = sin(angle/2);
    double c = cos(angle/2);
    quat.r = c;
    quat.i = s*axis[0];
    quat.j = s*axis[1];
    quat.k = s*axis[2];
    return this;
}

double c_quaternion::as_rotation(double axis[3])
{
    double m=this->modulus();
    double angle = 2*acos(quat.r/m);

    double sm = m*sin(angle/2);
    axis[0] = quat.i/sm;
    axis[1] = quat.j/sm;
    axis[2] = quat.k/sm;
    return angle;
}

c_quaternion *c_quaternion::conjugate(void)
{
    quat.r = quat.r;
    quat.i = -quat.i;
    quat.j = -quat.j;
    quat.k = -quat.k;
    return this;
}

c_quaternion *c_quaternion::reciprocal(void)
{
    this->conjugate();
    this->scale(1.0/this->modulus_squared());
    return this;
}


c_quaternion *c_quaternion::add_scaled(const c_quaternion *other, double scale)
{
    quat.r += other->quat.r*scale;
    quat.i += other->quat.i*scale;
    quat.j += other->quat.j*scale;
    quat.k += other->quat.k*scale;
    return this;
}


double c_quaternion::modulus_squared(void)
{
    return (quat.r*quat.r + 
            quat.i*quat.i + 
            quat.j*quat.j + 
            quat.k*quat.k);
}

double c_quaternion::modulus(void)
{
    return sqrt( quat.r*quat.r + 
                 quat.i*quat.i + 
                 quat.j*quat.j + 
                 quat.k*quat.k);
}

c_quaternion *c_quaternion::scale(double scale)
{
    quat.r *= scale;
    quat.i *= scale;
    quat.j *= scale;
    quat.k *= scale;
    return this;
}


c_quaternion *c_quaternion::normalize(void)
{
    double l = this->modulus();
    if ((l>-EPSILON) && (l<EPSILON))
        return this;
    return this->scale(1.0/l);
}

c_quaternion *c_quaternion::multiply(const c_quaternion *other)
{
    double r1, i1, j1, k1;
    double r2, i2, j2, k2;
    r1 = this->quat.r;
    i1 = this->quat.i;
    j1 = this->quat.j;
    k1 = this->quat.k;
    r2 = other->quat.r;
    i2 = other->quat.i;
    j2 = other->quat.j;
    k2 = other->quat.k;

    this->quat.r = r1*r2 - i1*i2 - j1*j2 - k1*k2;
    this->quat.i = r1*i2 + i1*r2 + j1*k2 - k1*j2;
    this->quat.j = r1*j2 + j1*r2 + k1*i2 - i1*k2;
    this->quat.k = r1*k2 + k1*r2 + i1*j2 - j1*i2;
    return this;
}


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

/*a "Class methods"
 */
/*f quaternion_identity - return identity quarternion
 */
extern c_quaternion *
quaternion_identity(void)
{
    return new c_quaternion();
}

extern c_quaternion *
quaternion_pitch(double angle, int degrees)
{
    return (new c_quaternion())->from_euler(0,angle,0, degrees);
}

extern c_quaternion *
quaternion_yaw(double angle, int degrees)
{
    return (new c_quaternion())->from_euler(0,0,angle, degrees);
}

extern c_quaternion *
quaternion_roll(double angle, int degrees)
{
    return (new c_quaternion())->from_euler(angle,0,0, degrees);
}

extern c_quaternion *
quaternion_of_euler(double roll, double pitch, double yaw, int degrees)
{
    return (new c_quaternion())->from_euler(roll, pitch, yaw, degrees);
}

extern c_quaternion *
quaternion_from_rotation(double angle, double axis[3], int degrees)
{
    return (new c_quaternion())->from_rotation(angle, axis, degrees);
}