from random import randint

def getFitProcess(i, arr):
  minVal = min([ p[i] for p in arr])
  fitArr = list(filter(lambda x: x[i] == minVal, arr))
  if len(fitArr) > 1:
    return getFitProcess(i - 1, fitArr)
  return fitArr[0]

def ShortestJobFirst(Np, intensity, burstTimeRange):
  # processer generating
  maxArriveGap = int(1 / intensity * 2)
  arriveGaps = [ randint(0, maxArriveGap) for n in range(Np)]
  arriveTimeStep = 0
  processes = list()
  for i in range(Np):
    arriveTimeStep += arriveGaps[i]
    process = [ i + 1, arriveTimeStep, randint(*burstTimeRange) ]
    processes.append(process)

  # calculating complete, turn-around and wait time
  processed = 0
  sumWaitTime = 0
  completed = list()
  arriveScale = min([ p[1] for p in processes ])
  while len(completed) < Np:
    fitArrived = list(filter(lambda x: x[1] <= arriveScale and x[0] not in completed, processes))
    # if idle scale
    if not len(fitArrived):
      arriveScale += 1
      continue
    #searching the best fit process
    fitProcess = getFitProcess(2, fitArrived)
    n, arriveTime, burstTime = fitProcess
    arriveScale += burstTime
    completeTime = arriveScale
    turnAroundTime = completeTime - arriveTime
    waitTime = turnAroundTime - burstTime
    sumWaitTime += waitTime
    processed += burstTime
    fitProcess += [ completeTime, turnAroundTime, waitTime ]
    completed.append(n)

  idle = (arriveScale - processed) / arriveScale * 100
  avgWaitTime = sumWaitTime / Np
  #intensity calculation
  arriveTimeSorted = [ p[1] for p in processes ]
  arriveTimeSorted.sort()
  intermediateTimeList = [ arriveTimeSorted[i + 1] - arriveTimeSorted[i] for i in range(Np - 1) ]
  intensity = 1 / (sum(intermediateTimeList) / len(intermediateTimeList))
  descriptor = dict(
    intensity = intensity,
    avgWaitTime = avgWaitTime,
    sumWaitTime = sumWaitTime,
    idle = idle
  )
  return processes, descriptor
