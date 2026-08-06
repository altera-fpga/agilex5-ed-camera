#!/usr/bin/env python3

# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/

#####################################################################################
# A collection of ISP and noise management libraries
#####################################################################################

import os
import numpy as np
import colour
from colour_demosaicing import demosaicing_CFA_Bayer_Malvar2004 as demosaic_malvar
from colour_demosaicing import demosaicing_CFA_Bayer_Menon2007 as demosaic_menon
from colour_demosaicing import demosaicing_CFA_Bayer_bilinear as demosaic_bilinear
from scipy.optimize import curve_fit
import ctypes
from scipy import LowLevelCallable
from scipy.ndimage import generic_filter
import time
import cv2
from PIL import Image

# Saturates the input within the range [low,high]
def saturate(x, low, high):
    x[x > high] = high
    x[x < low] = low
    return x

# Artificial temporal noise generator
def noiser(
    img_ref,
    exposure_time_multiplier=1.0, # a factor of exposure time (not actual time in seconds)
    quantum_efficiency=1.0,       # quantum efficiency of the sensor
    dark_noise=0.0,               # base dark noise parameter of the sensor
    analog_gain=1.0,              # analog gain factor (not real gain, but a scaler)
    digital_gain=1.0,             # digital gain factor
    bits=8,                       # number of quantization bits
    black_level=0,                # black level pedestal
    saturate_return=True,         # saturate the noisy image before returning
    random_state=np.random.RandomState(seed=42)):

    # Shot noise: part of the photon capture process that is a poisson distribution
    img = random_state.poisson(img_ref.astype(float) * float(exposure_time_multiplier))
    # Quantum efficiency: photon to electron conversion probability
    img = img * quantum_efficiency
    # Dark noise: false electrons generated (or lost) as a thermal process
    img += random_state.normal(scale=dark_noise, size=img.shape)
    # Gains: analog and digital gains with quantization
    img = ((img * float(analog_gain)).astype(int) * digital_gain).astype(int)
    # Black level: offset ensuring that negative part of the dark noise is retained
    img += black_level
    # Saturation
    if saturate_return:
        saturate(img, 0, int(2 ** bits - 1))
    
    return img

# Temporal noise function
def sqrt(x, dark_noise, k):
    return np.sqrt(k * x + dark_noise**2)

# Estimates temporal noise function from a dark frame and two frames from the same static scene
def noise_estimator(
    dark_frame,            # a dark frame captured with the same settings as the scene
    scene_frames,          # a couple of frames from a static scene with a wide histogram
    bits,                  # number of quantization bits
    intensity_cutoff=None, # reliable intensity range to be used in curve fitting
    bin_neighborhood=3     # the number of neighboring intensities to bin
    ):

    # if a tuple is given then skip default values
    if intensity_cutoff is None:
        # default range is 0 to 70% of the dynamic range
        intensity_cutoff = (0, 0.7 * 2**bits-1)
    elif np.isscalar(intensity_cutoff):
        # if a scalar is provided it is the upper range
        intensity_cutoff = (0, intensity_cutoff)

    # dark noise and black level are estimated from the dark frame
    dark_noise_est = np.std(dark_frame)
    black_level_est = int(np.round(np.mean(dark_frame)))

    # average of the two scene frames is used as a proxy for the expected value
    scene_avg = (scene_frames[0] + scene_frames[1]) / 2
    # first element of the noise curve is the dark noise estimated from the dark captures accurately
    noise_curve = np.zeros(2**bits)
    noise_curve[black_level_est] = dark_noise_est
    for i in range(black_level_est+1, 2**bits):#, int(max(1, 2**(bits-10)))):
        # Bin a small range of intensities to avoid scene histogram inequality noise
        bin_mask = np.logical_and(scene_avg <= i+bin_neighborhood, scene_avg >= i-bin_neighborhood)
        if np.count_nonzero(bin_mask) > 50:
            # calculate standard deviations per bin
            noise_curve[i] = np.std(scene_frames[0][bin_mask] - scene_frames[1][bin_mask]) / np.sqrt(2)
        else:
            # too few samples, set the bin to NaN to indicate
            noise_curve[i] = np.NaN

    # NaN values are meant to be zero
    noise_curve = np.nan_to_num(noise_curve)

    # fit a square root curve to the noise estimations
    x_data = np.arange(0, intensity_cutoff[1] - intensity_cutoff[0])
    y_data = noise_curve[intensity_cutoff[0]:intensity_cutoff[1]]
    # give equal weight to all noise sample points, except for the first point (dark noise)
    # which is known to be more accurate
    y_sigma = np.ones_like(y_data)
    y_sigma[0] = 0.25
    # fit the curve
    noise_params, _ = curve_fit(
        f=sqrt, xdata=x_data, ydata=y_data, sigma=y_sigma, p0=(dark_noise_est, 1.0),
        bounds=((dark_noise_est, 0.0), (np.inf, np.inf)))

    return noise_params, black_level_est, noise_curve

# Gaussian function
def gaussian(src, sigma):
    dst = np.exp(-1/2 * (src / sigma)**2)
    return dst

# Calculate weight kernel of a bilateral filter
def calc_kernel(src, sigmaColor, sigmaSpace):
    k = src.shape[0]
    r = k//2
    center = src[r]
    intensity_kernel = gaussian(center - src, sigmaColor)

    grid = np.mgrid[-r:r+1]
    spatial_kernel = gaussian(grid, sigmaSpace)
    if len(src.shape) == 2:
        spatial_kernel = spatial_kernel[:, None]
        spatial_kernel = np.tile(spatial_kernel, (1, 3))

    return intensity_kernel * spatial_kernel

# Bilateral filter 
def bilateral_filter_c(src, diameter, combined_gain, dark_noise_with_gain, stride=1, c_model='bfilt', recompile=True, bps=8):
    if recompile:
        from subprocess import run
        current_dir = os.path.dirname(os.path.abspath(__file__))
        run(['rm', '-f', current_dir+'/c/'+c_model+'.so'])
        run(['gcc', '-shared', '-Wall', '-Wextra', '-fpic', current_dir+'/c/'+c_model+'.c', '-o', current_dir+'/c/'+c_model+'.so'])
    # Load dynamic library
    if 'clib' in locals():
        print("Info: Dynamic library of the bilateral filter is already loaded. If modifications " +
              "were made to the C source code they might not be picked up until a clean rerun.")
    clib = ctypes.cdll.LoadLibrary(current_dir+'/c/'+c_model+'.so')
    # Define types expected by SciPy's low level callable pramework
    clib.bfilt.restype = ctypes.c_int
    clib.bfilt.argtypes = (
        ctypes.POINTER(ctypes.c_double),
        ctypes.c_long,
        ctypes.POINTER(ctypes.c_double),
        ctypes.c_void_p,
    )
    # User data is used to pass metadata
    radius = diameter // 2
    if c_model == "bfilt_bit_precise":
        grid = np.mgrid[1:radius+1] # option: x2 to tail off the distribution more agressively to preserve details
        spatial_distance_lut = list(0.5 * (grid / radius)**2)
        intensity_range = np.arange(0, 2**min(bps, 10), 2**max(0, bps - 10))
        intensity_range_lut = list(np.sqrt(1.0 / (2.0 * (intensity_range * combined_gain + dark_noise_with_gain**2))))
    else:
        grid = np.mgrid[-radius:radius+1] # option: x2 to tail off the distribution more agressively to preserve details
        spatial_distance_lut = list(np.exp(-0.5 * (grid / radius)**2))
        intensity_range_lut = []
    user_data_py = [diameter, radius, combined_gain, dark_noise_with_gain, bps]
    user_data_py = user_data_py + spatial_distance_lut + intensity_range_lut
    user_data_c = ctypes.cast((ctypes.c_double * len(user_data_py))(*user_data_py), ctypes.c_void_p)
    # Wrap up the low level function
    bfilt = LowLevelCallable(clib.bfilt, user_data_c)
    # Create separabel kernel masks of 'diameter' number of elements for every 'stride' number of pixels.
    filter_mask = np.zeros((stride*(diameter-1)+1,1))
    filter_mask[::stride] = 1
    filter_mask_t = filter_mask.T
    if len(src.shape) == 3:
        filter_mask = filter_mask[:,:,None]
        filter_mask_t = filter_mask_t[:,:,None]
    # Spatial denoising (vertical followed by horizontal)
    dst = generic_filter(src, bfilt, footprint=filter_mask, mode='constant')
    dst = generic_filter(dst, bfilt, footprint=filter_mask_t, mode='constant')
    return dst

# RGB bilateral filter
def bilateral_filter(src, d, sigmaColor, sigmaSpace, borderType=None, is_seperable=True):
    dst = np.zeros_like(src, dtype=np.float32)
    tmp = np.zeros_like(src, dtype=np.float32)
    r = d//2

    if is_seperable:
        for i in range(r, src.shape[0]-r):
            for j in range(0, src.shape[1]):
                src_local = src[i-r:i+r+1, j, :].astype(np.float32)
                kernel = calc_kernel(src_local, sigmaColor[i,j,:], sigmaSpace)
                tmp[i, j] = (src_local * kernel).sum(axis=0) / kernel.sum(axis=0)
        for i in range(r, src.shape[0]-r):
            for j in range(r, src.shape[1]-r):
                src_local = tmp[i, j-r:j+r+1, :]
                kernel = calc_kernel(src_local, sigmaColor[i,j,:], sigmaSpace)
                dst[i,j] = (src_local * kernel).sum(axis=0) / kernel.sum(axis=0)
    else:
        for i in range(r, src.shape[0]-r):
            for j in range(r, src.shape[1]-r):
                src_local = src[i-r:i+r+1, j-r:j+r+1, :].astype(np.float32)
                kernel = calc_kernel(src_local, sigmaColor[i,j,:], sigmaSpace)
                dst[i,j] = (src_local * kernel).sum(axis=(0,1)) / kernel.sum(axis=(0,1))

    return dst

# RAW bilateral filter
def bilateral_filter_raw(src, d, sigmaColor, sigmaSpace, borderType=None, is_seperable=True, alpha=1.0):
    dst = np.zeros_like(src, dtype=np.float32)
    tmp = np.zeros_like(src, dtype=np.float32)
    r = d//2

    if is_seperable:
        for i in range(r, src.shape[0]-r):
            for j in range(0, src.shape[1]):
                src_local = src[i-r:i+r+1:2, j].astype(np.float32)
                kernel = calc_kernel(src_local, sigmaColor[i,j], sigmaSpace)
                tmp[i, j] = (src_local * kernel).sum(axis=0) / kernel.sum(axis=0)
        for i in range(r, src.shape[0]-r):
            for j in range(r, src.shape[1]-r):
                src_local = tmp[i, j-r:j+r+1:2]
                kernel = calc_kernel(src_local, sigmaColor[i,j], sigmaSpace)
                dst[i,j] = (src_local * kernel).sum(axis=0) / kernel.sum(axis=0)
    else:
        for i in range(r, src.shape[0]-r):
            for j in range(r, src.shape[1]-r):
                src_local = src[i-r:i+r+1, j-r:j+r+1, :].astype(np.float32)
                kernel = calc_kernel(src_local, sigmaColor[i,j,:], sigmaSpace)
                dst[i,j] = (src_local * kernel).sum(axis=(0,1)) / kernel.sum(axis=(0,1))

    return alpha * dst + (1.0 - alpha) * src

def add_defect(x, percent):
    d = np.random.randint(0, 256, x.shape)
    mask = np.random.random(x.shape) < (percent / 100)
    y = x.copy()
    y[mask] = d[mask]
    return y

def dpc_color_ext(x):
    # Pad by CFA-aware replication
    xp = np.concatenate((x[:,0:2], x, x[:,-2:]), axis=1)
    xp = np.concatenate((xp[0:2,:], xp, xp[-2:,:]), axis=0)
    # Fill all with 8 for debug
    y = np.empty((x.shape[0], x.shape[1], 9))
    # CFA phase 0 (red or blue): 3x3 rectangular subsample from 5x5
    y[ ::2, ::2,0] = xp[ :-4:2, :-4:2]
    y[ ::2, ::2,1] = xp[ :-4:2,2:-2:2]
    y[ ::2, ::2,2] = xp[ :-4:2,4:  :2]
    y[ ::2, ::2,3] = xp[2:-2:2, :-4:2]
    y[ ::2, ::2,4] = xp[2:-2:2,2:-2:2]
    y[ ::2, ::2,5] = xp[2:-2:2,4:  :2]
    y[ ::2, ::2,6] = xp[4:  :2, :-4:2]
    y[ ::2, ::2,7] = xp[4:  :2,2:-2:2]
    y[ ::2, ::2,8] = xp[4:  :2,4:  :2]
    # CFA phase 1 (green): 3x3 diamond subsample from 5x5
    y[ ::2,1::2,0] = xp[ :-4:2,3:-2:2]
    y[ ::2,1::2,1] = xp[1:-4:2,2:-2:2]
    y[ ::2,1::2,2] = xp[1:-4:2,4:  :2]
    y[ ::2,1::2,3] = xp[2:-2:2,1:-4:2]
    y[ ::2,1::2,4] = xp[2:-2:2,3:-2:2]
    y[ ::2,1::2,5] = xp[2:-2:2,5:  :2]
    y[ ::2,1::2,6] = xp[3:-2:2,2:-2:2]
    y[ ::2,1::2,7] = xp[3:-2:2,4:  :2]
    y[ ::2,1::2,8] = xp[4:  :2,3:-2:2]
    # CFA phase 2 (green): 3x3 diamond subsample from 5x5
    y[1::2, ::2,0] = xp[3:-2:2, :-4:2]
    y[1::2, ::2,1] = xp[2:-2:2,1:-4:2]
    y[1::2, ::2,2] = xp[4:  :2,1:-4:2]
    y[1::2, ::2,3] = xp[1:-4:2,2:-2:2]
    y[1::2, ::2,4] = xp[3:-2:2,2:-2:2]
    y[1::2, ::2,5] = xp[5:  :2,2:-2:2]
    y[1::2, ::2,6] = xp[2:-2:2,3:-2:2]
    y[1::2, ::2,7] = xp[4:  :2,3:-2:2]
    y[1::2, ::2,8] = xp[3:-2:2,4:  :2]
    # CFA phase 3 (red or blue): 3x3 rectangular subsample from 5x5
    y[1::2,1::2,0] = xp[1:-4:2,1:-4:2]
    y[1::2,1::2,1] = xp[1:-4:2,3:-2:2]
    y[1::2,1::2,2] = xp[1:-4:2,5:  :2]
    y[1::2,1::2,3] = xp[3:-2:2,1:-4:2]
    y[1::2,1::2,4] = xp[3:-2:2,3:-2:2]
    y[1::2,1::2,5] = xp[3:-2:2,5:  :2]
    y[1::2,1::2,6] = xp[5:  :2,1:-4:2]
    y[1::2,1::2,7] = xp[5:  :2,3:-2:2]
    y[1::2,1::2,8] = xp[5:  :2,5:  :2]
    # Return
    return y

def median(x):
    xcol = dpc_color_ext(x)
    return np.median(xcol, axis=2)

def defect_pixel_correction(x, strength):
    if strength == 0:
        return x
    xcol = dpc_color_ext(x)
    xcol = np.delete(xcol, 4, axis=2)
    xcol.sort(axis=2)
    t_low  = xcol[:,:,  strength]
    t_high = xcol[:,:,7-strength]
    return np.minimum(np.maximum(x, t_low), t_high)

# Simple ISP
def isp(img_raw, black_level, bits, ab_weights=np.ones((4,)), ccm_matrix=np.eye(3), gamma=1.0, rggb='BGGR', demosaic="malvar", dpc=None):
    # Defect pixel correction
    if dpc is not None:
        img_raw_dpc = defect_pixel_correction(img_raw.astype(np.int32), 0)
    else:
        img_raw_dpc = img_raw.astype(np.int32)
    
    # Black level removal and white balance
    img_raw_wb = ((img_raw_dpc - black_level) / (2**bits-1)) * (2**bits-1)
    img_raw_wb[ ::2,  ::2] = img_raw_wb[ ::2,  ::2] * ab_weights[0]
    img_raw_wb[1::2,  ::2] = img_raw_wb[1::2,  ::2] * ab_weights[1]
    img_raw_wb[ ::2, 1::2] = img_raw_wb[ ::2, 1::2] * ab_weights[2]
    img_raw_wb[1::2, 1::2] = img_raw_wb[1::2, 1::2] * ab_weights[3]
    img_raw_wb = saturate(img_raw_wb, 0, 2**bits-1)

    # Demosaicing
    img_raw_pad = np.pad(img_raw_wb, 2)
    img_raw_pad[  :2,  : ] = img_raw_pad[ 2: 4,  :  ]
    img_raw_pad[-2: ,  : ] = img_raw_pad[-4:-2,  :  ]
    img_raw_pad[  : ,  :2] = img_raw_pad[  :  , 2: 4]
    img_raw_pad[  : ,-2: ] = img_raw_pad[  :  ,-4:-2]
    if demosaic == "menon":
        img_rgb = demosaic_menon(img_raw_pad, rggb, refining_step=False)[2:-2,2:-2]# / 2**(bits - 8)
    elif demosaic == "malvar":
        img_rgb = demosaic_malvar(img_raw_pad, rggb)[2:-2,2:-2]# / 2**(bits - 8)
    else: # Bilinear by default
        img_rgb = demosaic_bilinear(img_raw_pad, rggb)[2:-2,2:-2]# / 2**(bits - 8)
    img_rgb = saturate((img_rgb), 0, (2**bits-1))

    # Color correction
    img_rgb_ccm = np.einsum('ij,...j', ccm_matrix, img_rgb, optimize=True)
    img_rgb_ccm = saturate((img_rgb_ccm), 0, (2**bits-1))

    # Gamma correction
    img_rgb_gamma = (2**bits-1) * (img_rgb_ccm / (2**bits-1))**gamma

    return saturate((img_rgb_gamma), 0, (2**bits-1)).astype(np.int32)

# Simple remosaic
def remosaic(img_rgb, black_level, bits):
    # Remosaic
    img_raw = np.empty((img_rgb.shape[0], img_rgb.shape[1]))
    img_raw[ ::2,  ::2] = img_rgb[ ::2,  ::2, 2]
    img_raw[1::2,  ::2] = img_rgb[1::2,  ::2, 1]
    img_raw[ ::2, 1::2] = img_rgb[ ::2, 1::2, 1]
    img_raw[1::2, 1::2] = img_rgb[1::2, 1::2, 0]
    # Add black level and scale
    img_raw *= 2**bits - black_level
    img_raw /= 2**bits
    img_raw += black_level
    return img_raw

# Create a column-wise neighborhood (proxy for creating the neighborhood of MATLAB's colfilt() function)
def to_col(src, d=5):
    dst = np.empty((src.shape[0], src.shape[1], d**2))
    src_pad = np.pad(src, d//2)
    for k in range(d):
        for l in range(d):
            dst[:,:,k*d+l] = src_pad[k:k+src.shape[0],l:l+src.shape[1]]
    return dst

# Video writer
def video_writer(src, name):
    video = cv2.VideoWriter(name+'.avi', cv2.VideoWriter_fourcc(*'mp4v'), 24, (src[0].shape[1],src[0].shape[0]))
    for s in src:
        video.write(s[:,:,::-1].astype(np.uint8))
    Image.fromarray(saturate(src[-1], 0, 255).astype(np.uint8)).save(name+'.png', bits=24)