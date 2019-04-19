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

SkillType cache2;
SkillType cache3;
SkillType cache4;

SkillType NaoBehavior::selectSkill() {
	// cout << "Cache size: " << actionCache.size() << endl;
	if (cache2 == NULL && worldModel->getUNum() == 2) {
		cout << "initializing action cache for " << worldModel->getUNum() << endl;
		cache2 = SKILL_STAND;

	} else if (cache3 == NULL && worldModel->getUNum() == 3) {
		cout << "initializing action cache for " << worldModel->getUNum() << endl;
		cache3 = SKILL_STAND;

	} else if (cache4 == NULL && worldModel->getUNum() == 4) {
		cout << "initializing action cache for " << worldModel->getUNum() << endl;
		cache4 = SKILL_STAND;
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

                

                // TODO just do a read from the solution file for every tick
                
                

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
	                    system(command);
	                    cout << command << endl;
                	} else {
                		char command[200];
	                    snprintf(command, sizeof(command), "python /home/ryan/591/hw3/planner/wrapper.py %d %d %d 'at striker EI' %c%c %c%c %c%c 2>&1 &", worldModel->getUNum(), closeVal, playerClosestToBall, strikerPos[0], strikerPos[1], leftPos[0], leftPos[1], rightPos[0], rightPos[1]);
	                    system(command);
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
    }
	

	FILE* f;
    if ((f = fopen(solnFile, "r"))) {
        fseek(f, 0, SEEK_END);
        size_t size = ftell(f);
        char* cont = new char[size];
        rewind(f);
        fread(cont, sizeof(char), size, f);
        // cout << worldModel->getUNum() << ": file contents: " << cont << "| action command = " << cont[0] << endl;

        /*
			0 index of file is action id [0:move {tile}|1:shoot|2:pass {member}|3:stand]
        */

        SkillType updatedAction;
        

        if (cont[0] == '0') {
        	double dest_xval = -HALF_FIELD_X + x_increment * (int(cont[3]) - 97);
        	double dest_yval = HALF_FIELD_Y + -y_increment * (int(cont[2]) - 97);
        	updatedAction = goToTarget(VecPosition(dest_xval, dest_yval, 0));
        	cout << unum << " move to " << cont[2] << cont[3] << " - (" << dest_xval << "," << dest_yval << ")" << endl;
        } else if (cont[0] == '1') {
        	cout << unum << " shoot" << endl;
        	updatedAction = kickBall(KICK_FORWARD, VecPosition(15, 0, 0));
        } else if (cont[0] == '2') {
        	cout << unum << " pass (NEED TO FIX) " << cont[2] << endl;
        	updatedAction = SKILL_STAND; // TODO
        }

        return updatedAction;
    } else {
    	cout << "couldn't find solution file for agent " << unum << endl;
    	return SKILL_STAND;
    }
}