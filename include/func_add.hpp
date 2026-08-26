#include "subsystem.hpp"

inline int level = 0; /* keeps track of lift's current level */

inline bool loading = true; /* state of claw, loading (taking stacks from intake, only on lv0)
                               or scoring (parallel to ground to score)*/

inline bool scoring = false; /* is robot's next scoring system going to score or not
                                only happens higher than lv0*/

inline bool manual = false; /* state of control, manual or macroed.
                               when manual, driver will be able
                               to move the lift to whatever degree of movement he needs */

/* macro commands to move the scoring subsystem to certain heights, a.k.a levels */
inline void lift_lv0() {
    lift.move(lift.get_position() * -1);
}
inline void lift_lv1() {
    lift.move((1100 - lift.get_position()));
}
inline void lift_lv2() {
    lift.move((2200 - lift.get_position()));
}
inline void lift_lv3() {
    lift.move((3300 - lift.get_position()));
}
inline void lift_lv4() {
    lift.move((4400 - lift.get_position()));
}
inline void lift_lv5() {
    lift.move((5300 - lift.get_position()));
}

/* claw's position controlling macro. docking is only available at lv 0, load is on all level */
inline void claw_load() {
    if (loading == true) {
        lift.move((lift.get_position()) * 0.05);
        bar.move((bar_rotation.get_position()) * 0.01);
    }
}
inline void claw_score() {
    if (loading == false) {
        bar.move((10000 - bar_rotation.get_position()) * 0.01);
    }
}

/* scoring command */
inline void score_stack() {
    claw_piston.set_value(false);
    pros::delay(200);
    bar.move((12000 - bar_rotation.get_position()) * 0.01);
}