#!/usr/bin/env python3
'''generate and synchronize to frame'''
import argparse, sys
import liquid as dsp
import matplotlib.pyplot as plt
import numpy as np
import json
sys.path.extend(['.','..','./build'])
import liquid_wlan as wlan

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('-nodisplay', action='store_true', help='disable display')
args = p.parse_args()

print('liquid-wlan python version:',wlan.__version__)

# create frame
fg = wlan.framegen()
buf = fg.execute(datarate=6,length=1500)
print(buf.shape)

# user-defined callback function
def callback(context,payload,stats):
    print('frame detected!')
    print(json.dumps(stats['rxvector']))
    context.update(stats)

info = {}
fs = wlan.framesync(callback,info)
fs.execute(buf)

print('')
print('final statistics')
print(json.dumps(fs.framedatastats,indent=2))

# plot symbols
if 'framesyms' in info:
    fig,ax = plt.subplots(1,figsize=(8,8))
    ax.plot(np.real(info['framesyms']),np.imag(info['framesyms']),'.')
    ax.set_xlabel('Real')
    ax.set_ylabel('Imag')
    ax.set(xlim=(-1.5,1.5),ylim=(-1.5,1.5))
    ax.grid(True, zorder=5)
    plt.show()

