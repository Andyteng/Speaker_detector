# -*- coding: utf-8 -*-
"""
Created on Thu Jan 12 13:43:56 2017

@author: xiang teng
"""


import numpy as np
import sys
import cv2 
import os
import subprocess
import shutil


if len(sys.argv) < 1:
    print('Error')
    sys.exit('Usage: extract_DT_global subject_global')

subject_global=sys.argv[1] 

#Day1_CAM12_part1 = [2,5,9,10,12,13,14,16,17,21,22,23,25,31]
#Day1_CAM12_part1 = [18,28]
Day1_CAM12_part1 = [31]

#map global id to day and subjet per day

##extract dense trajectories
#THIS NEEDS TO BE CHANGED TO FIT USER
traj_dir='/home/andy/Dropbox/TUD/Year3/Social_Signal_Processing/code'
seq_dir='/home/andy/Dropbox/Graduation_Project/MatchMakersStudentEdition/video_mingle'

for x in Day1_CAM12_part1:
    out_dir_dt='/home/andy/Dropbox/Graduation_Project/MILES_gestures/DT_'+subject_global+'/subject'+str(x)
    print('Extracting trajectories, to be saved at '+out_dir_dt)

    if not os.path.exists(out_dir_dt):
        os.makedirs(out_dir_dt)

    os.chdir(traj_dir)
    subprocess.call(['./release/DenseTrack', seq_dir+'/'+str(subject_global)+'.mp4',out_dir_dt,str(x),'-L','20'])
