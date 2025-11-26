# Aykonet: Efficient CNN for Traffic Sign Recognition on RP2040

Real-time traffic sign recognition on microcontrollers (MCUs) introduces challenges due to MCUs' limited memory and processing capacity. In this work, we investigate the trade-offs between model size, classification accuracy, and inference latency within the hardware constraints of MCUs. 

We design an efficient CNN called **AykoNet** with two variants for traffic sign recognition on MCUs: 
- **AykoNet-Lite**: Prioritizes model size and inference latency
- **AykoNet-Pro**: Prioritizes classification accuracy

We train AykoNet on the German Traffic Sign Recognition Benchmark (GTSRB) and specifically optimize it for deployment on the *Raspberry Pi Pico* platform, which is based on the MCU *RP2040*. 

## Performance

| Model | Accuracy | Model Size | Inference Time |
|-------|----------|------------|----------------|
| **AykoNet-Lite** | 94.6% | 36.8 KB | 55.34 ms |
| **AykoNet-Pro** | 95.9% | 80.18 KB | 87.13 ms |

Our design shows the effectiveness of our domain-specific preprocessing, class-aware data augmentation, depthwise separable convolutions, and hardware optimizations. The experimental results validate the feasibility of real-time traffic sign recognition in resource-constrained embedded systems.

## Branches

- **main** branch: AykoNet-Pro classifying test images
- **aykonet-lite-test-images** branch: AykoNet-Lite classifying test images
- **aykonet-pro-real-time** branch: AykoNet-Pro classifying real-time with Arducam HM01B0

## Building and Deployment

### 1. Create a build directory
```bash
mkdir build
cd build
```

### 2. Build the project
```bash
cmake ..
make traffic_sign_recognition
```

### 3. Deploy to Raspberry Pi Pico
```bash
sudo picotool load -x traffic_sign_recognition.uf2
```

### 4. Connect to see results
```bash
screen /dev/ttyACM0 115200
```

## Training

Notebooks for training the models are available in the `/notebooks` directory.

## Citation

If you use this work, please cite:
```bibtex
@inproceedings{celen2025tinyml,
  author    = {Aykut Emre Celen and Ran Zhu and Qing Wang},
  title     = {TinyML-Based Traffic Sign Recognition on MCUs},
  booktitle = {EMERGE Workshop at EWSN},
  year      = {2025},
  month     = {September},
  pages     = {1--6},
  url       = {https://emergeworkshop.github.io/2025/papers/emerge25-final4.pdf}
}
```
## Publications

- [TinyML-Based Traffic Sign Recognition on MCUs](https://emergeworkshop.github.io/2025/papers/emerge25-final4.pdf)
- [Demo: On-MCU Traffic Sign Recognition with TinyML](https://drive.google.com/file/d/169uEARt_z3diBwG2SKyD63NCMdYzW4PG/view)