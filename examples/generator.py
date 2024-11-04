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
p.add_argument('-len',  type=int, default=int(5e6), help='buffer length')
p.add_argument('-nfft', type=int, default=640, help='FFT size')
p.add_argument('-noise_floor', type=float, default=-60, help='noise floor')
args = p.parse_args()

# create buffer of samples
nstd = 10**(args.noise_floor/20)
buf = np.random.randn(2*args.len).astype(np.single).view(np.csingle)*0.707*nstd

# design interpolator
interp = dsp.firinterp(5, 12, 60)
interp.scale = np.sqrt(0.2) # normalize to unity PSD

# create frame generator
fg = wlan.framegen()

# loop over relative center frequencies
for fc in (-0.2,0):
    # starting index of signal
    i0 = int(np.random.uniform(1000,4000))
    while True:
        if i0 >= len(buf):
            break

        num_bytes = int(np.random.uniform(200,1500))
        datarate  = int(np.random.choice((0,2,3,4,5,6,7,)))
        v = fg.execute(num_bytes, datarate)
        print('v',len(v))
        y = interp.execute(np.concatenate((v,np.zeros((100,),dtype=np.csingle))))
        n = len(y)
        i1 = i0 + n
        if i1 > len(buf):
            # truncate signal
            i1 = len(buf)
            n = i1 - i0
            y = y[:n]

        # add signal
        snr = np.random.uniform(-3,20)
        gain = 10**((args.noise_floor+snr)/20)
        t = np.arange(n)
        buf[i0:i1] += gain * y * np.exp(2j*np.pi*fc*t)

        # add delay
        i0 = i1 + int(np.random.uniform(10000,40000))

# estimate spectrum and plot results
psd = dsp.spgram(nfft=args.nfft)
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

# generate random signals in noise
psd = dsp.spwaterfall(nfft=args.nfft,time=args.nfft,delay=10)
psd.execute(buf)

# get spectrum plot and display
Sxx,t,f = psd.get_psd()
print('Sxx:',Sxx.shape,'t:',t.shape,'f:',f.shape)

fig,ax = plt.subplots(1,figsize=(8,8))
ax.pcolormesh(f,t,Sxx.T,shading='auto')
ax.set_xlabel('Frequency [f/Fs]')
ax.set_ylabel('Time [ms]')
if not args.nodisplay:
    plt.show()

