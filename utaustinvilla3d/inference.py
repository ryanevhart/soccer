from pgmpy.models import BayesianModel
from pgmpy.factors.discrete import TabularCPD
import subprocess
import random
import os
import sys
import fileinput


posBall = sys.argv[1]
leftOpp = sys.argv[2]
rightOpp = sys.argv[3]
unum = int(sys.argv[4])


vBC = 0
vBR = 0
vBL = 0
vLO = 0
vRO = 0

if posBall == "0":
	vBC = 1
	vBL = 0
	vBR = 0
elif posBall == "1":
	vBC = 0
	vBL = 1
	vBR = 0
elif posBall == "2":
	vBC = 0
	vBL = 0
	vBR = 1

if leftOpp >= "3":
	vLO = 1
	vRO = 0
elif rightOpp >= "3":
	vRO = 1
	vLO = 0

# Define the model structure (also see the instructions)
inference_model = BayesianModel([('LO', 'Goal'),
                              ('RO', 'Goal'),
                              ('BL', 'Goal'),
                              ('BR', 'Goal'),
                              ('BC', 'Goal')])

# now defining the parameters.
cpd_lo = TabularCPD(variable='LO', variable_card=2,
                      values=[[0.5], [0.5]])
cpd_ro = TabularCPD(variable='RO', variable_card=2,
                       values=[[0.5], [0.5]])
cpd_bl = TabularCPD(variable='BL', variable_card=2,
                       values=[[0.5], [0.5]])
cpd_br = TabularCPD(variable='BR', variable_card=2,
                       values=[[0.5], [0.5]])
cpd_bc = TabularCPD(variable='BC', variable_card=2,
                       values=[[0.5], [0.5]])
g_cpd = TabularCPD(variable='Goal', variable_card=3, values=
						[[1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,1,1,0,1,1,1,1,1,1,1,1,1,1,1],
						 [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0],
						 [0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]],
                        evidence=['LO','RO', 'BL', 'BR', 'BC'],
                        evidence_card=[2, 2, 2, 2, 2])

# Associating the parameters with the model structure.
inference_model.add_cpds(cpd_lo, cpd_ro, cpd_bl, cpd_br, cpd_bc, g_cpd)

# Doing exact inference using Variable Elimination
from pgmpy.inference import VariableElimination
inferOutput = VariableElimination(inference_model)

maxProb = 0
maxIndex = 0

for i in range(3):
    probability = inferOutput.query(variables=['Goal'], evidence={'LO': vLO, 'RO': vRO, 'BL': vBL, 'BR': vBR, 'BC': vBC})['Goal'].values[i]
    if probability > maxProb:
        maxProb = probability
        maxIndex = i

with open('../defender.soln', 'w') as file:
	file.write(str(maxIndex))
	file.close()

lockFile = "../%d.lock" % (unum)
time.sleep(10) # this python file being kicked off makes all the agents fall, sleep the agents have time to actually excecute their plan
os.remove(lockFile)