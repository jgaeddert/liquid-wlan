#!/usr/bin/env python3
'''demonstrate fractional sample delay object'''
import argparse, sys
import liquid as dsp
import matplotlib.pyplot as plt
import numpy as np
sys.path.extend(['.','..'])
import liquidwlan as wlan

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('-nodisplay', action='store_true', help='disable display')
args = p.parse_args()

print('liquid-wlan python version:',wlan.__version__)

# create frame
fg = wlan.framegen()
buf = fg.execute()
print(buf.shape)

# estimate spectrum and plot results
psd = dsp.spgram(nfft=600,wlen=400,delay=10)
psd.execute(buf)

# get spectrum plot and display
print(psd)
Sxx,f = psd.get_psd()
print('Sxx:',Sxx.shape,'f:',f.shape)

fig,ax = plt.subplots(1,figsize=(8,8))
ax.plot(f,Sxx)
ax.set_xlabel('Frequency [f/Fs]')
ax.set_ylabel('PSD [dB]')
ax.grid(True, zorder=5)
plt.show()

