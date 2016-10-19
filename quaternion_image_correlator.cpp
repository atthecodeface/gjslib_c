/*a Documentation
 */
/*a Includes
 */
#include "quaternion_image_correlator.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <map>
#include "filter.h" // for point_value
#include "vector.h"
#include "quaternion.h"

/*a Defines
*/
#define DEFAULT_MATCH_LENGTH (32)

/*a Base types
 */
/*t t_angle_axis
*/
typedef struct
{
    double angle;
    c_vector axis;
} t_angle_axis;

/*t t_qi_src_tgt_pv
*/
typedef struct
{
    const c_quaternion *src_q; // actual src_q of match
    const c_quaternion *tgt_q; // actual tgt_q of match
    t_point_value pv;
} t_qi_src_tgt_pv;

/*c c_qi_src_tgt_pair_mapping - pair of src_qs mapped to pair of tgt_qs

    A quaternion_mapping is a mapping from a pair of source
    quaternions to a pair of target quaternions, where each quaternion
    represents a point on an image (the representation being by the
    application of the quaternion to the vector (0,0,1)).

    The quaternions must be unit quaternions

    As such there is a unique quaternion that will transform both of
    the target quaternions' points to the respective source
    quaternions' points.

        There is a rotation (angle, axis) that has src_q0.vector_z/src_q1.vector_z on a great circle,
        and similarly for tgt_q0.vector_z/tgt_q1.vector_z
        (The spherical coordinates of a point corresponding to src_q/tgt_q is q.vector_z in our definition)

        Given this we have two great circles; these are either the same, or they meet at two points.
        If they are the same we have src_axis==tgt_axis
        We can find the transformation that rotates src axis to tgt axis - this we call diff_q
        Hence diff_q . src_axis = tgt axis

        Now, diff_q is a rotation about an axis which goes through the great circles for src and tgt
        each in two places. diff_q has no effect on these positions (da0 and -da0).

        So, we can consider a first transformation that moves src_q0.vector_z to da0; this is moving
        the point around the great circle with the axis src_axis. Next apply the diff_q transformation;
        this has no effect on the point. Finally apply a last transformation that moves da0 to tgt_q0.vector_z, 
        moving it along its great circle hence as a rotation around tgt_axis.
        This combination is a transformation for src_q0.vector_z to tgt_q0.vector_z

        Consider the effect on a point src_q1.vector_z - also on the
        great circle, perhaps 10 degrees further clockwise round. 
        Our first rotation will place the point at da0 + 10 degrees clockwise on the src great circle
        The diff_q transformation moves this to da0 + 10 degrees clockwise on the tgt great circle
        The last rotation moves it to tgt_q1.vector_z - i.e. tgt_q0.vector_z + 10 degrees clockwise
        (assuming the src/tgt points correspond).

        The first rotation is a rotation around src_axis to make src_q0.vector_z move to diff_axis.
        This is simply the transformation to move diff_axis to src_q0.vector_z on a great circle
        The last is is also just the transformation to move diff_axis to tgt_q0.vector_z on a great circle

        Now if src_axis == +-tgt_axis then all that is needed is a rotation around src_axis
*/
class c_qi_src_tgt_pair_mapping
{
public:
    static c_qi_src_tgt_pair_mapping *test_add( const c_quaternion *src_q0,
                                                const c_quaternion *src_q1,
                                                const c_quaternion *tgt_q0,
                                                const c_quaternion *tgt_q1,
                                                double max_angle_diff_ratio );
    c_qi_src_tgt_pair_mapping(const c_quaternion *src_q0,
                              const c_quaternion *src_q1,
                              const c_quaternion *tgt_q0,
                              const c_quaternion *tgt_q1,
                              const t_angle_axis *src_angle_axis,
                              const t_angle_axis *tgt_angle_axis);
    void calculate(c_quaternion &src_from_tgt_orient, const t_angle_axis *src_angle_axis, const t_angle_axis *tgt_angle_axis);
    inline double distance_to(const c_quaternion *src_from_tgt_q) {
        return src_from_tgt_q->distance_to(src_from_tgt_orient);
    }
    void repr(char *buffer, int buf_size);

    const c_quaternion *src_qs[2];
    const c_quaternion *tgt_qs[2];
    c_quaternion src_from_tgt_orient;
};

/*c c_qi_src_tgt_match - list of (single src_q to tgt_q match with t_point_value)
  Each elements of the list belongs to a src_qx and tgt_qx, where the elements are
  close enough to src_qx and tgt_qx (in terms of mapping of vector_z).
  
  Also contains a list of mappings
*/
class c_qi_src_tgt_match
{
public:
    c_qi_src_tgt_match(const c_quaternion *src_qx, const c_quaternion *tgt_qx);
    int add_match(const c_quaternion *src_q, const c_quaternion *tgt_q, const t_point_value *pv);
    double tgt_distance(const c_quaternion *tgt_q) const;
    void reset_mappings(void);
    inline void add_mapping(c_qi_src_tgt_pair_mapping *stpm) {mappings.push_back(stpm);}
    double score_src_from_tgt(const c_quaternion *src_from_tgt_q, double min_cos_sep, double max_q_dist) const;

    const c_quaternion *src_qx;
    const c_quaternion *tgt_qx;
    std::vector<t_qi_src_tgt_pv> matches;
    std::vector<c_qi_src_tgt_pair_mapping *> mappings;
};

/*a Statics
 */
/*v Statics
*/
static const double z[3]={0.0,0.0,1.0};
static c_vector vector_z=c_vector(3,z);

/*a c_qi_src_tgt_match methods
 */
/*f c_qi_src_tgt_match::c_qi_src_tgt_match
*/
c_qi_src_tgt_match::c_qi_src_tgt_match(const c_quaternion *src_qx, const c_quaternion *tgt_qx)
{
    this->src_qx = src_qx;
    this->tgt_qx = tgt_qx;
    this->matches = std::vector<t_qi_src_tgt_pv>();
    this->matches.reserve(DEFAULT_MATCH_LENGTH);
}

/*f c_qi_src_tgt_match::add_match
*/
int
c_qi_src_tgt_match::add_match(const c_quaternion *src_q, const c_quaternion *tgt_q, const t_point_value *pv)
{
    t_qi_src_tgt_pv qstp;
    qstp.src_q = src_q;
    qstp.tgt_q = tgt_q;
    qstp.pv = *pv;
    matches.push_back(qstp); // can throw bad_alloc on resize
    return 0;
}

/*f c_qi_src_tgt_match::tgt_distance
  Calculate a measure of distance between this tgt and another, where each is a point mapping
  of vector_z
*/
double
c_qi_src_tgt_match::tgt_distance(const c_quaternion *tgt_q) const
{
    c_quaternion tgt_q_diff = this->tgt_qx->angle_axis(tgt_q, vector_z);
    return tgt_q_diff.r();
}

/*f c_qi_src_tgt_match::score_src_from_tgt
 *
 * For a given tgt->src transformation, generate a score
 * The score is the count of matching source/target pairs which
 * have a transform that is similar
*/
double
c_qi_src_tgt_match::score_src_from_tgt(const c_quaternion *src_from_tgt_q, double min_cos_sep, double max_q_dist) const
{
    double score = 0;
    double dst = ((*src_from_tgt_q)*(*tgt_qx)).angle_axis(src_qx,vector_z).r();
    if (dst<min_cos_sep) return score;

    //if verbose: print "passed src_q/tgt_qx mapping"
    for (auto qm : mappings) {
        double dq = qm->distance_to(src_from_tgt_q);
        if (dq>max_q_dist)
            continue;
        score += 1;
    }
    return score;
}

/*a c_qi_src_tgt_pair_mapping methods
 */
/*f c_qi_src_tgt_pair_mapping::c_qi_src_tgt_pair_mapping
*/
c_qi_src_tgt_pair_mapping::c_qi_src_tgt_pair_mapping(const c_quaternion *src_q0,
                                                     const c_quaternion *src_q1,
                                                     const c_quaternion *tgt_q0,
                                                     const c_quaternion *tgt_q1,
                                                     const t_angle_axis *src_angle_axis,
                                                     const t_angle_axis *tgt_angle_axis)
{
    this->src_qs[0] = src_q0;
    this->src_qs[1] = src_q1;
    this->tgt_qs[0] = tgt_q0;
    this->tgt_qs[1] = tgt_q1;
    calculate(src_from_tgt_orient, src_angle_axis, tgt_angle_axis);
    // "These two should be equal... or near as... 1,0,0,0"
    if (0) {
        for (int i=1; i>=0; i--) {
            c_quaternion q=(src_from_tgt_orient * (*tgt_qs[i])).angle_axis((*src_qs[i]), vector_z);
            char buffer[1024];
            fprintf(stderr, "%d:%s\n",i,q.__str__(buffer,sizeof(buffer)));
        }
    }
}

/*f c_qi_src_tgt_pair_mapping::test_add
*/
c_qi_src_tgt_pair_mapping *
c_qi_src_tgt_pair_mapping::test_add(const c_quaternion *src_q0,
                                    const c_quaternion *src_q1,
                                    const c_quaternion *tgt_q0,
                                    const c_quaternion *tgt_q1,
                                    double max_angle_diff_ratio )
{
    c_quaternion src_q_diff = src_q0->angle_axis(src_q1, vector_z);
    c_quaternion tgt_q_diff = tgt_q0->angle_axis(tgt_q1, vector_z);

    t_angle_axis src_angle_axis;
    t_angle_axis tgt_angle_axis;
    src_angle_axis.angle = src_q_diff.as_rotation(src_angle_axis.axis);
    tgt_angle_axis.angle = tgt_q_diff.as_rotation(tgt_angle_axis.axis);

    if (fabs((src_angle_axis.angle-tgt_angle_axis.angle)) >=
        fabs(src_angle_axis.angle*max_angle_diff_ratio))
        return NULL;
    return new c_qi_src_tgt_pair_mapping(src_q0, src_q1, tgt_q0, tgt_q1, &src_angle_axis, &tgt_angle_axis);
}

/*f c_qi_src_tgt_pair_mapping::calculate
*/
void
c_qi_src_tgt_pair_mapping::calculate(c_quaternion &src_from_tgt_orient, const t_angle_axis *src_angle_axis, const t_angle_axis *tgt_angle_axis)
{
    c_vector sp0 = c_vector(src_qs[0]->rotate_vector(vector_z));
    c_vector tp0 = c_vector(tgt_qs[0]->rotate_vector(vector_z));
    c_vector diff_axis;

    c_quaternion diff_q = src_angle_axis->axis.angle_axis_to_v(tgt_angle_axis->axis);
    double diff_angle   = diff_q.as_rotation(diff_axis);
    if (0) {
        char buffer[1024];
        fprintf(stderr, "diff_q %s\n",                        diff_q.__str__(buffer,sizeof(buffer)));
        fprintf(stderr, "sp0 %lf, %lf, %lf\n",sp0.coords()[0],sp0.coords()[1],sp0.coords()[2]);
        fprintf(stderr, "tp0 %lf, %lf, %lf\n",tp0.coords()[0],tp0.coords()[1],tp0.coords()[2]);
        fprintf(stderr, "diff %lf: %lf, %lf, %lf\n",diff_angle, diff_axis.coords()[0],diff_axis.coords()[1],diff_axis.coords()[2]);
    }

    if (diff_q.r()>0.99999999) {
        src_from_tgt_orient = tp0.angle_axis_to_v(sp0);
        return;
    }

    c_quaternion src_sp0_orient_to_diff_axis = diff_axis.angle_axis_to_v(sp0);
    c_quaternion dst_tp0_orient_to_diff_axis = tp0.angle_axis_to_v(diff_axis);

    src_from_tgt_orient = src_sp0_orient_to_diff_axis * ~diff_q * dst_tp0_orient_to_diff_axis;
    if (0) {
        char buffer[1024];
        fprintf(stderr, "diff_q %s\n",                        diff_q.__str__(buffer,sizeof(buffer)));
        fprintf(stderr, "src_sp0_orient_to_diff_axis %s\n",   src_sp0_orient_to_diff_axis.__str__(buffer,sizeof(buffer)));
        fprintf(stderr, "dst_tp0_orient_to_diff_axis %s\n",   dst_tp0_orient_to_diff_axis.__str__(buffer,sizeof(buffer)));
        fprintf(stderr, "src_from_tgt_orient %s\n",           src_from_tgt_orient.__str__(buffer,sizeof(buffer)));
    }
}

/*a c_quaternion_image_correlator methods
 */
/*f c_quaternion_image_correlator::c_quaternion_image_correlator
 */
c_quaternion_image_correlator::c_quaternion_image_correlator(void)
{
    min_cos_angle_src_q = 0.9999; // 80pix35, 0.81 degrees
    min_cos_angle_tgt_q = 0.9999; // 80pix35, 0.81 degrees
    max_angle_diff_ratio = 0.02;  // for src->src/tgt->tgt to be permitted, 2% difference at most in angle between points on great circle
    min_cos_sep_score    = 0.9999; // 80pix35, 0.81 degrees
    max_q_dist_score     =0.0004; // 80pix35, 0.81 degrees

    //min_q_dist        = min_q_dists["80pix35"];
    //min_cos_sep       = min_cos_seps["80pix35"];
    //min_cos_sep_score = min_cos_seps["80pix35"];
    //max_q_dist_score  = min_q_dists["80pix35"];
}

/*f c_quaternion_image_correlator::find_close_src_qx
*/
const c_quaternion *
c_quaternion_image_correlator::find_close_src_qx(const c_quaternion *src_q, double *cos_angle_ptr) const
{
    for (auto src_qx : src_qs) {
        double cos_angle;
        cos_angle = src_qx->angle_axis(src_q, vector_z).r();
        if (cos_angle>min_cos_angle_src_q) {
            if (cos_angle_ptr) *cos_angle_ptr=cos_angle;
            return src_qx;
        }
    }
    return NULL;
}

/*f c_quaternion_image_correlator::find_closest_src_qx
*/
const c_quaternion *
c_quaternion_image_correlator::find_closest_src_qx(const c_quaternion *src_q, double *cos_angle_ptr) const
{
    double max_cos_angle;
    const c_quaternion *closest_src_qx=NULL;

    for (auto src_qx : src_qs) {
        double cos_angle;
        cos_angle = src_qx->angle_axis(src_q, vector_z).r();
        if ((!closest_src_qx) || (cos_angle>max_cos_angle)) {
            closest_src_qx = src_qx;
            max_cos_angle = cos_angle;
        }
    }
    return closest_src_qx;
}

/*f c_quaternion_image_correlator::find_closest_tgt_qx
 */
const c_qi_src_tgt_match *
c_quaternion_image_correlator::find_closest_tgt_qx( const c_quaternion *src_qx,
                                                    const c_quaternion *tgt_q,
                                                    double *cos_angle_ptr) const
{
    double max_cos_angle;
    const c_qi_src_tgt_match *closest_iqm=NULL;

    if (matches_by_src_q.count(src_qx)<1) return NULL;
    auto src_qx_ml = &(matches_by_src_q.at(src_qx));
    for (auto iqm : *src_qx_ml) {
        double cos_angle;
        cos_angle = tgt_q->angle_axis(iqm->tgt_qx, vector_z).r();
        if ((!closest_iqm) || (cos_angle>max_cos_angle)) {
            closest_iqm = iqm;
            max_cos_angle = cos_angle;
        }
    }
    if (cos_angle_ptr) *cos_angle_ptr=max_cos_angle;
    return closest_iqm;
}

/*f c_quaternion_image_correlator::find_or_add_tgt_q_to_src_q
*/
c_qi_src_tgt_match *
c_quaternion_image_correlator::find_or_add_tgt_q_to_src_q(const c_quaternion *src_qx,
                                                          t_quaternion_image_match_list *src_tgt_match_list,
                                                          const c_quaternion *tgt_q)
{
    c_qi_src_tgt_match *closest_iqm;
    closest_iqm = NULL;
    for (auto iqm : *src_tgt_match_list) {
        double d;
        d = iqm->tgt_distance(tgt_q);
        if (d>min_cos_angle_tgt_q) {
            closest_iqm = iqm;
            break;
        }
    }
    if (!closest_iqm) {
        closest_iqm = new c_qi_src_tgt_match(src_qx, tgt_q->copy());
        if (closest_iqm)
            src_tgt_match_list->push_back(closest_iqm);
    }
    return closest_iqm;
}

/*f c_quaternion_image_correlator::add_src_q
*/
const c_quaternion *
c_quaternion_image_correlator::add_src_q(const c_quaternion *src_q)
{
    c_quaternion *src_qx = src_q->copy();
    src_qs.push_back(src_qx);
    matches_by_src_q[src_qx] = t_quaternion_image_match_list();
    return src_qx;
}

/*f c_quaternion_image_correlator::find_of_add_src_q (find current src_q mapping point close enough to proposed src_q, or add a new match)
*/
const c_quaternion *
c_quaternion_image_correlator::find_or_add_src_q(const c_quaternion *src_q)
{
    const c_quaternion *close_src_qx;
    close_src_qx = find_close_src_qx(src_q, NULL);
    if (close_src_qx) return close_src_qx;
    return add_src_q(src_q);
}

/*f c_quaternion_image_correlator::add_match
*/
int
c_quaternion_image_correlator::add_match(const c_quaternion *src_q,
                                         const c_quaternion *tgt_q,
                                         const t_point_value *pv)
{
    const c_quaternion *src_qx;
    t_quaternion_image_match_list *match_list;
    c_qi_src_tgt_match *match; // src_qx,tgt_qx match - list of matches that are close enough to src_q and then close enough to tgt_q

    src_qx = find_or_add_src_q(src_q);
    match_list = &(matches_by_src_q.find(src_qx)->second);
    match = find_or_add_tgt_q_to_src_q(src_qx, match_list, tgt_q);
    if (!match) {
        return -1;
    }
    match->add_match(src_q, tgt_q, pv);
    return 0;
}

/*f c_quaternion_image_correlator::create_mappings
 * For every pair of src_qx's (src_q0,src_q1):
 *  for each tgt_q0, tgt_q1 in src_q0, src_q1:
 *    create a c_qi_src_tgt_pair_mapping from (src_q0, src_q1) to (tgt_q0, tgt_q1)
 *    IF such a mapping is viable (e.g. rotation angle from src_q0->1 is same as tgt_q0->q1)
*/
int
c_quaternion_image_correlator::create_mappings(void)
{
    //const c_quaternion *src_q0, *src_q1;
    //c_qi_src_tgt_match *iqm0, *iqm1;
    const c_quaternion *tgt_q0, *tgt_q1;
    t_quaternion_image_match_list *src_q0_ml, *src_q1_ml;
    c_qi_src_tgt_pair_mapping *qm;

    for (auto src_q0 : src_qs) {
        for (auto src_q1 : src_qs) {
            if (src_q0==src_q1) continue;
            src_q0_ml = &(matches_by_src_q[src_q0]);
            src_q1_ml = &(matches_by_src_q[src_q1]);
            for (auto iqm0 : *src_q0_ml) {
                tgt_q0 = iqm0->tgt_qx;
                for (auto iqm1 : *src_q1_ml) {
                    tgt_q1 = iqm1->tgt_qx;
                    qm = c_qi_src_tgt_pair_mapping::test_add(src_q0, src_q1, tgt_q0, tgt_q1, max_angle_diff_ratio);
                    if (!qm) continue;
                    iqm0->add_mapping(qm);
                }
            }
        }
    }
    return 0;
}

/*f c_quaternion_image_correlator::score_src_from_tgt
 *
 * For each src_qx
 *  For each tgt_qx that the src_qx maps to
 *   Generate a score
 *   If the score is the best for src_qx ->, then record as max
 *  Add up max scores as the total score
*/
double
c_quaternion_image_correlator::score_src_from_tgt(const c_quaternion *src_from_tgt_q)
{
    total_score.score = 0;
    total_score.match_list.clear();
    for (auto src_qx_ml : matches_by_src_q) {
        double max_score=-1;
        c_qi_src_tgt_match *max_iqm=NULL;
        for (auto iqm : src_qx_ml.second) {
            double score;
            score = iqm->score_src_from_tgt(src_from_tgt_q, min_cos_sep_score, max_q_dist_score);
            if (score>max_score) {
                max_score = score;
                max_iqm = iqm;
            }
        }
        if (max_iqm) {
            total_score.score += max_score;
            total_score.match_list.push_back(max_iqm);
        }
    }
    return total_score.score;
}

/*f c_quaternion_image_correlator::qstm_src_q
 */
const c_quaternion *
c_quaternion_image_correlator::qstm_src_q(class c_qi_src_tgt_match *qstm) const
{
    return qstm->src_qx;
}

/*f c_quaternion_image_correlator::qstm_tgt_q
 */
const c_quaternion *
c_quaternion_image_correlator::qstm_tgt_q(class c_qi_src_tgt_match *qstm) const
{
    return qstm->tgt_qx;
}

/*f c_quaternion_image_correlator::nth_src_tgt_q_mapping
 */
const c_quaternion *
c_quaternion_image_correlator::nth_src_tgt_q_mapping(const c_qi_src_tgt_match *qstm,
                                                     int n,
                                                     const c_quaternion *src_tgt_qs[4]) const
{
    if ((n<0) || (n>=qstm->mappings.size()))
        return NULL;

    src_tgt_qs[0] = qstm->mappings[n]->src_qs[0];
    src_tgt_qs[1] = qstm->mappings[n]->src_qs[1];
    src_tgt_qs[2] = qstm->mappings[n]->tgt_qs[0];
    src_tgt_qs[3] = qstm->mappings[n]->tgt_qs[1];
    return &(qstm->mappings[n]->src_from_tgt_orient);
}