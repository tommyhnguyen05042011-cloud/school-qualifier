#include "subsystem.hpp"

inline int level = 0; /* keeps track of lift's current level */
inline bool hold_score = false; /* determine whether Claw should stay
                                   at score or at load*/

inline bool loading = true; /* state of claw, loading (taking stacks from intake, only on lv0)
                               or scoring (parallel to ground to score)*/

inline bool scoring = false; /* is robot's next scoring system going to score or not
                                only happens higher than lv0*/

inline bool manual = false; /* state of control, manual or macroed.
                               when manual, driver will be able
                               to move the lift to whatever degree of movement he needs */

inline bool clawClose = true;
inline bool intake1Retract = false;
inline bool intake2Retract = false;
inline bool currentOpticalDetect = false;
inline bool lastOpticalDetect = false;

/* claw's position controlling macro. load is only available at lv 0, score is on all level */
inline void claw_load() {
    bar.move((10 - claw_rotation.get_position()) * 0.02);
}
inline void claw_score() {
    bar.move((9000 - claw_rotation.get_position()) * 0.02);
}

inline void macro_control() {
	while (true) {
        if (!scoring && !manual) {
            if (level > 0) {
                hold_score = clawClose;
            }
            if (level == -1) {
                claw_score();
                pros::delay(200);
                lift.move(-300 - lift.get_position() * 1);
            } else if (level == 0) {
                if (!clawClose) {
                    hold_score = false;
                }
                if (hold_score && clawClose) {
                    claw_score();
                } else {
                    claw_load();
                }
                lift.move((-lift.get_position()) * 0.5);
            } else if (level == 1) {
                lift.move((1200 - lift.get_position()) * 1);
                claw_score();
            } else if (level == 2) {
                lift.move((2000 - lift.get_position()) * 1);;
                claw_score();
            } else if (level == 3) {
                lift.move((2640 - lift.get_position()) * 1);
                claw_score();
            } else if (level == 4) {
                lift.move((3520 - lift.get_position()) * 1);
                claw_score();
            } else if (level == 5) {
                lift.move((4400 - lift.get_position()) * 1);
                claw_score();
            }
        }
		pros::delay(20);
	}
}

/* scoring command */
inline void score_stack() {
    claw_piston.set_value(false);
    pros::delay(200);
    bar.move((10000 - claw_rotation.get_position()) * 0.03);
    scoring = false;
}