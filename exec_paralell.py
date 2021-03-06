from multiprocessing import Process, Lock
import psutil
import subprocess
import sys
import os

## how to run## python exec_paralell.py FILENAME STARTINGFRAME ENDINGFRAME

# dont touch

FILENAMEPREFIX = sys.argv[1]
STARTINGFRAME = int(sys.argv[2])
ENDINGFRAME = int(sys.argv[3])
active_jobs = []

def Worker(l, framenum):
    #l.acquire()
    proc =  subprocess.Popen(["./singleframe.out", FILENAMEPREFIX, str(framenum).zfill(4)])
    proc.wait()
    print("\nCompleted Job # " + str(framenum))
    #l.release()

if __name__ == '__main__':
    lock = Lock()
    for procnum in range(ENDINGFRAME - STARTINGFRAME):
        active_jobs.append(Process(target = Worker, args = (lock, procnum)).start())
        print("\nStarted Job # " + str(procnum))
    exit(1)
