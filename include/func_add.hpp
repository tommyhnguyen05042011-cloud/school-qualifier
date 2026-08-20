#include "subsystem.hpp"


inline int level = 0; /* keeps track of lift's current level */

inline bool docking = true; /* state of claw, docking (taking stacks from intake)
                               or loading (taking match load), only read at lv0*/

inline bool scoring = false; /* is robot's next scoring system going to score or not
                                only happens higher than lv0*/

inline bool manual = false; /* state of control, manual or macroed.
                               when manual, driver will be able
                               to move the lift to whatever degree of movement he needs */

/* macro commands to move the scoring subsystem to certain heights, a.k.a levels */
inline void lv0() {}
inline void lv1() {}
inline void lv2() {}
inline void lv3() {}
inline void lv4() {}
inline void lv5() {}

/* claw's position controlling macro. dock is only available at lv0,
   loading is only available at lv 0-3, and raise is lv 4-5*/
inline void dock() {}
inline void load() {}
inline void raise() {}

/* scoring command*/
inline void score() {}