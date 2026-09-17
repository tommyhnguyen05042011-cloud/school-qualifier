#include "subsystem.hpp"

inline int level = 0; /* keeps track of lift's current level */

inline bool loading = true; /* state of claw, loading (taking stacks from intake, only on lv0)
                               or scoring (parallel to ground to score)*/

inline bool scoring = false; /* is robot's next scoring system going to score or not
                                only happens higher than lv0*/

inline bool manual = false; /* state of control, manual or macroed.
                               when manual, driver will be able
                               to move the lift to whatever degree of movement he needs */

/* claw's position controlling macro. load is only available at lv 0, score is on all level */
inline void claw_load() {
    bar.move((bar_rotation.get_position()) * -0.035);
}
inline void claw_score() {
    bar.move((8600 - bar_rotation.get_position()) * 0.01);
}

/* scoring command */
inline void score_stack() {
    claw_piston.set_value(false);
    pros::delay(200);
    bar.move((10000 - bar_rotation.get_position()) * 0.01);
    scoring = false;
}