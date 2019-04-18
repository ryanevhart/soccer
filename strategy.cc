#include "naobehavior.h"
#include "../rvdraw/rvdraw.h"

#include <cstdlib>
#include <fcntl.h>
#include <vector>

extern int agentBodyType;

/*
 * Real game beaming.
 * Filling params x y angle
 */
void NaoBehavior::beam( double& beamX, double& beamY, double& beamAngle ) {
    beamX = -HALF_FIELD_X + worldModel->getUNum();
    beamY = 0;
    beamAngle = 0;
}


int timeSlice = 0;

   /*
(-15,10)       (-13.3333,10)       (-11.6667,10)       (-10,10)       (-8.33333,10)       (-6.66667,10) (-5,10) (-3.33333,10) (-1.66667,10) (0,10) 
(-15,7.77778)  (-13.3333,7.77778)  (-11.6667,7.77778)  (-10,7.77778)  (-8.33333,7.77778)  (-6.66667,7.77778) (-5,7.77778) (-3.33333,7.77778) (-1.66667,7.77778) (0,7.77778) 
(-15,5.55556)  (-13.3333,5.55556)  (-11.6667,5.55556)  (-10,5.55556)  (-8.33333,5.55556)  (-6.66667,5.55556) (-5,5.55556) (-3.33333,5.55556) (-1.66667,5.55556) (0,5.55556) 
(-15,3.33333)  (-13.3333,3.33333)  (-11.6667,3.33333)  (-10,3.33333)  (-8.33333,3.33333)  (-6.66667,3.33333) (-5,3.33333) (-3.33333,3.33333) (-1.66667,3.33333) (0,3.33333) 
(-15,1.11111)  (-13.3333,1.11111)  (-11.6667,1.11111)  (-10,1.11111)  (-8.33333,1.11111)  (-6.66667,1.11111) (-5,1.11111) (-3.33333,1.11111) (-1.66667,1.11111) (0,1.11111) 
(-15,-1.11111) (-13.3333,-1.11111) (-11.6667,-1.11111) (-10,-1.11111) (-8.33333,-1.11111) (-6.66667,-1.11111) (-5,-1.11111) (-3.33333,-1.11111) (-1.66667,-1.11111) (0,-1.11111) 
(-15,-3.33333) (-13.3333,-3.33333) (-11.6667,-3.33333) (-10,-3.33333) (-8.33333,-3.33333) (-6.66667,-3.33333) (-5,-3.33333) (-3.33333,-3.33333) (-1.66667,-3.33333) (0,-3.33333) 
(-15,-5.55556) (-13.3333,-5.55556) (-11.6667,-5.55556) (-10,-5.55556) (-8.33333,-5.55556) (-6.66667,-5.55556) (-5,-5.55556) (-3.33333,-5.55556) (-1.66667,-5.55556) (0,-5.55556) 
(-15,-7.77778) (-13.3333,-7.77778) (-11.6667,-7.77778) (-10,-7.77778) (-8.33333,-7.77778) (-6.66667,-7.77778) (-5,-7.77778) (-3.33333,-7.77778) (-1.66667,-7.77778) (0,-7.77778) 
(-15,-10)      (-13.3333,-10)      (-11.6667,-10)      (-10,-10)      (-8.33333,-10)      (-6.66667,-10) (-5,-10) (-3.33333,-10) (-1.66667,-10) (0,-10)
    */

double x_increment = 2 * HALF_FIELD_X / 9;
double y_increment = 2 * HALF_FIELD_Y / 9;

// takes the position coordinates and turns it into letter equivalent
string getMyPosition(double xpos, double ypos) {

    

    double i;
    double j;

    double closest_i = 100;
    double closest_i_distance = 100;
    double closest_j = 100;
    double closest_j_distance = 100;

    char closest_i_char = '0';
    char closest_j_char = '0';
    int i_index = 65;
    int j_index = 65;
    
    for (j = HALF_FIELD_Y; j >= -HALF_FIELD_Y; j-=y_increment) {
        if (closest_j_distance > abs(ypos - j)) {
            closest_j = j;
            closest_j_distance = abs(ypos - j);
            closest_j_char = (char) j_index;
        }
        j_index++;
    }

    for (i = -HALF_FIELD_X; i <= HALF_FIELD_X; i+=x_increment) {    
        if (closest_i_distance > abs(xpos - i)) {
            closest_i = i;
            closest_i_distance = abs(xpos - i);
            closest_i_char = (char) i_index;
        }
        i_index++;
    }

    

    string retVal = "";
    retVal += closest_j_char;
    retVal += closest_i_char;

    return retVal;
}

string offender_goal = "";
int goal = -1;

SkillType attackerPlan;
SkillType defenderPlan;


double recent_i = -100;
double recent_j = -100;

double recent_i_attacker = -100;
double recent_j_attacker = -100;

int close(double x1, double y1, double x2, double y2) {
	int diff = abs(x1 - x2) + abs(y1 - y2);
	if (diff <= 7) {
		return 1;
	} else {
		return 0;
	}
}

std::map<int, SkillType> actionCache;

SkillType NaoBehavior::selectSkill() {
	// cout << "Cache size: " << actionCache.size() << endl;
	if (actionCache[worldModel->getUNum()] == NULL) {
		cout << "initializing action cache for " << worldModel->getUNum() << endl;

		actionCache[worldModel->getUNum()] = SKILL_STAND;
	}

    int lock_fd;
    char lockfile[50];
    snprintf(lockfile, sizeof(lockfile), "/home/ryan/591/final/%d.lock", worldModel->getUNum());
    lock_fd = open(lockfile, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    char solnFile[50];
    snprintf(solnFile, sizeof(solnFile), "/home/ryan/591/final/%d.soln", worldModel->getUNum());

    // cout << lockfile << "," << lock_fd << endl;
    int unum = worldModel->getUNum();
    
    if (lock_fd > 0 || !(unum == 2 || unum == 3 || unum == 4)) { // need to update action

        int playerClosestToBall = -1;
        double closestDistanceToBall = 10000;

        for(int i = WO_TEAMMATE1; i < WO_TEAMMATE1+NUM_AGENTS; ++i) {
            VecPosition temp;
            int playerNum = i - WO_TEAMMATE1 + 1;
            if (worldModel->getUNum() == playerNum) {
                // This is us
                temp = worldModel->getMyPosition();
            } else {
                WorldObject* teammate = worldModel->getWorldObject( i );
                if (teammate->validPosition) {
                    temp = teammate->pos;
                } else {
                    continue;
                }
            }
            temp.setZ(0);

            double distanceToBall = temp.getDistanceTo(ball);
            if (distanceToBall < closestDistanceToBall) {
                playerClosestToBall = playerNum;
                closestDistanceToBall = distanceToBall;
            }
        }

        bool ourBall = playerClosestToBall <= 11;
        // cout << ourBall << endl;

        string position = getMyPosition(worldModel->getMyPosition().getX(), worldModel->getMyPosition().getY()); // todo remove


        if (ourBall) {

        	bool iHaveBall = playerClosestToBall == worldModel->getUNum();

            if (worldModel->getUNum() == 1) { // goalie
                return goToTarget(VecPosition(-15, 0, 0)); 
            } else if (worldModel->getUNum() <= 6) { // attacker
                
                /*
					wrapper.py arguments (assumes we're on offense)
					1 unum-me
					2 unum-ball carrier
					3 close values
					4 goal
					5 striker pos
					6 left pos
					7 right pos
                */

                string strikerPos = getMyPosition(worldModel->getWorldObject(2)->pos.getX(), worldModel->getWorldObject(2)->pos.getY());
                string leftPos = getMyPosition(worldModel->getWorldObject(3)->pos.getX(), worldModel->getWorldObject(3)->pos.getY());
                string rightPos = getMyPosition(worldModel->getWorldObject(4)->pos.getX(), worldModel->getWorldObject(4)->pos.getY());
                

                if (worldModel->getUNum() == 2) { // striker

                	int closeVal = 4;
                	if (close(worldModel->getMyPosition().getX(), worldModel->getWorldObject(3)->pos.getX(), worldModel->getMyPosition().getY(), worldModel->getWorldObject(3)->pos.getY())) {
                		closeVal = closeVal + 2;
                	}

                	if (close(worldModel->getMyPosition().getX(), worldModel->getWorldObject(4)->pos.getX(), worldModel->getMyPosition().getY(), worldModel->getWorldObject(4)->pos.getY())) {
                		closeVal = closeVal + 1;
                	}

                   // this will return a location, if you're close to the ball kick it towards the location, else walk towards the location

                	if (iHaveBall) {
                		char command[200];
	                    snprintf(command, sizeof(command), "python /home/ryan/591/hw3/planner/wrapper.py %d %d %d 'touchdown' %c%c %c%c %c%c 2>&1 &", worldModel->getUNum(), closeVal, playerClosestToBall, strikerPos[0], strikerPos[1], leftPos[0], leftPos[1], rightPos[0], rightPos[1]);
	                    // system(command);
	                    cout << command << endl;
                	} else {
                		char command[200];
	                    snprintf(command, sizeof(command), "python /home/ryan/591/hw3/planner/wrapper.py %d %d %d 'at striker EI' %c%c %c%c %c%c 2>&1 &", worldModel->getUNum(), closeVal, playerClosestToBall, strikerPos[0], strikerPos[1], leftPos[0], leftPos[1], rightPos[0], rightPos[1]);
	                    // system(command);
	                    cout << command << endl;
                	}
	                    


                } else if (worldModel->getUNum() == 3) { // left wing

                	int closeVal = 2;
                	if (close(worldModel->getMyPosition().getX(), worldModel->getWorldObject(2)->pos.getX(), worldModel->getMyPosition().getY(), worldModel->getWorldObject(2)->pos.getY())) {
                		closeVal = closeVal + 4;
                	}

                	if (close(worldModel->getMyPosition().getX(), worldModel->getWorldObject(4)->pos.getX(), worldModel->getMyPosition().getY(), worldModel->getWorldObject(4)->pos.getY())) {
                		closeVal = closeVal + 1;
                	}

                	if (iHaveBall) {
                		char command[200];
	                    snprintf(command, sizeof(command), "python /home/ryan/591/hw3/planner/wrapper.py %d %d %d 'touchdown' %c%c %c%c %c%c 2>&1 &", worldModel->getUNum(), closeVal, playerClosestToBall, strikerPos[0], strikerPos[1], leftPos[0], leftPos[1], rightPos[0], rightPos[1]);
	                    system(command);
	                    cout << command << endl;
                	} else {
                		char command[200];
	                    snprintf(command, sizeof(command), "python /home/ryan/591/hw3/planner/wrapper.py %d %d %d 'at left CI' %c%c %c%c %c%c 2>&1 &", worldModel->getUNum(), closeVal, playerClosestToBall, strikerPos[0], strikerPos[1], leftPos[0], leftPos[1], rightPos[0], rightPos[1]);
	                    system(command);
	                    cout << command << endl;
                	}
                    
                } else if (worldModel->getUNum() == 4) { // right wing
                    int closeVal = 1;
                	if (close(worldModel->getMyPosition().getX(), worldModel->getWorldObject(3)->pos.getX(), worldModel->getMyPosition().getY(), worldModel->getWorldObject(3)->pos.getY())) {
                		closeVal = closeVal + 2;
                	}

                	if (close(worldModel->getMyPosition().getX(), worldModel->getWorldObject(2)->pos.getX(), worldModel->getMyPosition().getY(), worldModel->getWorldObject(2)->pos.getY())) {
                		closeVal = closeVal + 4;
                	}

                	if (iHaveBall) {
                		char command[200];
	                    snprintf(command, sizeof(command), "python /home/ryan/591/hw3/planner/wrapper.py %d %d %d 'touchdown' %c%c %c%c %c%c 2>&1 &", worldModel->getUNum(), closeVal, playerClosestToBall, strikerPos[0], strikerPos[1], leftPos[0], leftPos[1], rightPos[0], rightPos[1]);
	                    system(command);
	                    cout << command << endl;
                	} else {
                		char command[200];
	                    snprintf(command, sizeof(command), "python /home/ryan/591/hw3/planner/wrapper.py %d %d %d 'at right HI' %c%c %c%c %c%c 2>&1 &", worldModel->getUNum(), closeVal, playerClosestToBall, strikerPos[0], strikerPos[1], leftPos[0], leftPos[1], rightPos[0], rightPos[1]);
	                    system(command);
	                    cout << command << endl;
                	}

                } else { // rest
                 	if (iHaveBall) {
                 		if (closestDistanceToBall < 0.5) { 
					        return kickBall(KICK_FORWARD, worldModel->getWorldObject(2)->pos); // kick towards striker
					    } else { // otherwise walk in the direction of the ball
					        return goToTarget(ball);
					    }

                 	}  
                }

                FILE* f;
	            if ((f = fopen(solnFile, "r"))) {
	                fseek(f, 0, SEEK_END);
	                size_t size = ftell(f);
	                char* cont = new char[size];
	                rewind(f);
	                fread(cont, sizeof(char), size, f);
	                cout << worldModel->getUNum() << ": file contents: " << cont << "| action command = " << cont[0] << endl;

	                /*
						0 index of file is action id [0:move {tile}|1:shoot|2:pass {member}|3:stand]
	                */

	                if (cont[0] == '0') {
	                	cout << unum << " move to " << cont[2] << cont[3] << endl;
	                } else if (cont[0] == '1') {
	                	cout << unum << " shoot" << endl;
	                } else if (cont[0] == '2') {
	                	cout << unum << " pass " << cont[2] << endl;
	                } else if (cont[0] == '3') {
	                	cout << unum << " stand " << endl;
	                }

	                double dest_xval = -HALF_FIELD_X + x_increment * (int(cont[1]) - 97);
	                double dest_yval = HALF_FIELD_Y + -y_increment * (int(cont[0]) - 97);
	                actionCache[unum] = goToTarget(VecPosition(dest_xval, dest_yval, 0));
	                // cout << unum << " to " << dest_xval << "," << dest_yval << endl;
	            } 

                return actionCache[unum]; 
            } else { // defender
                return goToTarget(VecPosition(-15, -10, 0)); 
            }
        } else { // we are not in posession of the ball and need to defend
        	// TODO Habib you can work on this block
            if (worldModel->getUNum() == 1) { // goalie
                return goToTarget(VecPosition(-15, 0, 0)); 
            } else if (worldModel->getUNum() <= 6) { // attacker
                
                if (worldModel->getUNum() == 2) { // striker

                } else if (worldModel->getUNum() == 3) { // left wing

                } else if (worldModel->getUNum() == 4) { // right wing

                } else { // rest

                }

                return goToTarget(VecPosition(-15, 10, 0)); 
            } else { // defender
                return goToTarget(VecPosition(-15, -10, 0)); 
            }
        }      

        

    } else {
    	return actionCache[unum];
    }
    

    return SKILL_STAND;

    double closest_i = 100;// used in the defender block to decide action
    double closest_j = 100;

    
    double i;
    double j;

    
    double closest_i_distance = 100;
    
    double closest_j_distance = 100;
    

    double xPos, yPos;
    if (worldModel->getUNum() == 1) {
        xPos = worldModel->getMyPosition().getX();
        yPos = worldModel->getMyPosition().getY();
    } else {
        xPos = worldModel->getWorldObject(1)->pos.getX();
        yPos = worldModel->getWorldObject(1)->pos.getY();
    }

    for (j = HALF_FIELD_Y; j >= -HALF_FIELD_Y; j-=y_increment) {
        if (closest_j_distance > abs(yPos - j)) {
            closest_j = j;
            closest_j_distance = abs(yPos - j);
        }
    }

    for (i = -HALF_FIELD_X; i <= HALF_FIELD_X; i+=x_increment) {    
        if (closest_i_distance > abs(xPos - i)) {
            closest_i = i;
            closest_i_distance = abs(xPos - i);
        }
    }

    int fd;
    if (worldModel->getUNum() == 1) { // the locks are removed when the spawned processes finish
        char* filename = "./attack.lock";
        fd = open(filename, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); // this is an atomic process
    } else {
        char* filename = "./defend.lock";
        fd = open(filename, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    }

    

    //# action0 - up
    //# action1 - left
    //# action2 - down
    //# action3 - right
    //# goal0 - left corner
    //# goal1 - left post
    //# goal2 - right post
    //# goal3 - right corner
        

    if (worldModel->getUNum() == 1 && fd > -1) { //attacker

        //cout << "attacker updating" << endl;

        if (goal == -1) {

            goal = rand() % 4;
            if (goal == 0) {
                offender_goal = "JA";
            } else if (goal == 1) {
                offender_goal = "FA";
            } else if (goal == 2) {
                offender_goal = "EA";
            } else {
                offender_goal = "AA";
            }

            attackerPlan = goToTarget(VecPosition(0, 0, 0)); // initial value
            cout << "offender chose goal " << offender_goal << endl;
        } else {
            FILE* f;
            if ((f = fopen("/home/ryan/591/hw3/attacker.soln", "r"))) {
                fseek(f, 0, SEEK_END);
                size_t size = ftell(f);
                char* cont = new char[size];
                rewind(f);
                fread(cont, sizeof(char), size, f);
                //cout << "file contents: " << cont[0] - 97 << cont[1] - 97 << endl; 


                double dest_xval = -HALF_FIELD_X + x_increment * (int(cont[1]) - 97);
                double dest_yval = HALF_FIELD_Y + -y_increment * (int(cont[0]) - 97);
                attackerPlan = goToTarget(VecPosition(dest_xval, dest_yval, 0));
                //cout << "attacker to " << dest_xval << "," << dest_yval << endl;
            } 
        }

        string position = getMyPosition(worldModel->getMyPosition().getX(), worldModel->getMyPosition().getY());
        char command[200];
        snprintf(command, sizeof(command), "cd /home/ryan/591/hw3/planner; python wrapper.py %c%c %c%c 2>&1 &" , position[0], position[1], offender_goal[0], offender_goal[1]);
        system(command);

        return attackerPlan;
    } else if (worldModel->getUNum() == 2 && fd > -1) { //defender

        //cout << "defender updating" << endl;

        if (timeSlice == 0) {
            defenderPlan = goToTarget(VecPosition(0, 0, 0));
        } else {
            FILE* f;
            if ((f = fopen("/home/ryan/591/hw3/defender.soln", "r"))) {
                fseek(f, 0, SEEK_END);
                size_t size = ftell(f);
                char* cont = new char[size];
                rewind(f);
                fread(cont, sizeof(char), size, f);
                //cout << "file contents: " << cont[0] << endl;

                if (cont[0] == '0') {
                    cout << "defender suspects left corner" << endl;
                    defenderPlan = goToTarget(VecPosition(-15, -10, 0));
                } else if (cont[0] == '1') {
                    cout << "defender suspects left post" << endl;
                    defenderPlan = goToTarget(VecPosition(-15, -1.1, 0));
                } else if (cont[0] == '2') {
                    cout << "defender suspects right post" << endl;
                    defenderPlan = goToTarget(VecPosition(-15, 1.1, 0));
                } else if (cont[0] == '3') {
                    cout << "defender suspects right corner" << endl;
                    defenderPlan = goToTarget(VecPosition(-15, 10, 0));
                }
            } 
        }

        timeSlice++;

        int action;

        if (closest_i > recent_i) {
            action = 3;//left
        } else if (closest_i < recent_i) {
            action = 1;//right
        } else if (closest_j > recent_j) {
            action = 0;//up
        } else { // closest_j < recent_j
            action = 2;//down
        }

        //cout << "(" << closest_i << "," << closest_j << ") " << endl;
        


        char command[200];
        snprintf(command, sizeof(command), "python -W ignore /home/ryan/591/hw3/dbn.py %d %d 2>&1 &", timeSlice, action); //dbn.py is resposible for removing the lock
        system(command);


        return defenderPlan;
    } else if (worldModel->getUNum() == 1) { // the lock was not released, use cahced commands
        //return attackerPlan;

        int shouldAttackerActRandomly = rand() % 10; // generates number [0,9]
        if (shouldAttackerActRandomly < 4) { //40% chance of acting according to plan
            return attackerPlan;
        } else {
            int x_offset = (rand() % 2) - 2; // [-1, 1]
            int y_offset = (rand() % 2) - 2; // [-1, 1]
            return goToTarget(VecPosition(worldModel->getMyPosition().getX() + x_offset, 
                worldModel->getMyPosition().getY() + y_offset, 
                0));
        }
    } else {
        return defenderPlan;
    }

}
