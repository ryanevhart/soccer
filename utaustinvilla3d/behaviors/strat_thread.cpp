#include "naobehavior.h"
#include "../rvdraw/rvdraw.h"

#include <cstdlib>
#include <unistd.h>  //Header file for sleep(). man 3 sleep for details. 
#include <pthread.h> 

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

double recent_i = -100;
double recent_j = -100;
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

WorldModel* globalWorldModel;
string offender_goal = "";
string offender_position = "AA";
double dest_xval = -15;
double dest_yval = 0;
int goal = -1;


void* updateAttacker(void *vargp) {

    if (goal == 0) {
        offender_goal = "JA";
    } else if (goal == 1) {
        offender_goal = "FA";
    } else if (goal == 2) {
        offender_goal = "EA";
    } else {
        offender_goal = "AA";
    }
    
   string current_pos = getMyPosition(globalWorldModel->getWorldObject(1)->pos.getX(), globalWorldModel->getWorldObject(1)->pos.getY());

   if (current_pos[0] != offender_position[0] || current_pos[1] != offender_position[1]) {
    offender_position = current_pos;

    char psBuffer[128];
    FILE *pPipe;
    // cout << "attacker deciding what to do" << endl;
    char command[200];
    snprintf(command, sizeof(command), "cd /home/ryan/591/hw3/planner/; python wrapper.py %c%c %c%c", current_pos[0], current_pos[1], offender_goal[0], offender_goal[1]);
    cout << command << endl;

    // TODO include action and time slice
    if((pPipe = popen(command, "r")) == NULL)
        exit(1);

    int doing_i = 0;

    
    while(fgets(psBuffer, 128, pPipe)) {
        if (doing_i == 0) {
            dest_yval = HALF_FIELD_Y + -y_increment * (int(psBuffer[0]) - 48);
            doing_i++;
        } else {
            dest_xval = -HALF_FIELD_X + x_increment * (int(psBuffer[0]) - 48);
            break;
        }
        
    }

    cout << "[" << psBuffer << "]" << "|" << dest_xval << "," << dest_yval << endl;
   }

    
}




SkillType attackerPlan;
SkillType defenderPlan;



SkillType NaoBehavior::selectSkill() {
    globalWorldModel = worldModel;
    
    
    double i;
    double j;

    double closest_i = 100;
    double closest_i_distance = 100;
    double closest_j = 100;
    double closest_j_distance = 100;
    
    for (j = HALF_FIELD_Y; j >= -HALF_FIELD_Y; j-=y_increment) {
        if (closest_j_distance > abs(worldModel->getWorldObject(1)->pos.getY() - j)) {
            closest_j = j;
            closest_j_distance = abs(worldModel->getWorldObject(1)->pos.getY() - j);
        }
    }

    for (i = -HALF_FIELD_X; i <= HALF_FIELD_X; i+=x_increment) {    
        if (closest_i_distance > abs(worldModel->getWorldObject(1)->pos.getX() - i)) {
            closest_i = i;
            closest_i_distance = abs(worldModel->getWorldObject(1)->pos.getX() - i);
        }
    }

    //# action0 - up
    //# action1 - left
    //# action2 - down
    //# action3 - right
    //# goal0 - left corner
    //# goal1 - left post
    //# goal2 - right post
    //# goal3 - right corner

    // cout << recent_i << "," << closest_i << "|" << recent_j << "," << closest_j << worldModel->getUNum() << endl;
    pthread_t thread_id; 
    pthread_create(&thread_id, NULL, updateAttacker, NULL); 

    if (recent_i != closest_i || recent_j != closest_j) {
        

        if (worldModel->getUNum() == 1) { //attacker
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

                cout << "offender chose goal " << offender_goal << endl;
            }



            

            
        } else { //defender
            recent_i = closest_i;
            recent_j = closest_j;
        }    


        
    }

    if (worldModel->getUNum() == 1) { //attacker
        // cout << "attacker going to " << dest_xval << "," <<dest_yval << endl;
        return goToTarget(VecPosition(dest_xval, dest_yval, 0));
    } else {
        return goToTarget(VecPosition(-15, -10, 0));;
    }
    
    // return goToTarget(VecPosition(dest_xval, dest_yval, 0));

    // My position and angle
    // cout << worldModel->getUNum() << ": " << worldModel->getMyPosition() << "\n";

    // prints coordinates of field
    // for (j = HALF_FIELD_Y; j >= -HALF_FIELD_Y; j-=y_increment) {
    //     for (i = -HALF_FIELD_X; i <= HALF_FIELD_X; i+=x_increment) {    
    //         // cout << "(" << i << "," << j << ") ";
    //      }
    //     // cout << endl;
    // }


    // Position of the ball
    //cout << worldModel->getBall() << "\n";

    // Example usage of the roboviz drawing system and RVSender in rvdraw.cc.
    // Agents draw the position of where they think the ball is
    // Also see example in naobahevior.cc for drawing agent position and
    // orientation.
    /*
    worldModel->getRVSender()->clear(); // erases drawings from previous cycle
    worldModel->getRVSender()->drawPoint("ball", ball.getX(), ball.getY(), 10.0f, RVSender::MAGENTA);
    */

    // ### Demo Behaviors ###

    // Walk in different directions
    //return goToTargetRelative(VecPosition(1,0,0), 0); // Forward
    //return goToTargetRelative(VecPosition(-1,0,0), 0); // Backward
    //return goToTargetRelative(VecPosition(0,1,0), 0); // Left
    //return goToTargetRelative(VecPosition(0,-1,0), 0); // Right
    //return goToTargetRelative(VecPosition(1,1,0), 0); // Diagonal
    //return goToTargetRelative(VecPosition(0,1,0), 90); // Turn counter-clockwise
    //return goToTargetRelative(VecPdosition(0,-1,0), -90); // Turn clockwise
    //return goToTargetRelative(VecPosition(1,0,0), 15); // Circle

    // Walk to the ball
    //return goToTarget(ball);

    // Turn in place to face ball
    /*double distance, angle;
    getTargetDistanceAndAngle(ball, distance, angle);
    if (abs(angle) > 10) {
      return goToTargetRelative(VecPosition(), angle);
    } else {
      return SKILL_STAND;
    }*/

    // Walk to ball while always facing forward
    //return goToTargetRelative(worldModel->g2l(ball), -worldModel->getMyAngDeg());

    // Dribble ball toward opponent's goal
    //return kickBall(KICK_DRIBBLE, VecPosition(HALF_FIELD_X, 0, 0));

    // Kick ball toward opponent's goal
    //return kickBall(KICK_FORWARD, VecPosition(HALF_FIELD_X, 0, 0)); // Basic kick
    //return kickBall(KICK_IK, VecPosition(HALF_FIELD_X, 0, 0)); // IK kick

    // Just stand in place
    //return SKILL_STAND;

    // Demo behavior where players form a rotating circle and kick the ball
    // back and forth
    
    // return demoKickingCircle();
}

// SkillType NaoBehavior::defenderAction() {
//     double x_increment = 2 * HALF_FIELD_X / 9;
//     double y_increment = 2 * HALF_FIELD_Y / 9;
    
//     double i;
//     double j;

//     double closest_i = 100;
//     double closest_i_distance = 100;
//     double closest_j = 100;
//     double closest_j_distance = 100;
    
//     for (j = HALF_FIELD_Y; j >= -HALF_FIELD_Y; j-=y_increment) {
//         if (closest_j_distance > abs(worldModel->getMyPosition().getY() - j)) {
//             closest_j = j;
//             closest_j_distance = abs(worldModel->getMyPosition().getY() - j);
//         }
//     }

//     for (i = -HALF_FIELD_X; i <= HALF_FIELD_X; i+=x_increment) {    
//         if (closest_i_distance > abs(worldModel->getMyPosition().getX() - i)) {
//             closest_i = i;
//             closest_i_distance = abs(worldModel->getMyPosition().getX() - i);
//         }
//     }

//     //# action0 - up
//     //# action1 - left
//     //# action2 - down
//     //# action3 - right
//     //# goal0 - left corner
//     //# goal1 - left post
//     //# goal2 - right post
//     //# goal3 - right corner
//     if (recent_i != closest_i || recent_j != closest_j) {
//         timeSlice++;

//         int action;

//         if (closest_i > recent_i) {
//             action = 3;//left
//         } else if (closest_i < recent_i) {
//             action = 1;//right
//         } else if (closest_j > recent_j) {
//             action = 0;//up
//         } else { // closest_j < recent_j
//             action = 2;//down
//         }

//         cout << "(" << closest_i << "," << closest_j << ") " << endl;
//         recent_i = closest_i;
//         recent_j = closest_j;

//         char psBuffer[128];
//         FILE *pPipe;

//         char command[100];
//         snprintf(command, sizeof(command), "python /home/ryan/591/hw3/dbn.py %d %d 2>&1", timeSlice, action);
//         // cout << command << endl;

//         // TODO include action and time slice
//         if((pPipe = popen(command, "r")) == NULL)
//             exit(1);

//         while(fgets(psBuffer, 128, pPipe)) {}

//         // printf(psBuffer); // holds goal percentages
//         // close(pPipe);

//         std::string delimiter = ",";

//         std::string output = string(psBuffer);

//         std::string goalNumber;
//         std::string goalPercent;

//         size_t pos = 0;
//         std::string token;
//         while ((pos = output.find(delimiter)) != std::string::npos) {
//             goalNumber = output.substr(0, pos);
//             output.erase(0, pos + delimiter.length());
//         }

//         goalPercent = output; // has a newline at the end

//         if (goalNumber.compare("0") == 0) {
//             cout << "left corner" << endl;
//             return goToTarget(VecPosition(-15, -10, 0));
//         } else if (goalNumber.compare("1") == 0) {
//             cout << "left post" << endl;
//             return goToTarget(VecPosition(-15, -1.1, 0));
//         } else if (goalNumber.compare("2") == 0) {
//             cout << "right post" << endl;
//             return goToTarget(VecPosition(-15, 1.1, 0));
//         } else if (goalNumber.compare("3") == 0) {
//             cout << "right corner" << endl;
//             return goToTarget(VecPosition(-15, 10, 0));
//         } else {
//             cout << "CONFUSED " << goalNumber << endl;
//             return NULL;
//         }
//     }
// }

/*
 * Demo behavior where players form a rotating circle and kick the ball
 * back and forth
 */
SkillType NaoBehavior::demoKickingCircle() {
    // Parameters for circle
    VecPosition center = VecPosition(-HALF_FIELD_X/2.0, 0, 0);
    double circleRadius = 5.0;
    double rotateRate = 2.5;

    // Find closest player to ball
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

    if (playerClosestToBall == worldModel->getUNum()) {
        // Have closest player kick the ball toward the center
        return kickBall(KICK_FORWARD, center);
    } else {
        // Move to circle position around center and face the center
        VecPosition localCenter = worldModel->g2l(center);
        SIM::AngDeg localCenterAngle = atan2Deg(localCenter.getY(), localCenter.getX());

        // Our desired target position on the circle
        // Compute target based on uniform number, rotate rate, and time
        VecPosition target = center + VecPosition(circleRadius,0,0).rotateAboutZ(360.0/(NUM_AGENTS-1)*(worldModel->getUNum()-(worldModel->getUNum() > playerClosestToBall ? 1 : 0)) + worldModel->getTime()*rotateRate);

        // Adjust target to not be too close to teammates or the ball
        target = collisionAvoidance(true /*teammate*/, false/*opponent*/, true/*ball*/, 1/*proximity thresh*/, .5/*collision thresh*/, target, true/*keepDistance*/);

        if (me.getDistanceTo(target) < .25 && abs(localCenterAngle) <= 10) {
            // Close enough to desired position and orientation so just stand
            return SKILL_STAND;
        } else if (me.getDistanceTo(target) < .5) {
            // Close to desired position so start turning to face center
            return goToTargetRelative(worldModel->g2l(target), localCenterAngle);
        } else {
            // Move toward target location
            return goToTarget(target);
        }
    }
}


