#include <stdlib.h>

// prime // downscale treshold / upscale treshold

size_t primes[] = {
    12289, // NA / 3072
    24593, // 1537 / 6148
    49193, // 3074 / 12298
    98387, // 6149 / 24596
    196799, // etc
    393611,
    787243,
    1574491,
    3148987,
    6297979,
    12595991,
    25191989,
    50383981,
    100767977,
    201535967,
    403071937,
    806143879,
    1612287763,
    3224575537UL,
#if _LP64
    6449151103,
    12898302233,
    25796604473,
    51593208973,
    103186417951,
    206372835917,
    412745671837,
    825491343683,
    1650982687391,
    3301965374803,
    6603930749621,
    13207861499251,
    26415722998507,
    52831445997037,
    105662891994103,
    211325783988211,
    422651567976461,
    845303135952931,
    1690606271905871,
    3381212543811743,
    6762425087623523,
    13524850175247127,
    27049700350494287,
    54099400700988593,
    108198801401977301,
    216397602803954641,
    432795205607909293,
    865590411215818597,
    1731180822431637217,
#endif
};
