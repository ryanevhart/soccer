#include "naobehavior.h"
#include "../rvdraw/rvdraw.h"

#include <cstdlib>
#include <fcntl.h>
#include <vector>
#include <iostream>

extern int agentBodyType;

int pastDefender = -1;
string formation = "0";

/*
 * Real game beaming.
 * Filling params x y angle
 */
void NaoBehavior::beam( double& beamX, double& beamY, double& beamAngle ) {
    beamX = -HALF_FIELD_X + worldModel->getUNum();
    beamY = 0;
    beamAngle = 0;
}


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

    // cout << "(" << xpos << "," << ypos << ") is " << retVal << endl;
    
    return retVal;
}


int close(double x1, double y1, double x2, double y2) {
    int diff = abs(x1 - x2) + abs(y1 - y2);
    if (diff <= 7) {
        return 1;
    } else {
        return 0;
    }
}

SkillType NaoBehavior::selectSkill() {
    /*
    ifstream ifs ("E:/One\ Drive/OneDrive\ -\ Arizona\ State\ University/School/workspace/final/utaustinvilla3d-group/soccer/%d.lock");
    if (ifs.is_open()) {
        // print file:
        char c = ifs.get();
        while (ifs.good()) {
          std::cout << c;
          c = ifs.get();
        }
    }
    else {
        // show message:
        std::cout << "Error opening file";
    }
    */  

    int unum = worldModel->getUNum();
    int lock_fd = -1;

    if (unum == 2 || unum == 3 || unum == 4 || unum == 7) {
        char lockfile[50];
        snprintf(lockfile, sizeof(lockfile), "../%d.lock", worldModel->getUNum());
        lock_fd = open(lockfile, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    }
        

    char solnFile[50];
    snprintf(solnFile, sizeof(solnFile), "../%d.soln", worldModel->getUNum());

    int playerClosestToBall = -1;
    double closestDistanceToBall = 10000;

    for(int i = 1; i <= 11; ++i) {
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

    bool imClosestToBall = playerClosestToBall == worldModel->getUNum();
    bool ourBall = ball.getX() >= -3;
    VecPosition point = VecPosition(-4, 0, 0);

    // if this bool is true, the lock file for the corresponding agent will not be deleted, so we don't care about lock_fd
    bool dontReadCommandFromFile = !(unum == 2 || unum == 3 || unum == 4 || unum == 7); //2,3,4 are the only agents that read from file rn
    
    if (lock_fd > 0 || dontReadCommandFromFile) { // need to update action

        if (ourBall) {
            if (worldModel->getUNum() == 1) { // goalie
                if (imClosestToBall) {
                    if (closestDistanceToBall < 0.5) { 
                        return kickBall(KICK_FORWARD, VecPosition(15, 0, 0));
                    } else { // otherwise walk in the direction of the ball
                        return goToTarget(ball);
                    }
                } else {
                    return goToTarget(VecPosition(-15, 0, 0));
                }
                 
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

                    if (imClosestToBall) {
                        char command[200];
                        snprintf(command, sizeof(command), "python ../planner/wrapper.py %d %d %d 'touchdown' %c%c %c%c %c%c 2>&1 &", worldModel->getUNum(), playerClosestToBall, closeVal, strikerPos[0], strikerPos[1], leftPos[0], leftPos[1], rightPos[0], rightPos[1]);
                        system(command);
                        cout << command << endl;
                    } else {
                        char command[200];
                        snprintf(command, sizeof(command), "python ../planner/wrapper.py %d %d %d 'at striker EI' %c%c %c%c %c%c 2>&1 &", worldModel->getUNum(), playerClosestToBall, closeVal, strikerPos[0], strikerPos[1], leftPos[0], leftPos[1], rightPos[0], rightPos[1]);
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

                    if (imClosestToBall) {
                        char command[200];
                        snprintf(command, sizeof(command), "python ../planner/wrapper.py %d %d %d 'touchdown' %c%c %c%c %c%c 2>&1 &", worldModel->getUNum(), playerClosestToBall, closeVal, strikerPos[0], strikerPos[1], leftPos[0], leftPos[1], rightPos[0], rightPos[1]);
                        system(command);
                        cout << command << endl;
                    } else {
                        char command[200];
                        snprintf(command, sizeof(command), "python ../planner/wrapper.py %d %d %d 'at left CI' %c%c %c%c %c%c 2>&1 &", worldModel->getUNum(), playerClosestToBall, closeVal, strikerPos[0], strikerPos[1], leftPos[0], leftPos[1], rightPos[0], rightPos[1]);
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

                    if (imClosestToBall) {
                        char command[200];
                        snprintf(command, sizeof(command), "python ../planner/wrapper.py %d %d %d 'touchdown' %c%c %c%c %c%c 2>&1 &", worldModel->getUNum(), playerClosestToBall, closeVal, strikerPos[0], strikerPos[1], leftPos[0], leftPos[1], rightPos[0], rightPos[1]);
                        system(command);
                        cout << command << endl;
                    } else {
                        char command[200];
                        snprintf(command, sizeof(command), "python ../planner/wrapper.py %d %d %d 'at right HI' %c%c %c%c %c%c 2>&1 &", worldModel->getUNum(), playerClosestToBall, closeVal, strikerPos[0], strikerPos[1], leftPos[0], leftPos[1], rightPos[0], rightPos[1]);
                        system(command);
                        cout << command << endl;
                    }

                } else { // assistant attackers
                    if (imClosestToBall) {
                        if (closestDistanceToBall < 0.5) { 
                            return kickBall(KICK_FORWARD, worldModel->getWorldObject(2)->pos); // kick towards striker
                        } else { // otherwise walk in the direction of the ball
                            return goToTarget(ball);
                        }
                    } else {
                        if (unum == 5) {
                            return goToTarget(VecPosition(5, 5, 0));
                        } else if (unum == 6) {
                            return goToTarget(VecPosition(5, -5, 0));
                        }
                    }
                }
            } else { // defender during attack [7,11]

                if (imClosestToBall) {
                    if (closestDistanceToBall < 0.5) { 
                        return kickBall(KICK_FORWARD, worldModel->getWorldObject(2)->pos); // kick towards striker
                    } else { // otherwise walk in the direction of the ball
                        return goToTarget(ball);
                    }
                } else {
                    if (unum == 7) {
                        return goToTarget(VecPosition(0, 0, 0)); 
                    } else if (unum == 8) {
                        return goToTarget(VecPosition(0, 5, 0)); 
                    } else if (unum == 9) {
                        return goToTarget(VecPosition(0, -5, 0)); 
                    } else if (unum == 10) {
                        return goToTarget(VecPosition(0, 10, 0)); 
                    } else {
                        return goToTarget(VecPosition(0, -10, 0)); 
                    }
                }
            }
        } else { // we are not in posession of the ball and need to defend
            // TODO Habib you can work on this block
            // Find closest teammate to ball
            int teamClosestToBall = -1;
            double teamClosestDistanceToBall = 10000;
            for(int i = WO_TEAMMATE1; i <= 11; ++i) {
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

                double teamDistanceToBall = temp.getDistanceTo(ball);

                if (teamDistanceToBall < teamClosestDistanceToBall && i != pastDefender) {
                    teamClosestToBall = playerNum;
                    teamClosestDistanceToBall = teamDistanceToBall;

                    //If the opponent passes the defender then disregard the defender passed when selecting closest to ball.
                    if(ball.getX() - temp.getX() < -.5 && temp.getX() > -13) {
                        pastDefender = playerNum;
                        teamClosestDistanceToBall = 10000;
                    }
                }
            }


            //Goalie is closest to ball
            if (worldModel->getUNum() == 1){
                VecPosition goaliePos = worldModel->getMyPosition();
                double distGoalieToBall = goaliePos.getDistanceTo(ball);

                if (teamClosestToBall == 1) {
                    if (distGoalieToBall < 0.5) { 
                        return kickBall(KICK_DRIBBLE, VecPosition(15, 0, 0));
                    } else { // otherwise walk in the direction of the ball
                        return goToTarget(ball);
                    }
                } else {
                    double distance, angle;
                    getTargetDistanceAndAngle(ball, distance, angle);
                    double distGoalieToOurGoal = goaliePos.getDistanceTo(VecPosition(-14.3, 0, 0));
                    if (abs(angle) > 10 and distGoalieToBall < 1) {
                        return goToTargetRelative(VecPosition(), angle);
                    } else {
                        return goToTarget(VecPosition(-14.3, 0, 0));
                    }
                }
            }
            else if (worldModel->getUNum() <= 6) { // attacker
                if (worldModel->getUNum() == 2) { // striker
                    return goToTarget(VecPosition(0, 0, 0));
                } else if (worldModel->getUNum() == 3) { // right mid wing
                    return goToTarget(VecPosition(0, -5, 0));
                } else if (worldModel->getUNum() == 4) { //left mid wing
                    return goToTarget(VecPosition(0, 5, 0));
                } else if (worldModel->getUNum() == 5) { //right far wing
                    return goToTarget(VecPosition(0, -10, 0));
                } else if (worldModel->getUNum() == 6) { //left far wing
                    return goToTarget(VecPosition(0, 10, 0));
                } 
            } else {
                if (teamClosestToBall == worldModel->getUNum()) {
                    double allClosestDistanceToBall = 10000;
                    double allPlayerClosestToBall = -1;

                    //If opponent is near the ball, dribble towards goal. If not, kick towards goal
                    for (int i = 12; i <= 22; ++i)
                    {
                        VecPosition allTemp;
                        WorldObject* thisAgent = worldModel->getWorldObject( i );
                        if (thisAgent->validPosition)
                        {
                            allTemp = thisAgent->pos;
                        }

                        double allDistanceToBall = allTemp.getDistanceTo(ball);

                        if(allDistanceToBall < 1.5) {
                            //May need to change destination
                            return kickBall(KICK_DRIBBLE, VecPosition(5, 0, 0));
                        }   
                    }

                    //May need to change destination
                    return kickBall(KICK_FORWARD, VecPosition(5,0,0));

                }
                else {
                    // rest
                    // Move to specified positions around point and face point
                    VecPosition localPoint = worldModel->g2l(point);
                    SIM::AngDeg localPointAngle = atan2Deg(localPoint.getY(), localPoint.getX());

                    //If the ball moves behind our front position, move formation backwards
                    if(ball.getX() <= point.getX()){
                        point = VecPosition(ball.getX() - 1, point.getY(), 0);
                    }

                    VecPosition target = point;
                    string ballPos = "0";

                    //Where is the position of the ball?
                    //0 for center, 1 for left, 2 for right
                    if(ball.getY() >= 5)
                        ballPos = "1";
                    else if (ball.getY() <= -5)
                        ballPos = "2";
                    else
                        ballPos = "0";

                    int leftSideOpp = 0;
                    int rightSideOpp = 0;

                    //How many agents are on each side?
                    for (int i = 11; i <= 22; ++i)
                    {
                        VecPosition allTemp;
                        WorldObject* thisAgent = worldModel->getWorldObject( i );
                        if (thisAgent->validPosition)
                        {
                            allTemp = thisAgent->pos;
                        }

                        if (allTemp.getY() >= 5 && allTemp.getX() <= -4)
                            leftSideOpp++;
                        else if (allTemp.getY() <= -5 && allTemp.getX() <= -4)
                            rightSideOpp++;
                    }

                    switch(worldModel->getUNum())
                    {   
                        //Top Center Defender
                        case 7:
                            if(formation == "0")
                                target = point; //Center Formation
                            else if (formation == "1")
                                target = point - VecPosition(1, -3, 0); //Left Formation
                            else if (formation == "2")
                                target = point - VecPosition(1, 3, 0); //Right Formation

                            if(ball.getX() < -9) //Failsafe Formation
                                target = VecPosition(-12, 0, 0);
                            break;

                        //Middle Left Defender
                        case 8:
                            if(formation == "0")
                                target = point - VecPosition(2, -2, 0); // Center Formation
                            else if (formation == "1")
                                target = point - VecPosition(3, -3, 0); //Left Formation
                            else if (formation == "2")
                                target = point - VecPosition(2, 0, 0); //Right Formation

                            if(ball.getX() < -9) //Failsafe Formation
                                target = VecPosition(-13, 1, 0);
                            break;

                        //Middle Right Defender
                        case 9:
                            if(formation == "0")
                                target = point - VecPosition(2, 2, 0); //Center Formation
                            else if (formation == "1")
                                target = point - VecPosition(2, 0, 0); //Left Formation
                            else if (formation == "2")
                                target = point - VecPosition(3, 3, 0); //Right Formation

                            if(ball.getX() < -9)//Failsafe Formation
                                target = VecPosition(-13, -1, 0);
                            break;

                        //Back Left Defender
                        case 10:
                            if(formation == "0")
                                target = point - VecPosition(4, -3, 0); //Center Formation
                            else if (formation == "1")
                                target = point - VecPosition(5, -3, 0); //Left Formation
                            else if (formation == "2")
                                target = point - VecPosition(4, 0, 0); //Right Formation

                            if(ball.getX() < -9) //Failsafe Formation
                                target = VecPosition(-14, 2, 0);
                            break;

                        //Back Right Defender
                        case 11:
                            if(formation == "0")
                                target = point - VecPosition(4, 3, 0); //Center Formation
                            else if (formation == "1")
                                target = point - VecPosition(4, 0, 0);  //Left Formation
                            else if (formation == "2")
                                target = point - VecPosition(5, 3, 0); //Right Formation

                            if(ball.getX() < -9) //Failsafe Formation
                                target = VecPosition(-14, -2, 0);
                            break;
                        default:
                            target = point;
                            break;

                    }

                    // Adjust target to not be too close to teammates
                    target = collisionAvoidance(true /*teammate*/, false/*opponent*/, false/*ball*/, 1/*proximity thresh*/, .25/*collision thresh*/, target, true/*keepDistance*/);

                    if (lock_fd > 0 && unum == 7) {
                        if (me.getDistanceTo(target) < .25 && abs(localPointAngle) <= 10) {
                            // Close enough to desired position and orientation so just stand

                            char command[200];
                            snprintf(command, sizeof(command), "python inference.py %c %c %c &" , ballPos, leftSideOpp, rightSideOpp);
                            system(command);

                            ifstream inferFileOut("../defender.soln");
                            inferFileOut >> formation;

                            return SKILL_STAND;
                        } else if (me.getDistanceTo(target) < .5) {
                            // Close to desired position so start turning to face center

                            char command[200];
                            snprintf(command, sizeof(command), "python inference.py %c %c %c &" , ballPos, leftSideOpp, rightSideOpp);
                            system(command);
            
                            ifstream inferFileOut("../defender.soln");
                            inferFileOut >> formation;

                            return goToTargetRelative(worldModel->g2l(target), localPointAngle);
                        } else {
                            // Move toward target location
                            return goToTarget(target);
                        } 
                    }
                                  
                }
            }
        }      
        // Have closest player kick the ball toward the center
        return kickBall(KICK_FORWARD, VecPosition(HALF_FIELD_X,0,0));
    }
    

    if (imClosestToBall && closestDistanceToBall > 0.5) {
        return goToTarget(ball);
    }

    if (unum == 2 || unum == 3 || unum == 4) {
        FILE* f;
        if ((f = fopen(solnFile, "r"))) {
            fseek(f, 0, SEEK_END);
            size_t size = ftell(f);
            char* cont = new char[size];
            rewind(f);
            fread(cont, sizeof(char), size, f);
            fclose(f);

            /*
                0 index of file is action id [0:move {tile}|1:shoot|2:pass {member}|3:stand]
            */

            SkillType updatedAction;
            

            if (cont[0] == '0') {
                double dest_xval = -HALF_FIELD_X + x_increment * (int(cont[3]) - 97);
                double dest_yval = HALF_FIELD_Y + -y_increment * (int(cont[2]) - 97);
                if (imClosestToBall) {
                    updatedAction = kickBall(KICK_FORWARD, VecPosition(15, 0, 0));
                } else {
                    updatedAction = goToTarget(VecPosition(dest_xval, dest_yval, 0));   
                }
                
                // cout << unum << " move to " << cont[2] << cont[3] << " - (" << dest_xval << "," << dest_yval << ")" << endl;
            } else if (cont[0] == '1') {
                cout << unum << " shoot" << endl;
                updatedAction = kickBall(KICK_FORWARD, VecPosition(15, 0, 0));
            } else if (cont[0] == '2') {
                

                int destIndex = -1;
                if (cont[2] == 's') {
                    destIndex = 2;
                } else if (cont[2] == 'l') {
                    destIndex = 3;
                } else if (cont[2] == 'r') {
                    destIndex = 4;
                } else {
                    cout << "unrecognized pass destination: " << cont[2] << endl;
                }

                cout << unum << " pass " << cont[2] << ":" << worldModel->getMyPosition() << "|" << worldModel->getWorldObject(destIndex)->pos <<endl;

                if (closestDistanceToBall <= 0.5) {
                    if (destIndex > 0) {
                        if (worldModel->getMyPosition().getY() > worldModel->getWorldObject(destIndex)->pos.getY()) {
                            updatedAction = kickBall(KICK_FORWARD, VecPosition(15, 0, 0));
                        } else {
                            updatedAction = kickBall(KICK_FORWARD, worldModel->getWorldObject(destIndex)->pos);
                        }
                    }
                    
                } else {
                    return goToTarget(ball);
                }
                    
            } else {
                updatedAction = SKILL_STAND;
                cout << "couldn't identify command from file, contents - " << cont << endl;
            }

            return updatedAction;
        } else {
            //cout << "couldn't find solution file for agent " << unum << "|" << solnFile << endl;
            return SKILL_STAND;
        }
    }
        
}