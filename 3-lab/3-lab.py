import numpy
import matplotlib.pyplot as plt
from ShortestJobFirst import ShortestJobFirst

# Shortest Job First Algoritm

# plots data 1, 2
# input arguments
Np = 500
burstTimeRange = 3, 10
intensities = 0.1, 1, 0.1
intensitiesList = numpy.arange(*intensities).tolist()
# calculating
avgsTime, intensities, idles = list(), list(), list()
for intensity in intensitiesList:
  _, desc = ShortestJobFirst(Np, intensity, burstTimeRange)
  avgsTime.append(desc["sumWaitTime"])
  intensities.append(desc["intensity"])  
  idles.append(desc["idle"])

# plots data 3
# input arguments
completeStep = 100
completePer =  0
processes, _ = ShortestJobFirst(500, 0.07, burstTimeRange)
# calculating
completeFinal = processes[-1][3]
distribution = list()
while completePer < completeFinal:
  completeNext = completePer + completeStep
  completed = 0
  for p in processes:
    if p[3] > completePer and p[3] <= completeNext:
      completed += 1
  distribution.append((completeNext, completed))
  completePer = completeNext

procRange = [ avgT for _, avgT in distribution ]
avgsTime2 = [ pr for pr, _ in distribution ]

# PLOTTING
plt.plot(intensities, avgsTime)
plt.xlabel("Intensity")
plt.ylabel("Average wait time")
plt.title("AvgTime(Intensity)")
plt.figure()

plt.plot(intensities, idles)
plt.xlabel("Intensity")
plt.ylabel("Idle percentage")
plt.title("Idle percentage(Intensity)")
plt.figure()


plt.plot(avgsTime2, procRange)
plt.xlabel("Average wait time")
plt.ylabel("Processes amount")
plt.title("Processes amount(Average wait time)")

plt.show()
