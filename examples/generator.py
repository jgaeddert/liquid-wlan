#!/usr/bin/env python3
'''demonstrate fractional sample delay object'''
import argparse, sys
import liquid as dsp
import matplotlib.pyplot as plt
import numpy as np
sys.path.extend(['.','..'])
import liquidwlan as wlan

def normalize_spectrum(Sxx,pct=15,L=0.05,scale=0.015,n0_est=None):
    '''normalize spectrum data'''
    if n0_est is None:
        n0_est = estimate_n0(Sxx,pct,L)
    det = (Sxx - n0_est) * scale
    return np.uint8(np.minimum(255,np.maximum(0,np.round(255*det))))

def estimate_n0(Sxx,pct,L):
    '''return noise floor estimate from PSD waterfall along frequency, taking into account neighboring bins'''
    num_cols = Sxx.shape[1]
    if type(L)==float:
        L = int(num_cols*L/2)+1
    L = np.minimum(L,num_cols//4)
    L = np.maximum(L,0)
    if L<=0: # zero width; just take percentile along columns
        return np.percentile(Sxx,pct,axis=0)
    #print('Sxx.shape', Sxx.shape, 'L', L)
    c_idx = np.arange(2*L+1) - L # [-L, -L+1, -L+2, ..., 0, ... L-2, L-1, L]
    n0_est = np.zeros((num_cols,),dtype=np.single)
    for c in range(num_cols):
        n0_est[c] = np.percentile(Sxx.take(c_idx + c,axis=1,mode='wrap'), pct)
    return n0_est

def export_image(im, filename, grayscale=False, imsize=(640,640), dpi=96):
    '''export image with no border and matching image size'''
    print('im', im.shape, im.dtype, np.min(im), np.max(im))
    fig = plt.figure()
    fig.set_size_inches((imsize[0]/dpi, imsize[1]/dpi))
    ax = plt.Axes(fig, [0., 0., 1., 1.])
    ax.set_axis_off()
    fig.add_axes(ax)
    plt.set_cmap('Greys_r' if grayscale else 'viridis')
    ax.imshow(np.flipud(im), aspect='equal')
    fig.savefig(filename, dpi=dpi)
    plt.close(fig)

def main():
    # options
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('-display', action='store_true', help='enable display')
    p.add_argument('-nfft', type=int, default=640, help='FFT size')
    p.add_argument('-gray', action='store_true', help='set gray scale image')
    p.add_argument('-noise_floor', type=float, default=-60, help='noise floor')
    args = p.parse_args()

    # fixed values to ensure proper sizing
    #nfft = 640; time = 640; delay = 244
    num_samples, psd_delay = int(5e6), 244
    num_samples, psd_delay = int(1e6)-100, 195

    # create buffer of samples
    nstd = 10**(args.noise_floor/20)
    buf = np.random.randn(2*num_samples).astype(np.single).view(np.csingle)*0.707*nstd

    # design interpolator
    interp = dsp.firinterp(5, 12, 60)
    interp.scale = np.sqrt(0.2) # normalize to unity PSD

    # create frame generator
    fg = wlan.framegen()

    # loop over relative center frequencies
    labels = []
    for f0 in (2412,2437,2462,):
        fc = (f0 - 2450)/100
        # starting index of signal
        i0 = int(np.random.uniform(1e3,4e3))
        while True:
            if i0 >= len(buf):
                break

            num_bytes = int(np.random.uniform(20,1500))
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
            delay = int(np.random.uniform(8e3,20e3))
            if np.random.choice((True,False)): delay *= 10
            i0 = i1 + delay

    # compute spectrum waterfall
    psd = dsp.spwaterfall(nfft=640,time=640,delay=psd_delay)
    print('processing buffer...')
    psd.execute(buf)

    # get spectrum plot and display
    Sxx,t,f = psd.get_psd()
    Sxx = Sxx.T
    print('Sxx:',Sxx.shape,'t:',t.shape,'f:',f.shape)
    print('normalizing spectrum...')
    im = normalize_spectrum(Sxx, n0_est=args.noise_floor-3)
    print('saving image...')
    export_image(im, 'wifi_generator.png', grayscale=args.gray)
    print('done')

    fig,ax = plt.subplots(1,figsize=(8,8))
    ax.pcolormesh(f*100 + 2450,t/100e3,Sxx,shading='auto')
    ax.set_xlabel('Frequency [MHz]')
    ax.set_ylabel('Time [ms]')
    
    if args.display:
        plt.show()

if __name__=='__main__':
    main()

