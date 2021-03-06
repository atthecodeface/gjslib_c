#!/usr/bin/env python
#
#a Imports
import OpenGL.GLUT
import OpenGL.GL
import gjslib_c
import math
import sys
img_png_n=0
vector_z = gjslib_c.vector(vector=(0,0,1))
vector_x = gjslib_c.vector(vector=(1,0,0))

#c c_great_circle_arc
class c_great_circle_arc(object):
    """
    Points on a great circle are p such that p.n=0, where |p| =
    1. Effectively the points on the plane through the origin that has
    the great circle axis as its normal, where the points are distance
    1 from the origin (i.e. a circle radius one on the plane centered
    at the origin).

    An arc on the great circle is between two points p0 and p1, both
    on the great circle. These points can be described by the axis
    and, for each, and angle.

    As the are two halves to the great circle split by the points p0
    and p1, we have to define 'between'. We do this by defining the
    points segment 'between' to be the anticlockwise turn from p0 to
    p1; this reinforces the axis as p0 x p1.

    There is thus a set of three vectors gx, gz, gy: p0, axis, and
    axis x p0.  These are mutually orthogonal. p1 is then a linear
    combination of gx and gy (p1.gx, and p1.gy).

    If p1 is not in the first quadrant, then p can first be checked to
    see if it is in any quadrant between p1 and p0 except the first.
    The quadrant is the first if p.gx and p.gy are both positive; the
    second if p.gx is negative and p.gy is positive; the third if p.gx
    is negative and p.gy is negative; the fourth if p.gx is positive
    and p.gy is negative. (1,1, -1,1, -1,-1, 1,-1)

    Hence the quadrant of p is (1-sign(p.gx)*sign(p.gy))/2+(1-sign(p.gy)),
    or (3-sign(p.gy)*(2+sign(p.gx)))/2
    [ (3-1*(2+1))/2, (3-1*(2-1))/2, (3+1*(2-1))/2,  (3+1*(2+1))/2 ]
    [ 0, 1, 2, 3 ]

    So, find out the quadrant of p; if p is in the same quadrant as
    p1, then rotate both back into the first quadrant.
    In quadrant 1, map (p.gx,p.gy) to (p.gy,-p.gx)
    In quadrant 2, map (p.gx,p.gy) to (-p.gx,-p.gy)
    In quadrant 3, map (p.gx,p.gy) to (-p.gy,p.gx)

    If p1 is in the first quadrant (p0->p1 is < 90 degrees) then
    points between p0 and p1 will have p0.gx==1>=p.gx>=p1.gx,
    and p0.gy(==0)<p.gy<p1.gy
    """
    #f __init__
    def __init__(self, p0, p1):
        self.p1 = p1
        self.axis = p0.cross_product(p1)
        self.axis.normalize()
        self.gx = p0
        self.gy = (self.axis.cross_product(self.gx)).normalize()
        self.p1_quad_gx_gy = self.quad_coords(p1)
        pass
    #f quad_coords
    def quad_coords(self, p):
        """
        Calculate the quadrant of p, and gx/gy given p
        rotated back by 90 degree increments to get it
        into the first quadrant
        Then return a measure of how far round;
        this can be gy in the first quadrant (0<gy<1),
        and therefore quadrant+gy
        """
        gx = p.dot_product(self.gx)
        gy = p.dot_product(self.gy)
        if gy<0:
            if gx>0: return 3+gx
            return 2-gy
        if gx<0: return 1-gx
        return gy
    #f is_on_arc
    def is_on_arc(self, quad_gx_gy):
        """
        p0 is at quad=0, gx=1, gy=0
        p1 is at self.p1_quad_gx_gy

        a point is on the arc if its quad < p1's quad
        OR
        if its quad == p1's quad AND its gx > p1's gx
        """
        if quad_gx_gy>self.p1_quad_gx_gy: return False
        return True
    #f intersect
    def intersect(self, other):
        """
        All points a the GC have p.n=0
        Points where two great circles meet have p.n1 = p.n2 = 0

        A clear solution to this is p = k * (n1 x n2),
        as p.n1 = k*(n1 x n2).n1 = k*0 = 0, and similarly for p.n2
        k can be any real number.

        For points that are on the GC, they are distance 1 from the
        origin. Hence k = +-1, if n1 and n2 are unit vectors.

        Hence the intersection of two great circles is +- n1 x n2

        The questions then are: which of the two intersection points
        are we interested in, and does the intersection point lie
        'between' p0 and p1 for each great circle.

        Since an intersection point is +n1 x n2, we can find (pa.gx,
        pa.gy) for great circle A and (pb.gx, pb.gy) for great circle
        B. Note that -n1 x n2 is (-pa.gx, -pb.gy) and (-pb.gx, -pb.gy):
        or quadrant+2 of +n1 x n2.

        Also, it is good to know the sense of the crossings at an
        intersection. The 'sense' is whether pb0 is on the 'left' as
        one travels from pa0 to pa1; i.e. whether pb0-pb1.(pan) is +ve
        or -ve
        """
        p = self.axis.cross_product(other.axis)
        p.normalize()
        pa_quad_gx_gy = self.quad_coords(p)
        pb_quad_gx_gy = other.quad_coords(p)
        pb_sense_across_pa = cmp(self.axis.dot_product(other.p1-other.gx),0.0)
        pa_sense_across_pb = cmp(other.axis.dot_product(self.p1-self.gx),0.0)
        if (self.is_on_arc(pa_quad_gx_gy) and
            other.is_on_arc(pb_quad_gx_gy)): return (p, pa_quad_gx_gy, pb_quad_gx_gy, pb_sense_across_pa, pa_sense_across_pb)
        pa_quad_gx_gy += 2
        if pa_quad_gx_gy>4: pa_quad_gx_gy-=4
        pb_quad_gx_gy += 2
        if pb_quad_gx_gy>4: pb_quad_gx_gy-=4
        if (self.is_on_arc(pa_quad_gx_gy) and
            other.is_on_arc(pb_quad_gx_gy)): return (-p, pa_quad_gx_gy, pb_quad_gx_gy, pb_sense_across_pa, pa_sense_across_pb)
        return None
        

#f spherical_area_of_polygon
def spherical_area_of_polygon(p):
    l = len(p)
    area = 0
    for i in range(l):
        area += -spherical_angle(p[(i+1)%l], p[i], p[(i+2)%l])
        pass
    area -= (l-2)*math.pi
    return area

#c spherical_angle
def spherical_angle(p, p0, p1):
    """
    A spherical angle is formed from three points on the sphere, p, p0 and p1

    Two great circles (p,p0) and (p,p1) have an angle between them (the rotation
    anticlockwise about p that it takes to move the p,p0 great circle p,p1).
    Hence there is an axis through (p, 0, -p) about which p0 rotates to p1

    Consider the great circle for (p, p0); this is a rotation around
    an axis p x p0. Similarly for (p, p1).  If unit rotation axes are
    chosen (p_p0_r and p_p1_r) then the angle can be determined from
    p_p0_r.p_p1_r (cos of angle) and p.(p_p0_r x p_p1_r) (sin of angle)
    """
    p_p0_r = p.cross_product(p0).normalize()
    p_p1_r = p.cross_product(p1).normalize()
    ps = p_p0_r.cross_product(p_p1_r).dot_product(p)
    pc = p_p0_r.dot_product(p_p1_r)
    return math.atan2(ps, pc)

#c c_image_data
from octagon_image_maps_fine import image_map_data
class c_image_data(object):
    #f __init__
    def __init__(self, image_filename, focal_length, lens_type):
        self.image_filename = image_filename
        self.lp = gjslib_c.lens_projection(width=2.0, height=2.0*5184/3456, frame_width=22.3, focal_length=focal_length, lens_type=lens_type)
        self.map_data = {}
        self.base_orientation = None
        self.corners = None
        pass
    #f set_base_orientation
    def set_base_orientation(self, orientation):
        self.base_orientation = orientation
        self.lp.orient(orientation)
        self.corner_qs = ( self.lp.orientation_of_xy( (-1.0,-1.0) ),
                           self.lp.orientation_of_xy( ( 1.0,-1.0) ),
                           self.lp.orientation_of_xy( ( 1.0, 1.0) ),
                           self.lp.orientation_of_xy( (-1.0, 1.0) ) )
        self.corners = ( self.corner_qs[0].rotate_vector(vector_z),
                         self.corner_qs[1].rotate_vector(vector_z),
                         self.corner_qs[2].rotate_vector(vector_z),
                         self.corner_qs[3].rotate_vector(vector_z) )
                         
        self.arcs = ( c_great_circle_arc(self.corners[0], self.corners[1]),
                      c_great_circle_arc(self.corners[1], self.corners[2]),
                      c_great_circle_arc(self.corners[2], self.corners[3]),
                      c_great_circle_arc(self.corners[3], self.corners[0]))
        self.area = spherical_area_of_polygon(self.corners)
        pass
    #f find_overlap
    def find_overlap(self, other):
        overlaps = []
        does_overlap = False
        for s in range(4):
            for o in range(4):
                i = self.arcs[s].intersect(other.arcs[o])
                if i is not None:
                    does_overlap=True
                    overlaps.append((s,o,i))
                    pass
                pass
            pass
        if not does_overlap: return None

        #b find first overlap point around self
        result = []
        start = overlaps[0]
        current = start
        print overlaps
        def follow_overlap(overlaps, other_not_self, edge, start_p_quad_gx_gy):
            found_oi = None
            found_quad_gx_gy = None
            for i in range(len(overlaps)):
                if overlaps[i][other_not_self] != edge: continue
                (pt, pa_quad_gx_gy, pb_quad_gx_gy, pb_sense_across_pa, pa_sense_across_pb) = overlaps[i][2]
                quad_gx_gy = (pa_quad_gx_gy, pb_quad_gx_gy)[other_not_self]
                if quad_gx_gy <= start_p_quad_gx_gy: continue
                if (found_quad_gx_gy is not None) and quad_gx_gy>found_quad_gx_gy: continue
                found_oi = i
                found_quad_gx_gy = quad_gx_gy
                pass
            if found_oi is not None:
                if found_oi==0: return None
                return overlaps[found_oi]
            if other_not_self:
                return (None,(edge+1)%4,None)
            return ((edge+1)%4,None,None)
                
        while current is not None:
            result.append(current)
            (s,o,i) = current
            if o is None:
                current = follow_overlap(overlaps,0,s,-1)
                continue
            if s is None:
                current = follow_overlap(overlaps,1,o,-1)
                continue
            (pt, pa_quad_gx_gy, pb_quad_gx_gy, pb_sense_across_pa, pa_sense_across_pb) = i
            if pb_sense_across_pa<0:
                # follow self edge 's' beyond pa_quad_gx_gy
                current = follow_overlap(overlaps,0,s,pa_quad_gx_gy)
                continue
            # follow other edge 'o' beyond pb_quad_gx_gy
            current = follow_overlap(overlaps,1,o,pb_quad_gx_gy)
            pass

        polygon = []
        for (s,o,i) in result:
            if o is None:
                polygon.append(self.corners[s])
                continue
            if s is None:
                polygon.append(other.corners[o])
                continue
            (pt, pa_quad_gx_gy, pb_quad_gx_gy, pb_sense_across_pa, pa_sense_across_pb) = i
            polygon.append(pt)
            pass
        return polygon
    def find_overlaps(self, others):
        result = {}
        if self.corners is None:
            return None
        for o in others:
            if o==self: continue
            if o.corners is None: continue
            overlap = self.find_overlap(o)
            if overlap is not None:
                result[o.image_filename] = overlap
                pass
            pass
        return result
    def add_map_data(self, other, direction, image_mapping):
        self.map_data[(other, direction)] = image_mapping
        pass
    def propagate(self, ids_propagated_to):
        """
        for every other
        if that other is not in the set ids_propagated_to
        then propagate the top optimized orientation to that other
        and return list of others that were propagated to
        """
        if self.base_orientation is None: return
        result = []
        md = self.map_data.keys()
        md.sort(cmp=lambda x,y:cmp(str(x),str(y)))
        for od in md:
            (other, direction) = od
            if other in ids_propagated_to: continue
            bqs = self.map_data[od]['best_qs']
            if len(bqs)==0: continue
            (iteration, score, opt_q, start_q) = bqs[-1]
            if (iteration==0) and (score<20):
                print "Ignoring poor match %s => %s"%(self.image_filename, other.image_filename)
                continue

            opt_q = gjslib_c.quaternion(r=opt_q[0],i=opt_q[1],j=opt_q[2],k=opt_q[3])
            if od[1]==0: opt_q = ~opt_q
            other.set_base_orientation(self.base_orientation * opt_q)
            print self.image_filename, '-->',   other.image_filename
            result.append(other)
            pass
        return result
    def __str__(self):
        r = self.image_filename +" => "
        for od in self.map_data:
            (other, direction)=od
            r += " "+other.image_filename
            pass
        return r
    pass

ids = {}
for (i0, i1) in image_map_data:
    if i0 not in ids: ids[i0] = c_image_data(i0, focal_length=20.0, lens_type="rectilinear" )
    if i1 not in ids: ids[i1] = c_image_data(i1, focal_length=20.0, lens_type="rectilinear" )
    ids[i0].add_map_data( other=ids[i1], direction=1, image_mapping=image_map_data[(i0, i1)] )
    ids[i1].add_map_data( other=ids[i0], direction=0, image_mapping=image_map_data[(i0, i1)] )
    pass

image_names = ids.keys()
image_names.sort()
ids_sorted = []
for i in image_names:
    ids_sorted.append(ids[i])
    pass
key_image = ids[image_names[-1]]
key_image.set_base_orientation(gjslib_c.quaternion(r=1))

ids_to_propagate=[key_image]
ids_propagated_to=set()
while len(ids_to_propagate)>0:
    next_id = ids_to_propagate.pop(0)
    ids_propagated_to.add(next_id)
    ids_to_propagate.extend(next_id.propagate(ids_propagated_to))
    pass

vfs = []
vf_c = vector_z
vf_c -= vf_c
for idn in image_names:
    vf = ids[idn].base_orientation.rotate_vector(vector_z)
    vf_c += vf
    vfs.append( (idn, vf) )
    pass
vf_c.normalize()
vfs.sort(cmp=lambda x,y: cmp(y[1].dot_product(vf_c),x[1].dot_product(vf_c)))
print vf_c
for (i,vf) in vfs:
    print i, vf
    pass
key_image = ids[vfs[0][0]]

image_evaluation_path = []
image_overlaps = []
images_evaluated = set()
images_evaluated.add(key_image)
images_not_evaluated = set()
for id in ids.values():
    images_not_evaluated.add(id)
    pass
image_to_evaluate = key_image    
while image_to_evaluate is not None:
    images_not_evaluated.discard(image_to_evaluate)
    overlaps = image_to_evaluate.find_overlaps(ids.values()) # dict of name => overlap
    for i_name in overlaps:
        if ids[i_name] not in images_not_evaluated: continue
        polygon = overlaps[i_name]
        polygon_area = spherical_area_of_polygon(polygon)
        image_overlaps.append( (polygon_area, image_to_evaluate, ids[i_name], polygon) )
        pass
    image_overlaps.sort(cmp=lambda x,y:cmp(y[0],x[0])) # Sort image_overlaps largest overlap first
    image_to_evaluate = None
    while len(image_overlaps)>0:
        overlap = image_overlaps.pop(0)
        if overlap[2] not in images_not_evaluated: continue
        image_evaluation_path.append(overlap)
        image_to_evaluate = overlap[2]
        break
    pass
print "Image evaluation path", len(image_evaluation_path)
for (area, src, tgt, polygon) in image_evaluation_path:
    print src.image_filename, "->", tgt.image_filename, area
    pass

for i in image_names:
    print i, ids[i].base_orientation, str(ids[i])
    pass

for i in image_names:
    q = ids[i].base_orientation
    if q is None:
        print "# %s has no orientation"%(i,)
        pass
    else:
        print "('../images/%s',None,quaternion(r=%s,i=%s,j=%s,k=%s)),"%(i, q.r,q.i,q.j,q.k)
        pass
    pass

overlaps = key_image.find_overlaps(ids.values())
for i in overlaps:
    print "'%s':"%i, overlaps[i],","
    pass

print "Image evaluation path", len(image_evaluation_path)
for (area, src, tgt, polygon) in image_evaluation_path:
    rq = ~src.base_orientation * tgt.base_orientation
    print "$(eval $(call image_fine,%s,%s,20,canon_20_35_rebel2Ti_20,'%s'))"%(src.image_filename, tgt.image_filename,
                                                                  str(rq.rijk))
    pass

