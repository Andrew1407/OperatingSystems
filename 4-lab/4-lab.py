import numpy as np

getMinSumRow = lambda matrix: matrix.sum(axis=1).argmin()

def generateMatrix(size):
  matrix = np.random.randint(0, 2, (size, size))
  [ zeroRows ] = np.where(~matrix.any(axis=1))
  [ zeroCols ] = np.where(~matrix.any(axis=0))
  if len(zeroRows) or len(zeroCols):
    return generateMatrix(size)
  return matrix

def getMaxSumCol(matrix):
  [ columnsIdx ] = np.where(matrix[0] != 0)
  maxSumCol = [ matrix[:, col].sum() for col in columnsIdx ]
  idx = np.array(maxSumCol).argmax()
  return columnsIdx[idx]

def runPlanner(matrix):
  for i in range(len(matrix)):
    subMatrix = matrix[i:, i:]
    subRowIdx = i + getMinSumRow(subMatrix)

    rowMatrix_i = A[i].copy()
    matrix[i] = matrix[subRowIdx].copy()
    matrix[subRowIdx] = rowMatrix_i

    subColIdx = i + getMaxSumCol(subMatrix)

    colMatrix_i = matrix[:, i].copy()
    matrix[:, i] = matrix[:, subColIdx].copy()
    matrix[:, subColIdx] = colMatrix_i

    # iteration result logging 
    # print(f"Process (row) {subRowIdx} is sent to resource (column) {subColIdx}")


A_SIZE = 10
A = generateMatrix(A_SIZE)
print("Generated matrix (input):\n", A, "\n")
runPlanner(A)
print("\nResult (output):\n", A)
