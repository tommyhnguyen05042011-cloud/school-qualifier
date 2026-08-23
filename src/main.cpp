#include "lemlib/api.hpp" // IWYU pragma: keep
#include "pros/ai_vision.hpp"
#include "pros/misc.h"
#include "func_add.hpp"
#include <csignal>
#include <cstdio>
#include "main.h"
#include "pros/llemu.hpp"

using namespace pros;
using namespace lemlib;

void initialize() {
	lcd::initialize();

	// ai vision initialize
	aiVision.reset();
    aiVision.enable_detection_types(AivisionModeType::tags);
    aiVision.set_tag_family(AivisionTagFamily::tag_21H7, true);

	// bar rotation reset
	bar_rotation.reset_position();
	// lift rotation reset
	lift.set_zero_position_all(0);
	// piston state
	claw_piston.set_value(true);
	intake_pistion_front.set_value(true);
	intake_piston_back.set_value(true);

	Task([&] {
        while (true) {
			lcd::print(0, "lift Level: %d", level);

			if (loading == true) {
				lcd::print(1, "Bar: Dock");
			} else {
				lcd::print(1, "Bar: Load");
			}

			if (scoring == true) {
				lcd::print(2, "Scoring");
			} else {
				lcd::print(2, "Not Scoring");
			}

			lcd::print(3, "Bar Rotation: %d", bar_rotation.get_position());
    }
    });
}

void disabled() {}

void competition_initialize() {}

void autonomous() {}

Controller master(E_CONTROLLER_MASTER);

void opcontrol() {
	bool clawState = true;
	bool lastLEFTState;
	bool currentLEFTState;
	
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

		// toggle claw
		currentLEFTState = master.get_digital(E_CONTROLLER_DIGITAL_LEFT);
		if (currentLEFTState && !lastLEFTState) {
			clawState = !clawState;
			claw_piston.set_value(clawState);
		}
		lastLEFTState = currentLEFTState;

		// manual toggle
		if (master.get_digital_new_press(E_CONTROLLER_DIGITAL_B)) {
			manual = !manual;
		} else {}

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
				} else {}
			} else if (master.get_digital_new_press(E_CONTROLLER_DIGITAL_RIGHT)) {
				scoring = false;
				if (level > 0) {
					level--;
				}
			}
		}

		// dock, load & score
		if (master.get_digital_new_press(E_CONTROLLER_DIGITAL_X)) {
			if (level == 0) {
				loading = !loading;
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
				if (level == 0) {
					lift_lv0();
				} else if (level == 1) {
					lift_lv1();
					loading = false; /* this is to prevent the robot from automatically return to docking position
										if the driver wish to stay at loading position */
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
		} else {
			score_stack();
		}

		delay(30);
	}
}