# Calibration Workspace

This directory contains the offline ISP calibration workflow used to derive:
- White Balance Coefficients (WBC)
- Color Correction Matrix (CCM)

The main workflow is implemented in the notebook `color_calibration.ipynb`.

## Contents

- `color_calibration.ipynb`: Main notebook for WBC/CCM computation and validation.
- `setup_venv.sh`: Creates a local Python virtual environment and installs dependencies.
- `requirements.txt`: Python package versions used by the notebook.
- `isp/`: Helper Python module used by the notebook.
- `Images/`: Example reference images.
- `imx678_calibration/wb_ccm/`: Input TIFF captures for different color temperatures.
- `imx678_calibration/anr/`: Input images for ANR-related analysis.
- `Sensor_Calibration_Guide.pdf`: Additional calibration reference guide.

## Quick Start

Run from this directory:

```bash
cd vvp-isp/calibration
bash setup_venv.sh
source python_env/bin/activate
```

Then start Jupyter Notebook:

```bash
jupyter notebook
```

Open `color_calibration.ipynb` and run the cells top to bottom.

## Input Data Expectations

The notebook expects raw Bayer TIFF captures in:

```text
imx678_calibration/wb_ccm/
```

Expected example naming:
- `2700K.tif`
- `3200K.tif`
- `4000K.tif`
- `5000K.tif`
- `6000K.tif`
- `6500K.tif`
- `8000K.tif`
- `9990K.tif`

Notes:
- Input images are expected to be 2D CFA data.
- The notebook currently defaults to `sensor = 'imx678'`.
- Sensor-specific black level and crop settings are defined in notebook cells.

## What The Notebook Produces

For each input color temperature, the notebook computes and prints:
- Combined CCM/WB matrix
- Decomposed `rgb_scalars` (WBC)
- `ccm_coeffs`
- Delta-E quality metrics (max and average)

These printed blocks can be copied into downstream ISP configuration as needed.

## Adapting For A Different Sensor

1. Add a new folder like `<sensor>_calibration/wb_ccm/` with TIFF captures.
2. Update the `sensor` variable in `color_calibration.ipynb`.
3. Set sensor-specific values in the notebook:
   - `bits_capture`
   - `black_level_pedestals`
   - `cbox` crop coordinates
4. Re-run the notebook and review delta-E metrics.

## Troubleshooting

- If imports fail, ensure the virtual environment is active:
  ```bash
  source python_env/bin/activate
  ```
- If `cv2.ccm` is missing, verify `opencv-contrib-python` is installed from `requirements.txt`.
- If TIFF files are skipped, confirm file extension is `.tif` or `.tiff`.
