#!/usr/bin/env python
import math

PI = 3.14159265


#a Useful functions
def match_to_gimp(tx,ty,r,s):
    r = 2*PI/360*r
    return ( tx-512+s*(512*math.cos(r)-512*math.sin(r)), ty-512+s*(512*math.cos(r)+512*math.sin(r)))

#a Classes
#c c_mapping
class c_mapping(object):
    dist_factor = 25.0
    def __init__(self, src_pt, tgt_pt=None, translation=(0,0), rotation=0.0, scale=1.0, strength=0.0, other=None ):
        self.src_pt = src_pt
        self.tgt_pt = tgt_pt
        self.translation = translation
        self.rotation = rotation
        self.scale = scale
        self.strength = strength
        self.other = other
        pass
    def map_strength(self, translation=[0,0], scale=1.0, rotation=0.0, proposition=None):
        if proposition is not None:
            translation = proposition[0]
            rotation = proposition[1]
            scale = proposition[2]
            pass
        strength = self.strength
        if scale<self.scale:
            strength *= scale/self.scale
            pass
        else:
            strength *= self.scale/scale
            pass
        (dx,dy) = (translation[0]-self.translation[0], translation[1]-self.translation[1])
        translation_dist = math.sqrt(dx*dx+dy*dy)
        strength *= self.dist_factor/(self.dist_factor+translation_dist)
        strength *= math.cos(2*2*PI/360*(rotation-self.rotation))
        return strength
    def position_map_strength(self, translation=[0,0], scale=1.0, rotation=0.0, proposition=None):
        if proposition is not None:
            translation = proposition[0]
            rotation = proposition[1]
            scale = proposition[2]
            pass
        dsrc = self.src_pt[0], self.src_pt[1]
        cosang = -math.cos(2*PI/360*rotation)
        sinang = -math.sin(2*PI/360*rotation)
        dtgt = (translation[0] + scale*(cosang*dsrc[0] - sinang*dsrc[1]),
                translation[1] + scale*(cosang*dsrc[1] + sinang*dsrc[0]))
        dx = dtgt[0]-self.tgt_pt[0]
        dy = dtgt[1]-self.tgt_pt[1]
        dist = math.sqrt(dx*dx+dy*dy)
        strength = self.strength * (4.0/(4.0+dist))
        strength *= math.cos(2*2*PI/360*(rotation-self.rotation))
        return strength
    def __repr__(self):
        r = "(%4d,%4d) -> (%4d,%4d) : (%8.2f,%8.2f) %6.2f %5.3f    %8.5f  %s"%(self.src_pt[0], self.src_pt[1],
                                                                               self.tgt_pt[0], self.tgt_pt[1],
                                                                               self.translation[0], self.translation[1],
                                                                               self.rotation, self.scale,
                                                                               self.strength,
                                                                               str(self.other))
        return r

#c c_mapping_beliefs
class c_mapping_beliefs(object):
    """
    A point on the source image has a set of mapping beliefs, each with a concept of 'strength'
    These can be its ideas that it maps to particular points on the target with an image rotation,
    or it can be its ideas that the mapping for an image is a rotation and scale followed by a translation (tx, ty)
    This latter is built up from pairs of points, and is purer in our application as the rotation is derived
    from the pair of points rather than the phase of the FFT (which is more of a clue than a calculation)

    The point can be asked for its most-trusted proposition that it has not been told to ignore. It may have none.
    The main algorithm can combine these propositions from all source points to come up with a 'consensus proposition'
    Each point is then asked to use the consensus proposition to produce a new most-trusted proposition (even using data it has been told to ignore I think)
    The points may have none.
    A consensus propositino is again reached - and we repeat to get a most-agreed consensus.
    At this point the main algorithm outputs the most-agreed consensus, and informs each
    point to ignore propositions that support this consensus
    Repeat from the beginning until we have enough consensus propositions or there are no propositions
    """
    def __init__(self, src_pt):
        self.src_pt = src_pt
        self.mappings = []
        pass
    def add_mapping(self, mapping):
        self.mappings.append(mapping)
        pass
    def find_strongest_belief(self, proposition=None ):
        max_m = (None, 0.0)
        for m in self.mappings:
            s = m.strength
            if proposition is not None:
                s = m.position_map_strength(proposition=proposition)
                pass
            if (s>max_m[1]):
                max_m = (m, s)
                pass
            pass
        return max_m
    def strength_in_belief(self, proposition):
        strength = 0
        for m in self.mappings:
            strength += m.position_map_strength(proposition=proposition)
            pass
        return strength
    def defuse_beliefs(self, proposition):
        for m in self.mappings:
            s = m.position_map_strength(proposition=proposition)
            if (s>0):
                m.strength *= (1-s)*(1-s)
            pass
        pass
#a Data
matches = {}
matches[(409,648)] = [ ((415,601), 0.409896, -165.94),
                       ((468,514), 0.298426, 105.36),
                       ((414,597), 0.279425, -153.93),
                       ((329,893), 0.257135, 21.05),
                       ((329,900), 0.224686, 19.96),
                       ((655,627), 0.216222, 139.88),
                       ((328,887), 0.214691, 15.09),
                       ((415,591), 0.203371, -147.03),
                       ((614,735), 0.192571, 32.76),
                       ((329,905), 0.191142, 22.13),
                       ((83,933), 0.182600, 127.26),
                       ((475,513), 0.181218, 129.87),
                       ((43,921), 0.177076, 126.39),
                       ((116,720), 0.170693, 123.91),
                       ((329,917), 0.166949, 19.23),
                       ((616,699), 0.165070, 30.91),
                       ((479,514), 0.164416, 132.55),
                       ((47,922), 0.163267, 126.43),
                       ((246,769), 0.163116, 168.65),
                       ((612,741), 0.160636, 42.89),
                       ]
matches[(629,599)] = [ ((639,601), 0.236301, -168.64),
                       ((643,606), 0.175940, -147.35),
                       ((56,1008), 0.132194, -156.25),
                       ((636,597), 0.114021, -20.40),
                       ((159,742), 0.105704, 144.16),
                       ((415,622), 0.103199, 73.53),
                       ((460,599), 0.101663, -148.36),
                       ((710,671), 0.101293, -30.47),
                       ((166,745), 0.100905, 145.23),
                       ((658,678), 0.099398, -128.96),
                       ((603,666), 0.093681, -24.94),
                       ((555,722), 0.091070, 159.63),
                       ((341,992), 0.086761, -142.87),
                       ((459,623), 0.085996, 159.02),
                       ((693,665), 0.085820, -25.04),
                       ((493,684), 0.084699, 155.78),
                       ((416,628), 0.084201, 45.59),
                       ((458,595), 0.083956, -151.30),
                       ((142,737), 0.082411, 141.12),
                       ((436,544), 0.081900, 75.20),
                       ]
matches[(718,654)] = [ ((713,678), 0.200690, -152.11),
                       ((414,623), 0.178359, 67.23),
                       ((234,936), 0.123525, -28.86),
                       ((417,627), 0.123108, 51.13),
                       ((254,805), 0.114160, -143.66),
                       ((717,679), 0.111448, -154.93),
                       ((410,624), 0.111120, 61.12),
                       ((449,540), 0.103328, -39.62),
                       ((715,672), 0.101867, -19.58),
                       ((255,801), 0.092327, -141.01),
                       ((236,942), 0.090595, -165.81),
                       ((413,630), 0.088116, 28.37),
                       ((433,547), 0.086263, 62.89),
                       ((611,713), 0.084045, -116.03),
                       ((29,879), 0.082718, 34.83),
                       ((96,828), 0.079268, 124.87),
                       ((90,935), 0.078717, -40.84),
                       ((141,931), 0.076056, -49.78),
                       ((611,708), 0.073945, -109.61),
                       ((330,971), 0.072264, -6.99),
                       ]
matches[(109,1006)] = [ ((38,881), 0.236136, -157.62),
                             ((610,698), 0.235767, 52.12),
                             ((60,905), 0.229784, -20.86),
                             ((247,796), 0.213243, 36.09),
                             ((213,757), 0.209966, 151.15),
                             ((609,691), 0.191575, 49.94),
                             ((22,795), 0.173057, -144.54),
                             ((146,810), 0.171453, -44.62),
                             ((168,996), 0.169900, -66.56),
                             ((322,915), 0.169736, 47.32),
                             ((98,923), 0.160092, -138.61),
                             ((109,811), 0.159286, -37.48),
                             ((35,924), 0.155427, 140.52),
                             ((103,922), 0.154469, -148.35),
                             ((65,906), 0.146636, -32.18),
                             ((657,663), 0.146008, 48.14),
                             ((112,808), 0.144829, -41.13),
                             ((501,532), 0.144166, 152.04),
                             ((584,574), 0.142939, 152.86),
                             ((598,581), 0.142726, 155.12),
                             ]
matches[(594,730)] = [ ((575,722), 0.124875, -163.01),
                       ((556,708), 0.081450, 112.35),
                       ((48,795), 0.078939, -44.93),
                       ((90,901), 0.078477, -27.73),
                       ((552,712), 0.064928, 139.56),
                       ((243,802), 0.061766, -168.74),
                       ((78,769), 0.056576, -21.62),
                       ((207,757), 0.054343,  9.57),
                       ((222,775), 0.053606, 53.79),
                       ((52,766), 0.052642, -124.72),
                       ((58,837), 0.052184, -174.38),
                       ((318,920), 0.050444, -174.69),
                       ((21,981), 0.049639, -90.95),
                       ((94,902), 0.048829, -27.63),
                       ((216,769), 0.048819, -94.29),
                       ((563,707), 0.048477, 96.16),
                       ((212,775), 0.048363, -157.08),
                       ((38,988), 0.048336, -107.78),
                       ((557,714), 0.047889, 127.12),
                       ((46,998), 0.046616, 71.78),
                       ]
matches[(552,701)] = [ ((541,686), 0.191650, -164.02),
                            ((650,638), 0.078340, 130.38),
                            ((448,556), 0.068941, -55.90),
                            ((504,674), 0.059671, 24.31),
                            ((540,682), 0.058969, -158.21),
                            ((535,681), 0.058641, -147.65),
                            ((514,669), 0.057503, -159.54),
                            ((647,685), 0.054547, 19.61),
                            ((165,960), 0.053580, 23.39),
                            ((121,943), 0.050715, 173.93),
                            ((656,634), 0.048761, 129.14),
                            ((235,766), 0.048080, 145.11),
                            ((440,557), 0.047664, -46.80),
                            ((64,946), 0.047367, -13.63),
                            ((231,767), 0.046593, 132.21),
                            ((446,561), 0.046109, -46.28),
                            ((220,762), 0.044707, 42.21),
                            ((499,672), 0.044643, 32.05),
                            ((147,936), 0.044492, 53.40),
                            ((534,686), 0.043918, -163.75),
                            ]
matches[(413,674)] = [ ((413,628), 0.466134, -168.56),
                       ((613,709), 0.356187, 28.75),
                       ((412,632), 0.304900, -27.05),
                       ((664,637), 0.276853, 162.97),
                       ((610,712), 0.269210, 45.14),
                       ((444,541), 0.250117, 99.67),
                       ((188,743), 0.237414, 116.28),
                       ((598,748), 0.234832, -70.23),
                       ((412,624), 0.232249, -149.53),
                       ((415,636), 0.229338, -44.46),
                       ((326,876), 0.224096, 161.48),
                       ((192,744), 0.211649, 120.81),
                       ((31,880), 0.210817, -6.90),
                       ((246,770), 0.208041, 155.80),
                       ((614,705), 0.207894, 16.04),
                       ((434,548), 0.206973, -149.35),
                       ((34,886), 0.205353, -28.66),
                       ((416,587), 0.201332, -155.80),
                       ((44,902), 0.201292, -28.66),
                       ((603,748), 0.191263, 77.04),
                       ]
matches[(145,929)] = [ ((92,816), 0.325405, -158.52),
                            ((187,756), 0.226588, 50.82),
                            ((300,899), 0.199496, -94.23),
                            ((190,768), 0.189899, 29.46),
                            ((320,863), 0.182721, 80.88),
                            ((237,963), 0.174060, -26.93),
                            ((45,734), 0.170572, -49.32),
                            ((36,750), 0.168238, -6.44),
                            ((106,797), 0.167412, 24.55),
                            ((316,876), 0.161262, -62.51),
                            ((11,746), 0.150455, -168.25),
                            ((315,862), 0.147647, 70.84),
                            ((232,964), 0.146534, -6.25),
                            ((188,764), 0.144624, 48.05),
                            ((116,960), 0.143698, 28.69),
                            ((111,725), 0.140363, 24.57),
                            ((303,910), 0.140077, 53.88),
                            ((316,914), 0.139641, -149.18),
                            ((321,859), 0.137761, 61.32),
                            ((26,740), 0.137561, -53.04),
                            ]
matches[(259,965)] = [ ((192,876), 0.082994, -162.07),
                       ((80,976), 0.071699, 152.04),
                       ((198,877), 0.070038, -3.65),
                       ((145,877), 0.069728, -65.46),
                       ((152,868), 0.063078, -75.67),
                       ((189,822), 0.062293, -80.05),
                       ((167,850), 0.060983, -58.14),
                       ((49,932), 0.059570, -5.65),
                       ((543,708), 0.059150,  7.32),
                       ((529,698), 0.055070, -166.65),
                       ((80,773), 0.054241, -13.89),
                       ((65,937), 0.054155, -1.50),
                       ((58,935), 0.054060, -2.95),
                       ((88,944), 0.051972, -3.34),
                       ((30,925), 0.050702, 178.15),
                       ((505,684), 0.050331, -163.08),
                       ((140,879), 0.050152, 114.13),
                       ((194,881), 0.049875, -176.98),
                       ((161,854), 0.049602, 114.93),
                       ((36,927), 0.048910, 179.88),
                       ]
matches[(264,990)] = [ ((193,902), 0.157038, -161.46),
                            ((24,800), 0.093502, 170.15),
                            ((112,726), 0.091155, 119.44),
                            ((315,860), 0.090666, 178.12),
                            ((241,962), 0.086588, 85.30),
                            ((102,839), 0.082765, -98.09),
                            ((183,758), 0.080365, -178.48),
                            ((236,963), 0.077242, 104.03),
                            ((599,581), 0.076183, 157.04),
                            ((595,579), 0.074320, 163.39),
                            ((617,590), 0.069319, 149.07),
                            ((221,862), 0.069178, -127.49),
                            ((229,962), 0.068666, 112.60),
                            ((142,809), 0.067738, -54.33),
                            ((42,731), 0.067662, 84.75),
                            ((301,910), 0.067393, 176.81),
                            ((37,735), 0.066844, 65.86),
                            ((204,885), 0.066820, -119.47),
                            ((55,899), 0.065919, 141.79),
                            ((596,584), 0.065709, 155.39),
                            ]
matches[(294,930)] = [ ((236,851), 0.437860, -150.67),
                       ((254,951), 0.287798, 48.28),
                       ((235,846), 0.256970, -13.85),
                       ((219,879), 0.241496, -167.24),
                       ((293,845), 0.240931, 111.44),
                       ((60,847), 0.233739, 106.04),
                       ((283,909), 0.229294, 15.74),
                       ((249,817), 0.198830, 114.43),
                       ((64,848), 0.188376, 110.12),
                       ((238,856), 0.186796, -142.04),
                       ((234,857), 0.184837, -127.42),
                       ((172,931), 0.184453, -59.06),
                       ((195,784), 0.181197, 115.02),
                       ((289,905), 0.176077,  2.28),
                       ((177,934), 0.174474, -59.45),
                       ((286,912), 0.173371,  9.80),
                       ((206,902), 0.170376, -164.69),
                       ((289,844), 0.168690, 111.83),
                       ((308,852), 0.167410, 133.64),
                       ((168,929), 0.164013, -63.15),
                       ]
matches[(551,714)] = [ ((536,697), 0.331201, -168.01),
                            ((511,678), 0.172937, -163.80),
                            ((204,986), 0.131510, 165.03),
                            ((43,939), 0.121593, 169.39),
                            ((82,952), 0.111902, 169.52),
                            ((56,944), 0.100629, 170.72),
                            ((199,986), 0.099903, 158.89),
                            ((102,817), 0.098606, -137.33),
                            ((36,937), 0.097754, 167.25),
                            ((208,988), 0.097154, 165.22),
                            ((212,986), 0.095952, 161.74),
                            ((540,700), 0.092605, -163.05),
                            ((100,987), 0.091561, 164.20),
                            ((91,955), 0.089775,  9.46),
                            ((47,941), 0.089469, 165.97),
                            ((80,947), 0.088714, 171.65),
                            ((112,992), 0.087905, 169.02),
                            ((107,990), 0.083717, 169.97),
                            ((643,700), 0.083353, 161.15),
                            ((52,943), 0.082646, 167.94),
                            ]
matches[(270,942)] = [ ((211,856), 0.188994, -166.28),
                       ((311,859), 0.103776, -47.10),
                       ((164,802), 0.100402, 96.82),
                       ((207,854), 0.096542, -162.02),
                       ((208,884), 0.080647, 175.78),
                       ((208,859), 0.079350, -167.44),
                       ((133,812), 0.078826, 41.07),
                       ((443,568), 0.077114, -34.83),
                       ((107,843), 0.073743, -4.49),
                       ((96,845), 0.070855, 119.58),
                       ((101,841), 0.070316, 11.14),
                       ((297,901), 0.063573, -104.66),
                       ((230,863), 0.060409, 124.24),
                       ((207,894), 0.059695, 146.32),
                       ((222,863), 0.059550, 42.62),
                       ((203,851), 0.058337, -142.02),
                       ((103,847), 0.057528, 11.95),
                       ((42,764), 0.057224, -113.74),
                       ((204,858), 0.057057, -157.57),
                       ((193,1002), 0.056677, 106.77),
                       ]
matches[(307,981)] = [ ((307,856), 0.381343, -122.24),
                            ((238,903), 0.312026, -165.53),
                            ((233,904), 0.278860, -141.21),
                            ((227,896), 0.268652, 113.99),
                            ((57,705), 0.253079, -132.16),
                            ((244,901), 0.250045, -23.94),
                            ((35,692), 0.234918, -132.72),
                            ((77,717), 0.222121, -132.01),
                            ((240,842), 0.216840, 78.98),
                            ((17,796), 0.214774, 112.97),
                            ((50,701), 0.207043, -134.71),
                            ((126,756), 0.205934, 33.92),
                            ((24,685), 0.205683, -131.95),
                            ((223,875), 0.203742, -37.52),
                            ((51,904), 0.201062, -121.19),
                            ((45,698), 0.200580, -135.52),
                            ((166,809), 0.199272, -19.43),
                            ((201,907), 0.198774, -43.43),
                            ((18,790), 0.197856, 135.87),
                            ((40,695), 0.196991, -132.55),
                            ]
matches[(280,954)] = [ ((66,843), 0.398235, -98.85),
                       ((94,717), 0.325705, 106.38),
                       ((65,847), 0.305602, -91.74),
                       ((309,959), 0.295836, -57.78),
                       ((203,892), 0.289276, -158.15),
                       ((35,811), 0.275289, 117.70),
                       ((317,859), 0.270160, 139.85),
                       ((155,787), 0.257478, -58.72),
                       ((164,804), 0.256179, -42.70),
                       ((62,841), 0.256118, -102.15),
                       ((124,764), 0.254651, 67.20),
                       ((30,804), 0.249719, 119.57),
                       ((206,955), 0.248734, 82.04),
                       ((151,781), 0.248186, -61.23),
                       ((90,714), 0.240025, 101.44),
                       ((306,954), 0.239195, -77.29),
                       ((37,738), 0.237451, 31.17),
                       ((106,929), 0.234039, 146.51),
                       ((33,741), 0.232759, 30.45),
                       ((70,849), 0.232252, -95.55),
                       ]
matches[(441,552)] = [ ((470,514), 0.655382, -150.17),
                            ((656,629), 0.350765, -127.56),
                            ((466,515), 0.344277, -172.07),
                            ((474,513), 0.332944, -138.57),
                            ((655,690), 0.323298, 124.08),
                            ((660,632), 0.299571, -115.15),
                            ((204,749), 0.285818, -142.47),
                            ((650,694), 0.268626, 155.68),
                            ((82,915), 0.264180, 23.14),
                            ((76,914), 0.262070, 28.91),
                            ((241,903), 0.258955, 173.58),
                            ((646,696), 0.248461, 19.64),
                            ((432,574), 0.247354, -136.21),
                            ((244,770), 0.240595, -100.72),
                            ((66,913), 0.238569, 38.57),
                            ((420,579), 0.231901, -33.54),
                            ((86,914), 0.230115, 18.99),
                            ((200,747), 0.227972, -142.44),
                            ((328,893), 0.226728, 115.36),
                            ((71,914), 0.224047, 30.04),
                            ]
matches[(298,970)] = [ ((246,895), 0.138040, 29.00),
                       ((230,891), 0.137592, -171.75),
                       ((167,766), 0.128329, -77.46),
                       ((134,747), 0.121663, -70.19),
                       ((233,888), 0.121634, -151.63),
                       ((54,903), 0.115944, -71.14),
                       ((233,899), 0.094396, 80.26),
                       ((591,577), 0.091746, 108.39),
                       ((235,884), 0.085811, -167.55),
                       ((186,907), 0.085281, -48.48),
                       ((310,853), 0.084000, -65.04),
                       ((48,897), 0.083349, -39.64),
                       ((253,893), 0.081567, 29.24),
                       ((39,883), 0.080949, -42.02),
                       ((171,769), 0.080680, -62.45),
                       ((247,884), 0.078802, 160.65),
                       ((139,750), 0.075258, -67.25),
                       ((24,805), 0.074650, -46.61),
                       ((305,850), 0.074086, -73.25),
                       ((146,754), 0.074014, -67.76),
                       ]
matches[(420,655)] = [ ((423,609), 0.140029, -165.20),
                            ((279,964), 0.090139, -50.00),
                            ((146,954), 0.078129, 136.99),
                            ((23,977), 0.062647, -46.94),
                            ((509,660), 0.059900, -37.43),
                            ((134,1013), 0.058805, 157.55),
                            ((31,979), 0.058607, -35.62),
                            ((561,698), 0.058420, -140.46),
                            ((643,621), 0.057680, 13.82),
                            ((318,882), 0.057621, 18.79),
                            ((243,798), 0.055923, -25.62),
                            ((320,887), 0.054920, 25.25),
                            ((150,734), 0.054646, -41.09),
                            ((283,965), 0.054449, -39.78),
                            ((506,645), 0.054300, -38.47),
                            ((544,662), 0.053592, -35.46),
                            ((422,613), 0.053410, -161.82),
                            ((193,1009), 0.052487, 136.21),
                            ((559,706), 0.052344, -130.64),
                            ((24,860), 0.051709, -36.15),
                            ]
matches[(107,911)] = [ ((59,788), 0.511781, -158.49),
                       ((114,963), 0.194855, 59.65),
                       ((287,935), 0.182603,  3.69),
                       ((116,729), 0.179082, 176.88),
                       ((243,1012), 0.176449, 71.39),
                       ((153,809), 0.175116, 134.74),
                       ((299,897), 0.167825, 78.07),
                       ((199,783), 0.167468, 162.96),
                       ((110,965), 0.161662, 70.85),
                       ((244,1008), 0.160140, 61.73),
                       ((29,787), 0.158158, 57.88),
                       ((12,743), 0.157133, 13.06),
                       ((106,936), 0.154762, 78.66),
                       ((118,966), 0.154224, 60.43),
                       ((60,736), 0.152840, -16.95),
                       ((12,835), 0.152440, -6.72),
                       ((222,793), 0.151766,  3.68),
                       ((250,1010), 0.151011, 53.37),
                       ((208,792), 0.150898,  6.74),
                       ((309,880), 0.150754, 84.52),
                       ]
matches[(118,1011)] = [ ((238,964), 0.467828, 136.63),
                             ((609,685), 0.362084, 72.09),
                             ((233,964), 0.351836, 148.78),
                             ((50,893), 0.343653, -140.45),
                             ((46,889), 0.338257, -157.93),
                             ((15,675), 0.338113, -36.86),
                             ((41,821), 0.327050, -157.21),
                             ((95,908), 0.321751, -23.84),
                             ((303,912), 0.315482, -138.76),
                             ((242,963), 0.313578, 128.53),
                             ((21,675), 0.308182, -22.22),
                             ((127,991), 0.302878, -28.64),
                             ((120,762), 0.301065, 144.08),
                             ((151,808), 0.297952, -39.22),
                             ((67,772), 0.295774, -38.71),
                             ((316,855), 0.295419, -157.98),
                             ((139,807), 0.293201, -30.15),
                             ((305,952), 0.289085, 13.31),
                             ((101,911), 0.287358, -145.17),
                             ((438,580), 0.283115, 139.60),
                             ] 

#a Toplevel
print "*"*80
print "\n"*5
print matches.keys()
points_passed = []
correlations = []    
for src_pt in matches.keys():
    if len(points_passed)==0:
        points_passed.append(src_pt)
        continue
    for (tgt_pt,l,angle) in matches[src_pt]:
        for p in points_passed:
            for (tgt_pt_p,l_p,angle_p) in matches[p]:
                dx0 = tgt_pt[0]-tgt_pt_p[0]
                dy0 = tgt_pt[1]-tgt_pt_p[1]
                l0 = math.sqrt(dx0*dx0+dy0*dy0)

                dx1 = src_pt[0]-p[0]
                dy1 = src_pt[1]-p[1]
                l1 = math.sqrt(dx1*dx1+dy1*dy1)
                cosang = -(dx0*dx1+dy0*dy1)/l0/l1 # rotation
                cosang2 = math.cos((angle-angle_p)*2*PI/360) # both rotations should be the same - cos should be near 1
                # cosang should equal cosang3 give or take
                # angdiff should be about rotation
                angdiff = 360*(math.atan2(dy0,dx0)-math.atan2(dy1,dx1))/2/PI+180
                if (angdiff<0): angdiff += 360
                if (angdiff>=360): angdiff -= 360
                cosmatch2 = math.cos((angdiff-angle)*2*PI/360)
                scale = l0/l1
                if False:
                    print " (%d,%d) -> (%d,%d)  and   (%d,%d) -> (%d,%d)"%(
                                src_pt[0], src_pt[1], tgt_pt[0], tgt_pt[1],
                                p[0], p[1], tgt_pt_p[0], tgt_pt_p[1] )
                    print "   src_dxy (%f,%f) tgt_dxy (%f,%f) src_l %f tgt_l %f"%(
                                dx1, dy1, dx0, dy0, l1, l0 )
                    print "   scale %f rotation %f fft_rotation %f pm_fft_rotation %f"%(
                                scale, angdiff, angle, angle_p )
                    print "   cos_rot_diff %f"%(cosmatch2)
                    pass
                if (cosang2>0.90) and (cosmatch2>0.90) and (scale>0.95) and (scale<1.05):
                    if True:
                        print " (%d,%d) -> (%d,%d)  and   (%d,%d) -> (%d,%d)"%(
                            src_pt[0], src_pt[1], tgt_pt[0], tgt_pt[1],
                            p[0], p[1], tgt_pt_p[0], tgt_pt_p[1] )
                        print "   src_dxy (%f,%f) tgt_dxy (%f,%f) src_l %f tgt_l %f"%(
                            dx1, dy1, dx0, dy0, l1, l0 )
                        print "   scale %f rotation %f fft_rotation %f pm_fft_rotation %f"%(
                            scale, angdiff, angle, angle_p )
                        print "   cos_rot_diff %f"%(cosmatch2)
                        pass
                    # There is a mapping src -> tgt that is tgt = scale*rotation*src + translation
                    # Hence (tgt_pt - tgt_pt_p) = scale * rotation * (src_pt-p)
                    # or (dx0,dy0) = scale*rotation*(dx1,dy1)
                    # Hence (dx0,dy0).(dx1.dy1) = (scale*rotation*(dx1,dy1)).(dx1,dy1)
                    # Hence (dx0,dy0).(dx1.dy1) = scale*cos(rotation)*((dx1,dy1).(dx1,dy1))
                    # Hence scale*cos(rotation) = (dx0,dy0).(dx1,dy1) / (l1^2)
                    # Hence scale*cos(rotation) = l0.l1.cosang / (l1^2)
                    # Hence cos(rotation) should match cosang, and scale is l0/l1
                    # tgt_pt = scale*rotation*src_pt + translation
                    # Hence translation = tgt_pt - scale*rotation*src_pt
                    cosang3 = -math.cos((angdiff)*2*PI/360)
                    sinang3 = -math.sin((angdiff)*2*PI/360)
                    translation = (tgt_pt[0] - scale*(src_pt[0]*cosang3 - src_pt[1]*sinang3),
                                   tgt_pt[1] - scale*(src_pt[1]*cosang3 + src_pt[0]*sinang3))
                    mapping = c_mapping( src_pt = src_pt,
                                         tgt_pt = tgt_pt,
                                         translation = translation,
                                         rotation = angdiff,
                                         scale    = scale,
                                         strength = 1.0,
                                         other = (p, tgt_pt_p) )
                    correlations.append(mapping)
                    pass
                pass
            pass
        #print dx0,dy0,src_pt,tgt_pt
        pass
    points_passed.append(src_pt)
    pass
mapping_beliefs = {}
for c in correlations:
    if c.src_pt not in mapping_beliefs:
        mapping_beliefs[c.src_pt] = c_mapping_beliefs(c.src_pt)
        pass
    mapping_beliefs[c.src_pt].add_mapping(c)
    pass
print "*"*80
for m in mapping_beliefs:
    print
    #print mapping_beliefs[m].find_strongest_belief()
    for mm in mapping_beliefs[m].mappings:
        #print mm, mm.position_map_strength(translation=(168,-126),scale=1.0,rotation=13.5)
        #print mm, mm.position_map_strength(translation=(-168,126),scale=1.0,rotation=-13.5)
        print mm, mm.position_map_strength(translation=(168,-126),scale=1.0,rotation=180+13.5)
        #print mm, mm.position_map_strength(translation=(168,-126),scale=1.0,rotation=180-13.5)
    pass

print "*"*80

for j in range(10):
    best_mapping = (None,0)
    for m in mapping_beliefs:
        (mapping, s_in_m) = mapping_beliefs[m].find_strongest_belief()
        if (s_in_m>best_mapping[1]):
            best_mapping = (mapping, s_in_m)
            pass
        pass
    if best_mapping[0] is None:
        break
    d = {"translation":mapping.translation, "rotation":mapping.rotation, "scale":mapping.scale}
    # d = {"translation":(600,600), "rotation":180, "scale":1}
    for i in range(10):
        rotation = d["rotation"]
        scale    = d["scale"]
        translation   = d["translation"]
        next_data = {"translation":(0,0), "rotation":0, "scale":0}
        total_strength = 0
        for m in mapping_beliefs:
            s_in_b = mapping_beliefs[m].strength_in_belief( (translation, rotation, scale) )
            (mapping,s_in_m) = mapping_beliefs[m].find_strongest_belief( (translation, rotation, scale) )
            if mapping is not None:
                s = s_in_m
                next_data["translation"] = (next_data["translation"][0]+s*mapping.translation[0],
                                       next_data["translation"][1]+s*mapping.translation[1])
                next_data["rotation"] += s*mapping.rotation
                next_data["scale"]    += s*mapping.scale
                total_strength        += s
                pass
            #print s_in_b, s_in_m, mapping
            pass
        next_data["translation"] = (next_data["translation"][0]/total_strength,
                               next_data["translation"][1]/total_strength)
        next_data["rotation"] = next_data["rotation"]/total_strength
        next_data["scale"] = next_data["scale"]/total_strength
        next_data["strength"] = total_strength
        d = next_data
    print j, d
    proposition = (d["translation"], d["rotation"], d["scale"])
    for m in mapping_beliefs:
        mapping_beliefs[m].defuse_beliefs(proposition)
        pass
    pass
