#include "lemlib/api.hpp" // IWYU pragma: keep
#include "lemlib/chassis/chassis.hpp"
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

	// rotation reset
	claw_rotation.reset_position();
	lift_rotation.reset_position();
	// piston state
	claw_piston.set_value(false);
	intake_pistion_front.set_value(false);
	intake_piston_back.set_value(false);

	//chassis initilize
	chassis.calibrate();
	chassis.setPose(-58.85, -6.5, 270, false);

	Task([&] {
		while (true) {
			lcd::print(0, "X: %.3f", chassis.getPose().x);
            lcd::print(1, "Y: %.3f", chassis.getPose().y);
            lcd::print(2, "Theta: %.3f", chassis.getPose().theta);

			lcd::print(3, "lift Level: %d", level);

			lcd::print(4, "Claw Rotation: %d", claw_rotation.get_position());

			lcd::print(5, "Left Drive Temp: %d", left_motor_group.get_temperature());
			lcd::print(6, "Right Drive Temp: %d", right_motor_group.get_temperature());

			lcd::print(7, "Lift Error: %d", liftError);

			delay(100);
    	}
    });
}

Task auto_lift_task(lift_auto);
Task macro_intake_task(macro_intake);

void disabled() {}

void competition_initialize() {}

void autonomous() {
	liftTarget = 0;
	clawTarget = 0;
	// toggle
	chassis.tank(-60, -60);
	delay(400);
	chassis.tank(60, 60);
	delay(530);

	// alliance goal 1
	chassis.moveToPose(-45, -20, 0, 2000, {.forwards = false});
	liftTarget = 500;
	clawTarget = 9000;
	while (chassis.isInMotion()) {
		delay(2);
	}
	chassis.tank(-30, -30); // push into goal
	liftTarget = -950;
	while (std::abs(liftError) > 10) {
		delay(2);
	}
	delay(200);
	claw_piston.set_value(true);
	delay(200);
	liftTarget = 1000;

	// stack up
	chassis.setPose(47, 16.67, 0); // reset pose to minimise error
	intake.move(127);
	chassis.moveToPose(-39.2, -14.5, 75, 3000, {.minSpeed = 50, .earlyExitRange = 4}); // motion chain
	delay(400);
	clawTarget = 0;
	liftTarget = 0;
	chassis.moveToPose(-26.6, -19.7, 137, 5000);
	// while (chassis.isInMotion() || std::abs(liftError) > 20) { // wait for robot to stop AND lift to finish movement
	// 	delay(2);
	// }
	// chassis.tank(-20, -20); // prevent stack from stucking
	// delay(200);
	// chassis.tank(0, 0);
	// delay(800); // delay before checking stage 2 intake state
	// while (intake2Retract) { // wait until intake stage 2 drops down
	// 	delay(2);
	// }
	// delay(2000); // wait for stack to settle inside claw
	
	// claw_piston.set_value(false);
	// clawTarget = 9000;
	// liftTarget = 1900;
	// chassis.turnToPoint(-40, -23.5, 3000);
	// while (chassis.isInMotion() || std::abs(liftError) > 20) {
	// 	delay(2);
	// }
	// chassis.moveToPose(-40, -23.5, 90, 3000);
}

Controller master(E_CONTROLLER_MASTER);

void opcontrol() {
	auto_lift_task.remove();
	Task macro_lift_task(macro_lift);
	manual = false;
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
		intake_pistion_front.set_value(!intake1Retract); //flip both variable bc our pistons are flipped
		intake_piston_back.set_value(!intake2Retract);

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
			if (master.get_digital_new_press(E_CONTROLLER_DIGITAL_LEFT)) {
				clawLower = !clawLower;
			}
			if (clawLower) {
				claw.move((9000 - claw_rotation.get_position()) * 0.01);
			} else {
				claw.move((7000 - claw_rotation.get_position()) * 0.01);
			}
		} else {
			if (master.get_digital_new_press(E_CONTROLLER_DIGITAL_Y)) {
				scoring = false;
				if (level < 5) {
					level++;
				}
			} else if (master.get_digital_new_press(E_CONTROLLER_DIGITAL_RIGHT)) {
				scoring = false;
				if (level > 0) {
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