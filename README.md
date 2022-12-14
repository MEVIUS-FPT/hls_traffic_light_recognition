# Traffic Light Recognition with HLS
This is an implementation for High-Level Synthesis, which recognizes the color of LEDs in traffic lights being developed for an autonomous driving competition. We refer to a previous study[1] as the algorithm for detecting traffic light. This repository explains how to run C simulation, how to generate Bitstream, and how to run the generated circuit. Commits on the master branch (a.k.a. main branch) may not match the contents of this *README*. Please refer to [release tags](https://github.com/MEVIUS-FPT/hls_traffic_light_recognition/releases) for exact information.  

## Tested Environment
- Vitis HLS 2022.1
- Vivado 2022.1
- OpenCV 4.4.0 (Used for test bench only in Vitis HLS)
- Kria KV260 Vision AI Starter Kit
  - [2022.1 Boot FW Update](https://xilinx-wiki.atlassian.net/wiki/spaces/A/pages/1641152513/Kria+K26+SOM#Boot-Firmware-Updates)
  - [Ubuntu 22.04](https://ubuntu.com/download/amd-xilinx)
  - [Kria-PYNQ v3.0](https://github.com/Xilinx/Kria-PYNQ/releases/tag/v3.0)  

## How to run C simulation on Vitis HLS
1. Clone this repository  
   ```
   git clone git@github.com:MEVIUS-FPT/hls_traffic_light_recognition.git -b v1.2
   ```
2. Create Vitis HLS project  
   The following command will generate a directory about the project named *traffic_light_recognition_vitis_hls/* by Vitis HLS.
   ```
   cd hls 
   vitis_hls create_vitis_hls_project.tcl
   ```
3. Start GUI and open the created project  
4. Follow [the instructions for the test image](./image/README.md) to save the image properly   
5. Set the OpenCV library path  
  In the Simulation tab of Project Settings, we have set the following. It may differ depending on your environment.
   - Linker Flags
   ```
   -L/usr/local/lib -lopencv_core -lopencv_imgcodecs -lopencv_imgproc
   ```
   - CFLAGS Value in *traffic_light_recognition_tb.cpp*
   ```
   -I/usr/local/include/opencv4 -std=c++14
   ```
6. Run  
  The result of recognition for the input image is displayed as a number. The meanings of the numbers are as follows:  
   
   | Result | Meaning |
   |--------|-----------|
   |   3    |   Green   |
   |   2    |   Yellow  |
   |   1    |    Red    |
   |   0    |     Color recognition failed due to no detection    |

## Hardware circuit generation on Vivado
1. C Synthesis and Export RTL as IP from Vitis HLS  
2. Create Block Design  
  Add ZYNQ PS, DMA Controller, and Traffic Light Recognition IP to the block design file. Then configure the IP core settings: For ZYNQ PS, enable one GP port (master) and one HP port (slave). For DMA Controller, disable `Scatter Gather Engine` and `Write Channel` and set `Width of Buffer Length Register` to 26. In addition, set `Memory Map Data Width` and `Stream Data Width` of the `Read Channel` to 32. Finally, connect the blocks together with `Run Connection Automation`. The block design will be as follows: 
   ![block_design](https://user-images.githubusercontent.com/8480644/204107024-887ac390-9fc7-4458-a1c4-7cc8432aefd5.png)

  
3. Generate Bitstream  
  The hardware utilization after implementation for K26 is as follows:  
  ![hardware_utilization](https://user-images.githubusercontent.com/8480644/204107145-6f5369dc-da89-4cee-ba6b-89b8107b3f5d.png)

## Traffic light recognition with PYNQ on your board
After generating Bitstream, let's see how it works on your board. This section must be run on the FPGA board.  
1. Upload necessary files to jupyter notebook server  
  The required files are as follows:  
     * Generated `.bit` and `.hwh`  
     * [sample notebook](./notebook/sample.ipynb)  
     * Input Image  
       [Images used in C Simulation](./image/README.md) can be used
2. Edit Notebook   
  The path to the `.bit` and the path to the input image are hard-coded, so please make the appropriate settings in your environment.  
3. Run  
  The filename of input images and the resulting values as described in the C simulation section will be output.  

## License
Available under The 3-Clause BSD License.  

## Reference
[1] OMACHI, Masako; OMACHI, Shinichiro. Traffic light detection with color and edge information. In: 2009 2nd IEEE International Conference on Computer Science and Information Technology. IEEE, 2009. p. 284-287.  
