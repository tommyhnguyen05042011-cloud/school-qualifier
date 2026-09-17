#include "lemlib/api.hpp" // IWYU pragma: keep
#include "pros/misc.h"
#include "func_add.hpp"
#include <csignal>
#include <cstdio>
#include "main.h"
#include "pros/llemu.hpp"
#include "pros/rtos.hpp"

using namespace pros;
using namespace lemlib;

void initialize() {
	lcd::initialize();

	// bar rotation reset
	bar_rotation.set_position(0);
	// lift rotation reset
	lift.set_zero_position_all(0);
	// piston state
	claw_piston.set_value(true);
	intake_pistion_front.set_value(false);
	intake_piston_back.set_value(false);

	Task([&] {
		while (true) {
			lcd::print(0, "lift Level: %d", level);
			lcd::print(1, "Lift Pose: %f", lift.get_position());

			if (loading == true) {
				lcd::print(2, "Bar: Dock");
			} else {
				lcd::print(2, "Bar: Load");
			}

			if (scoring == true) {
				lcd::print(3, "Scoring");
			} else {
				lcd::print(3, "Not Scoring");
			}

			lcd::print(4, "Bar Rotation: %d", bar_rotation.get_position());

			lcd::print(5, "Left Drive Temp: %f", left_motor_group.get_temperature());
			lcd::print(6, "Right Drive Temp: %f", right_motor_group.get_temperature());

			delay(100);
    	}
    });
}

void disabled() {}

void competition_initialize() {}

void autonomous() {}

Controller master(E_CONTROLLER_MASTER);

bool clawState = false;
bool intake1State = false;
bool intake2State = false;
bool currentOpticalState = false;
bool lastOpticalState = false;

void macro_control() {
    if (level == -1) {
        lift.move(300 - lift.get_position() * 1);
        claw_load();
        pros::delay(100);
        lift.move(300 - lift.get_position() * -1);
    } else if (level == 0) {
		if (clawState == false) {
			claw_load();
			delay(100);
		} else {
			claw_score();
		}
        lift.move(lift.get_position() * -1);
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

void opcontrol() {	
	while (true) {
		// get left y and right x positions
        int leftY = master.get_analog(E_CONTROLLER_ANALOG_LEFT_Y);
        int rightX = master.get_analog(E_CONTROLLER_ANALOG_RIGHT_X);

        // move the robot
        chassis.arcade(leftY, rightX, false, 0.5);

		// intake control
		if (master.get_digital(E_CONTROLLER_DIGITAL_L1)) {
			intake.move(127);
		} else if (master.get_digital(E_CONTROLLER_DIGITAL_L2)) {
			intake.move(-127);
		} else {
			intake.move(0);
		}

		// intake pneumatics control
		if (master.get_digital_new_press(E_CONTROLLER_DIGITAL_A)) {
			intake1State = !intake1State;
			intake2State = !intake2State;
		}
		intake_pistion_front.set_value(intake1State);
		intake_piston_back.set_value(intake2State);

		// toggle claw
		if (master.get_digital_new_press(E_CONTROLLER_DIGITAL_R1)) {
			clawState = !clawState;
		}
		claw_piston.set_value(clawState);

		// manual toggle
		if (master.get_digital_new_press(E_CONTROLLER_DIGITAL_B)) {
			manual = !manual;
		}

		// what to do if manual or not
		if (manual == true) {
			if (master.get_digital(E_CONTROLLER_DIGITAL_Y)) {
				lift.move(127);
			} else if (master.get_digital(E_CONTROLLER_DIGITAL_RIGHT)) {
				lift.move(-127);
			} else {
				lift.move(0);
			}
		} else if (manual == false) {
			if (master.get_digital_new_press(E_CONTROLLER_DIGITAL_Y)) {
				scoring = false;
				if (level < 5) {
					level++;
				}
			} else if (master.get_digital_new_press(E_CONTROLLER_DIGITAL_RIGHT)) {
				scoring = false;
				if (level > -1) {
					level--;
				}
			}
		}

		// load & score
		if (master.get_digital_new_press(E_CONTROLLER_DIGITAL_X)) {
			if (level == 0) {
				clawState = false;
				claw_piston.set_value(clawState);
			} else {
				scoring = !scoring;
			}
		}

		//reseting lift
		if (master.get_digital(E_CONTROLLER_DIGITAL_UP)) {
			scoring = false;
			level = 0;
		}

		// level --> mech control (manual is false)
		if (scoring == false) {
			if (manual == false) {
				Task macro_control_task(macro_control);
			}
		} else {
			Task scoring_task(score_stack);
		}

		delay(20);
	}
}