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
	claw_rotation.reset_position();
	// lift rotation reset
	lift.tare_position_all();
	// piston state
	claw_piston.set_value(true);
	intake_pistion_front.set_value(false);
	intake_piston_back.set_value(false);

	Task([&] {
		while (true) {
			lcd::print(0, "lift Level: %d", level);
			lcd::print(1, "Lift Rotation: %f", lift.get_position());

			if (loading == true) {
				lcd::print(2, "Bar: Load");
			} else {
				lcd::print(2, "Bar: Score");
			}

			if (scoring == true) {
				lcd::print(3, "Scoring");
			} else {
				lcd::print(3, "Not Scoring");
			}

			lcd::print(4, "Bar Rotation: %d", claw_rotation.get_position());

			lcd::print(5, "Left Drive Temp: %d", left_motor_group.get_temperature());
			lcd::print(6, "Right Drive Temp: %d", right_motor_group.get_temperature());

			delay(100);
    	}
    });
}

void disabled() {}

void competition_initialize() {}

void autonomous() {}

Controller master(E_CONTROLLER_MASTER);

void opcontrol() {
	// Start macro task ONCE
	Task macro_control_task(macro_control);

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
			intake1Retract = !intake1Retract;
			intake2Retract = !intake2Retract;
		}
		intake_pistion_front.set_value(intake1Retract);
		intake_piston_back.set_value(intake2Retract);

		// toggle claw
		if (master.get_digital_new_press(E_CONTROLLER_DIGITAL_R1)) {
				clawClose = !clawClose;
		}
		if (!scoring) {
			claw_piston.set_value(!clawClose); //claw piston is flipped so i flipped the clawClose as well
		}

		// manual toggle
		if (master.get_digital_new_press(E_CONTROLLER_DIGITAL_B)) {
			manual = !manual;
		}

		// what to do if manual or not
		if (manual) {
			if (master.get_digital(E_CONTROLLER_DIGITAL_Y)) {
				lift.move(127);
			} else if (master.get_digital(E_CONTROLLER_DIGITAL_RIGHT)) {
				lift.move(-127);
			} else {
				lift.move(0);
			}
		} else {
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
				clawClose = false;
				claw_piston.set_value(clawClose);
			} else {
				scoring = !scoring;
				if (scoring) {
					Task scoring_task(score_stack); // only summon scoring task ONCE if scoring is true
				}
			}
		}

		//reseting lift
		if (master.get_digital(E_CONTROLLER_DIGITAL_UP)) {
			scoring = false;
			level = 0;
		}

		delay(20);
	}
}