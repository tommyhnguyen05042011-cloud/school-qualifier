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
inline void lift_lvsub0() {
    lift.move(300 - lift.get_position() * -1);
}
inline void lift_lv0() {
    lift.move(lift.get_position() * -1);
}
inline void lift_lv1() {
    lift.move((1200 - lift.get_position()) * 1);
}
inline void lift_lv2() {
    lift.move((2000 - lift.get_position()) * 1);
}
inline void lift_lv3() {
    lift.move((2640 - lift.get_position()) * 1);
}
inline void lift_lv4() {
    lift.move((3520 - lift.get_position()) * 1);
}
inline void lift_lv5() {
    lift.move((4400 - lift.get_position()) * 1);
}

/* claw's position controlling macro. load is only available at lv 0, score is on all level */
inline void claw_load() {
    bar.move((bar_rotation.get_position()) * -0.035);
}
inline void claw_score() {
    bar.move((8600 - bar_rotation.get_position()) * 0.035);
}

// lift macro level control (run in a task)
inline void macro_control() {
    if (level == -1) {
        lift_lvsub0();
    } else if (level == 0) {
        lift_lv0();
        pros::delay(200);
        claw_load();
    } else if (level == 1) {
        lift_lv1();
        claw_score();
    } else if (level == 2) {
        lift_lv2();
        claw_score();
    } else if (level == 3) {
        lift_lv3();
        claw_score();
    } else if (level == 4) {
        lift_lv4();
        claw_score();
    } else if (level == 5) {
        lift_lv5();
        claw_score();
    }
}

/* scoring command */
inline void score_stack() {
    claw_piston.set_value(false);
    pros::delay(200);
    bar.move((10000 - bar_rotation.get_position()) * 0.01);
}