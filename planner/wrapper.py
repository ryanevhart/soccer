import sys
import subprocess
import os
import time

# example: python wrapper.py 2 4 4 'AT RIGHT EI' AA BA CA

# TODO need to take the unum as argv[1] to do %d.soln and %d.lock

""" can take argv[3]
Close truth table
S L R
F F F 0
F F T 1
F T F 2
F T T 3 --
T F F 4
T F T 5 --
T T F 6 --
T T T 7 --
"""

unum = int(sys.argv[1])

ballCarrierString = ""
ballCarrier = int(sys.argv[2])
if ballCarrier == 2:
	ballCarrierString = "STRIKER"
elif ballCarrier == 3:
	ballCarrierString = "LEFT"
elif ballCarrier == 4:
	ballCarrierString = "RIGHT"
else:
	ballCarrierString = "NONE"


closeValueString = ""
closeValue = int(sys.argv[3])
if closeValue & 3 == 3:
	closeValueString += "\n(CLOSE LEFT RIGHT)\n(CLOSE RIGHT LEFT)"

if closeValue & 5 == 5:
	closeValueString += "\n(CLOSE STRIKER RIGHT)\n(CLOSE RIGHT STRIKER)"

if closeValue & 6 == 6:
	closeValueString += "\n(CLOSE STRIKER LEFT)\n(CLOSE LEFT STRIKER)"


goal = sys.argv[4] # can take values [touchdown | AT ?p ?loc]


problem_contents = """(define (problem attacker)
	(:domain SOCCER)

	(:objects
		STRIKER LEFT RIGHT NONE
		AA AB AC AD AE AF AG AH AI AJ BA BB BC BD BE BF BG BH BI BJ CA CB CC CD CE CF CG CH CI CJ DA DB DC DD DE DF DG DH DI DJ EA EB EC ED EE EF EG EH EI EJ FA FB FC FD FE FF FG FH FI FJ GA GB GC GD GE GF GG GH GI GJ HA HB HC HD HE HF HG HH HI HJ IA IB IC ID IE IF IG IH II IJ JA JB JC JD JE JF JG JH JI JJ
	)

	(:init
		(AT STRIKER %s)
		(AT LEFT %s)
		(AT RIGHT %s)
		(HASBALL %s)%s
		(CONNECTED AA AB)
		(CONNECTED AA BA)
		(CONNECTED AB AA)
		(CONNECTED AB AC)
		(CONNECTED AB BB)
		(CONNECTED AC AB)
		(CONNECTED AC AD)
		(CONNECTED AC BC)
		(CONNECTED AD AC)
		(CONNECTED AD AE)
		(CONNECTED AD BD)
		(CONNECTED AE AD)
		(CONNECTED AE AF)
		(CONNECTED AE BE)
		(CONNECTED AF AE)
		(CONNECTED AF AG)
		(CONNECTED AF BF)
		(CONNECTED AG AF)
		(CONNECTED AG AH)
		(CONNECTED AG BG)
		(CONNECTED AH AG)
		(CONNECTED AH AI)
		(CONNECTED AH BH)
		(CONNECTED AI AH)
		(CONNECTED AI AJ)
		(CONNECTED AI BI)
		(CONNECTED AJ AI)
		(CONNECTED AJ BJ)
		(CONNECTED BA BB)
		(CONNECTED BA AA)
		(CONNECTED BA CA)
		(CONNECTED BB BA)
		(CONNECTED BB BC)
		(CONNECTED BB AB)
		(CONNECTED BB CB)
		(CONNECTED BC BB)
		(CONNECTED BC BD)
		(CONNECTED BC AC)
		(CONNECTED BC CC)
		(CONNECTED BD BC)
		(CONNECTED BD BE)
		(CONNECTED BD AD)
		(CONNECTED BD CD)
		(CONNECTED BE BD)
		(CONNECTED BE BF)
		(CONNECTED BE AE)
		(CONNECTED BE CE)
		(CONNECTED BF BE)
		(CONNECTED BF BG)
		(CONNECTED BF AF)
		(CONNECTED BF CF)
		(CONNECTED BG BF)
		(CONNECTED BG BH)
		(CONNECTED BG AG)
		(CONNECTED BG CG)
		(CONNECTED BH BG)
		(CONNECTED BH BI)
		(CONNECTED BH AH)
		(CONNECTED BH CH)
		(CONNECTED BI BH)
		(CONNECTED BI BJ)
		(CONNECTED BI AI)
		(CONNECTED BI CI)
		(CONNECTED BJ BI)
		(CONNECTED BJ AJ)
		(CONNECTED BJ CJ)
		(CONNECTED CA CB)
		(CONNECTED CA BA)
		(CONNECTED CA DA)
		(CONNECTED CB CA)
		(CONNECTED CB CC)
		(CONNECTED CB BB)
		(CONNECTED CB DB)
		(CONNECTED CC CB)
		(CONNECTED CC CD)
		(CONNECTED CC BC)
		(CONNECTED CC DC)
		(CONNECTED CD CC)
		(CONNECTED CD CE)
		(CONNECTED CD BD)
		(CONNECTED CD DD)
		(CONNECTED CE CD)
		(CONNECTED CE CF)
		(CONNECTED CE BE)
		(CONNECTED CE DE)
		(CONNECTED CF CE)
		(CONNECTED CF CG)
		(CONNECTED CF BF)
		(CONNECTED CF DF)
		(CONNECTED CG CF)
		(CONNECTED CG CH)
		(CONNECTED CG BG)
		(CONNECTED CG DG)
		(CONNECTED CH CG)
		(CONNECTED CH CI)
		(CONNECTED CH BH)
		(CONNECTED CH DH)
		(CONNECTED CI CH)
		(CONNECTED CI CJ)
		(CONNECTED CI BI)
		(CONNECTED CI DI)
		(CONNECTED CJ CI)
		(CONNECTED CJ BJ)
		(CONNECTED CJ DJ)
		(CONNECTED DA DB)
		(CONNECTED DA CA)
		(CONNECTED DA EA)
		(CONNECTED DB DA)
		(CONNECTED DB DC)
		(CONNECTED DB CB)
		(CONNECTED DB EB)
		(CONNECTED DC DB)
		(CONNECTED DC DD)
		(CONNECTED DC CC)
		(CONNECTED DC EC)
		(CONNECTED DD DC)
		(CONNECTED DD DE)
		(CONNECTED DD CD)
		(CONNECTED DD ED)
		(CONNECTED DE DD)
		(CONNECTED DE DF)
		(CONNECTED DE CE)
		(CONNECTED DE EE)
		(CONNECTED DF DE)
		(CONNECTED DF DG)
		(CONNECTED DF CF)
		(CONNECTED DF EF)
		(CONNECTED DG DF)
		(CONNECTED DG DH)
		(CONNECTED DG CG)
		(CONNECTED DG EG)
		(CONNECTED DH DG)
		(CONNECTED DH DI)
		(CONNECTED DH CH)
		(CONNECTED DH EH)
		(CONNECTED DI DH)
		(CONNECTED DI DJ)
		(CONNECTED DI CI)
		(CONNECTED DI EI)
		(CONNECTED DJ DI)
		(CONNECTED DJ CJ)
		(CONNECTED DJ EJ)
		(CONNECTED EA EB)
		(CONNECTED EA DA)
		(CONNECTED EA FA)
		(CONNECTED EB EA)
		(CONNECTED EB EC)
		(CONNECTED EB DB)
		(CONNECTED EB FB)
		(CONNECTED EC EB)
		(CONNECTED EC ED)
		(CONNECTED EC DC)
		(CONNECTED EC FC)
		(CONNECTED ED EC)
		(CONNECTED ED EE)
		(CONNECTED ED DD)
		(CONNECTED ED FD)
		(CONNECTED EE ED)
		(CONNECTED EE EF)
		(CONNECTED EE DE)
		(CONNECTED EE FE)
		(CONNECTED EF EE)
		(CONNECTED EF EG)
		(CONNECTED EF DF)
		(CONNECTED EF FF)
		(CONNECTED EG EF)
		(CONNECTED EG EH)
		(CONNECTED EG DG)
		(CONNECTED EG FG)
		(CONNECTED EH EG)
		(CONNECTED EH EI)
		(CONNECTED EH DH)
		(CONNECTED EH FH)
		(CONNECTED EI EH)
		(CONNECTED EI EJ)
		(CONNECTED EI DI)
		(CONNECTED EI FI)
		(CONNECTED EJ EI)
		(CONNECTED EJ DJ)
		(CONNECTED EJ FJ)
		(CONNECTED FA FB)
		(CONNECTED FA EA)
		(CONNECTED FA GA)
		(CONNECTED FB FA)
		(CONNECTED FB FC)
		(CONNECTED FB EB)
		(CONNECTED FB GB)
		(CONNECTED FC FB)
		(CONNECTED FC FD)
		(CONNECTED FC EC)
		(CONNECTED FC GC)
		(CONNECTED FD FC)
		(CONNECTED FD FE)
		(CONNECTED FD ED)
		(CONNECTED FD GD)
		(CONNECTED FE FD)
		(CONNECTED FE FF)
		(CONNECTED FE EE)
		(CONNECTED FE GE)
		(CONNECTED FF FE)
		(CONNECTED FF FG)
		(CONNECTED FF EF)
		(CONNECTED FF GF)
		(CONNECTED FG FF)
		(CONNECTED FG FH)
		(CONNECTED FG EG)
		(CONNECTED FG GG)
		(CONNECTED FH FG)
		(CONNECTED FH FI)
		(CONNECTED FH EH)
		(CONNECTED FH GH)
		(CONNECTED FI FH)
		(CONNECTED FI FJ)
		(CONNECTED FI EI)
		(CONNECTED FI GI)
		(CONNECTED FJ FI)
		(CONNECTED FJ EJ)
		(CONNECTED FJ GJ)
		(CONNECTED GA GB)
		(CONNECTED GA FA)
		(CONNECTED GA HA)
		(CONNECTED GB GA)
		(CONNECTED GB GC)
		(CONNECTED GB FB)
		(CONNECTED GB HB)
		(CONNECTED GC GB)
		(CONNECTED GC GD)
		(CONNECTED GC FC)
		(CONNECTED GC HC)
		(CONNECTED GD GC)
		(CONNECTED GD GE)
		(CONNECTED GD FD)
		(CONNECTED GD HD)
		(CONNECTED GE GD)
		(CONNECTED GE GF)
		(CONNECTED GE FE)
		(CONNECTED GE HE)
		(CONNECTED GF GE)
		(CONNECTED GF GG)
		(CONNECTED GF FF)
		(CONNECTED GF HF)
		(CONNECTED GG GF)
		(CONNECTED GG GH)
		(CONNECTED GG FG)
		(CONNECTED GG HG)
		(CONNECTED GH GG)
		(CONNECTED GH GI)
		(CONNECTED GH FH)
		(CONNECTED GH HH)
		(CONNECTED GI GH)
		(CONNECTED GI GJ)
		(CONNECTED GI FI)
		(CONNECTED GI HI)
		(CONNECTED GJ GI)
		(CONNECTED GJ FJ)
		(CONNECTED GJ HJ)
		(CONNECTED HA HB)
		(CONNECTED HA GA)
		(CONNECTED HA IA)
		(CONNECTED HB HA)
		(CONNECTED HB HC)
		(CONNECTED HB GB)
		(CONNECTED HB IB)
		(CONNECTED HC HB)
		(CONNECTED HC HD)
		(CONNECTED HC GC)
		(CONNECTED HC IC)
		(CONNECTED HD HC)
		(CONNECTED HD HE)
		(CONNECTED HD GD)
		(CONNECTED HD ID)
		(CONNECTED HE HD)
		(CONNECTED HE HF)
		(CONNECTED HE GE)
		(CONNECTED HE IE)
		(CONNECTED HF HE)
		(CONNECTED HF HG)
		(CONNECTED HF GF)
		(CONNECTED HF IF)
		(CONNECTED HG HF)
		(CONNECTED HG HH)
		(CONNECTED HG GG)
		(CONNECTED HG IG)
		(CONNECTED HH HG)
		(CONNECTED HH HI)
		(CONNECTED HH GH)
		(CONNECTED HH IH)
		(CONNECTED HI HH)
		(CONNECTED HI HJ)
		(CONNECTED HI GI)
		(CONNECTED HI II)
		(CONNECTED HJ HI)
		(CONNECTED HJ GJ)
		(CONNECTED HJ IJ)
		(CONNECTED IA IB)
		(CONNECTED IA HA)
		(CONNECTED IA JA)
		(CONNECTED IB IA)
		(CONNECTED IB IC)
		(CONNECTED IB HB)
		(CONNECTED IB JB)
		(CONNECTED IC IB)
		(CONNECTED IC ID)
		(CONNECTED IC HC)
		(CONNECTED IC JC)
		(CONNECTED ID IC)
		(CONNECTED ID IE)
		(CONNECTED ID HD)
		(CONNECTED ID JD)
		(CONNECTED IE ID)
		(CONNECTED IE IF)
		(CONNECTED IE HE)
		(CONNECTED IE JE)
		(CONNECTED IF IE)
		(CONNECTED IF IG)
		(CONNECTED IF HF)
		(CONNECTED IF JF)
		(CONNECTED IG IF)
		(CONNECTED IG IH)
		(CONNECTED IG HG)
		(CONNECTED IG JG)
		(CONNECTED IH IG)
		(CONNECTED IH II)
		(CONNECTED IH HH)
		(CONNECTED IH JH)
		(CONNECTED II IH)
		(CONNECTED II IJ)
		(CONNECTED II HI)
		(CONNECTED II JI)
		(CONNECTED IJ II)
		(CONNECTED IJ HJ)
		(CONNECTED IJ JJ)
		(CONNECTED JA JB)
		(CONNECTED JA IA)
		(CONNECTED JB JA)
		(CONNECTED JB JC)
		(CONNECTED JB IB)
		(CONNECTED JC JB)
		(CONNECTED JC JD)
		(CONNECTED JC IC)
		(CONNECTED JD JC)
		(CONNECTED JD JE)
		(CONNECTED JD ID)
		(CONNECTED JE JD)
		(CONNECTED JE JF)
		(CONNECTED JE IE)
		(CONNECTED JF JE)
		(CONNECTED JF JG)
		(CONNECTED JF IF)
		(CONNECTED JG JF)
		(CONNECTED JG JH)
		(CONNECTED JG IG)
		(CONNECTED JH JG)
		(CONNECTED JH JI)
		(CONNECTED JH IH)
		(CONNECTED JI JH)
		(CONNECTED JI JJ)
		(CONNECTED JI II)
		(CONNECTED JJ JI)
		(CONNECTED JJ IJ))

	(:goal 
		(%s))
)
""" % (sys.argv[5], sys.argv[6], sys.argv[7], ballCarrierString, closeValueString, goal)

# print problem_contents

root = '../planner/' # TODO dont hardcode root
problem_file_loc = root + "problem_file.domain"

f = open(problem_file_loc, "w")
f.write(problem_contents)
f.close()



proc = subprocess.Popen(['python', root + 'fast-downward.py', root + 'domain.pddl', problem_file_loc, '--search', 'astar(lmcut())'], stdout=subprocess.PIPE)

out, err = proc.communicate()
arr = out.splitlines()

# print out

solnFile = "../%d.soln" % (unum)
lockFile = "../%d.lock" % (unum)


alreadyWrote = 0

for line in arr:
	writeAction = False

	if "(1)" in line:
		if unum == 2 and "striker" in line:
			writeAction = True
		elif unum == 3 and "left" in line:
			writeAction = True
		elif unum == 4 and "right" in line:
			writeAction = True
	
	if writeAction:
		if alreadyWrote > 3: # look a couple move steps ahead, updating for a single move is too frequent and causes constant falling
			break
		if alreadyWrote > 0 and "move" not in line: # look a couple move steps ahead, updating for a single move is too frequent and causes constant falling
			break
		with open(solnFile, 'w') as file:	
			if "move" in line:
				fields = line.split(" ")
				file.write("0 " + fields[3])
			elif "shoot" in line:
				file.write("1")
				break
			elif "pass" in line:
				fields = line.split(" ")
				file.write("2 " + fields[2])
				break
			else:
				print 'UNRECOGNIZED ACTION: ' + line
			file.close()			
		alreadyWrote = alreadyWrote + 1
		

time.sleep(10) # this python file being kicked off makes all the agents fall, sleep the agents have time to actually excecute their plan
os.remove(lockFile)
#python /home/ryan/591/hw3/planner/fast-downward.py /home/ryan/591/hw3/planner/domain.pddl /home/ryan/591/hw3/planner/right_corner_problem.pddl --search "astar(lmcut())"