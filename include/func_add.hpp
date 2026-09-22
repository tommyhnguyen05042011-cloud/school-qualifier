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
inline bool intake1Retract = true;
inline bool intake2Retract = true;
inline bool currentOpticalDetect = false;
inline bool lastOpticalDetect = false;
inline int clawTarget = 0;
inline int liftTarget = 0;

inline void macro_intake() {
    while (true) {
        currentOpticalDetect = (optical.get_proximity() > 50 && optical.get_proximity() < 255);

        if (!intake1Retract) {
            intake2Retract = false;
        } else if (currentOpticalDetect && !lastOpticalDetect) {
            intake2Retract = false;
            pros::delay(1500);
        } else if (!currentOpticalDetect) {
            intake2Retract = true;
        }

        lastOpticalDetect = currentOpticalDetect;
        pros::delay(20);
    }
}

inline void macro_lift() {
	while (true) {
        if (!scoring && !manual) {
            if (level > 0) {
                hold_score = clawClose;
            }
            if (level == 0) {
                if (!clawClose) {
                    hold_score = false;
                }
                if (hold_score && clawClose) {
                    claw.move_absolute(9000, 127);
                    clawTarget = 9000;
                } else {
                    claw.move_absolute(100, 127);
                    clawTarget = 100;
                }
                lift.move_absolute(0, 127);
                liftTarget = 0;
            } else if (level == 1) {
                claw.move_absolute(9000, 127);
                clawTarget = 9000;
                lift.move_absolute(2300, 127);
                liftTarget = 2300;
            } else if (level == 2) {
                claw.move_absolute(9000, 127);
                clawTarget = 9000;
                lift.move_absolute(4500, 127);
                liftTarget = 4500;
            } else if (level == 3) {
                claw.move_absolute(9000, 127);
                clawTarget = 9000;
                lift.move_absolute(6000, 127);
                liftTarget = 6000;
            }
            while (!(lift.get_position() > (liftTarget + 5) && lift.get_position() < (liftTarget - 5) &&
                     claw.get_position() > (clawTarget + 5) && claw.get_position() < (clawTarget - 5))) {
                pros::delay(2);
            }
            lift.brake();
            claw.brake();
        }
		pros::delay(20);
	}
}

/* scoring command */
inline void score_stack() {
    if (level != 0) {
        claw_piston.set_value(true);
        pros::delay(200);
        claw.move_absolute(10000, 127);
        clawTarget = 10000;
        while (!((claw.get_position() > (clawTarget + 5)) && (claw.get_position() < (clawTarget - 5)))) {
            pros::delay(2);
        }
        pros::delay(1000);
        clawClose = false;
        scoring = false;
    }
}