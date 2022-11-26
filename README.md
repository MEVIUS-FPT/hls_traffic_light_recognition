# Traffic Light Recognition with HLS Implementation
This is an implementation for High-Level Synthesis that recognizes the LED color of the traffic light using a traffic light detection algorithm[1]. Commits on the master branch (a.k.a. main branch) may not match the contents of this *README*. Please refer to [release tags](https://github.com/DYGV/hls_traffic_light_recognition/releases) for exact information.  

## Tested Environment
- Vitis HLS 2022.1
- Vivado 2022.1
- OpenCV 4.4.0 (Used for test bench only in Vitis HLS)
- Kria KV260 Vision AI Starter Kit
  - [Ubuntu 22.04](https://ubuntu.com/download/amd-xilinx)
  - [Kria-PYNQ v3.0](https://github.com/Xilinx/Kria-PYNQ/releases/tag/v3.0)  

## How to run C simulation on Vitis HLS
1. Create Vitis HLS project  
   The following command will generate a directory about the project named *traffic_light_recognition_vitis_hls/* by Vitis HLS.
   ```
   cd hls 
   vitis_hls create_vitis_hls_project.tcl
   ```
2. Start GUI and open the created project  
2. Follow [the instructions for the test image](./image/README.md) to save the image properly   
3. Set the OpenCV library path  
  In the Simulation tab of Project Settings, we have set the following. It may differ depending on your environment.
   - Linker Flags
   ```
   -L/usr/local/lib -lopencv_core -lopencv_imgcodecs -lopencv_imgproc
   ```
   - CFLAGS Value in *traffic_light_recognition_tb.cpp*
   ```
   -I/usr/local/include/opencv4 -std=c++14
   ```
4. Run  
  The result of recognition for the input image is displayed as a number. The meanings of the numbers are as follows:  
   
   | Result | Meaning |
   |--------|-----------|
   |   3    |   Green   |
   |   2    |   Yellow  |
   |   1    |    Red    |
   |   0    |     Color recognition failed due to no detection    |
 
## License
Available under The 3-Clause BSD License.  

## Reference
[1] OMACHI, Masako; OMACHI, Shinichiro. Traffic light detection with color and edge information. In: 2009 2nd IEEE International Conference on Computer Science and Information Technology. IEEE, 2009. p. 284-287.  