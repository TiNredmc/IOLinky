# IOLinky

IOLinky is an attempt to implement an open-source bare-minimum working IO-Link device.
This project is the part of the undergraduate Automation engineering Co-operative education project while working as R&D internship at Delta Electrnics (Thailand) PCL. From King Mongkut's Institute of Technology Ladkrabang (KMITL). 

**WARNING: THIS SOFTWARE IS NOT MEANT TO BE USED IN COMMERCIAL PRODUCT, I, THIPOK JIAMJARAPAN WILL NOT BE RESPONSIBLE FOR ANY FAILURES/MULFUNCTIONS CAUSED BY THE USE OF THIS SOFTWARE (OR ANY MATERIAL OF THIS PROJECT) IN THE COMMERCIAL PRODUCT, ESPECIALLY SAFETY-RELATED PRODUCT. USER WHO INSISTED TO USE THIS SOFTWARE IN COMMERCIAL PRODUCT WILL HAVE TO TAKE ALL THE RISK ON YOUR OWN. GOOD LUCK, YOU HAVE BE WARNED**

The example application of the IOLinky is the 24 to 5 VDC buck converter, with the feature of input and output monitoring plus Efuse (I2t) implementation.

The original goal of IOLinky is to build the bare-minimum IO-Link compatible device. But the software stack is endded up working quite well. This will soon shifted from a single-use code to the library-code approach. Contribution to project are appreciately welcome!

# Acknowledgement 

This project was made possible by the generous support from these great individuals.
 - **Mr. P. Haribabu**, RD Firmware engineer from **Delta Electronics (Thailand) PCL** for the internship opportunity and project supervision. 
 - **Mr. Steve Liang**, Technical engineer from **STMicroelectronics** for the IO-Link technical support. 
 - **Wurth Electronics (Thailand) Co., Ltd** for providing free component samples to be used in this project.  
 - **Assoc.Prof.Dr. Apinai Rerkratn** from Automation engineering department, school of engineering **KMITL**.


# General Info

- COM modes
    - Peak COM3 response time (L6362A + STM32F042) at **```51.2us```** 
	- Peak COM3 response time (L6362A + GD32E230F8 + New UART API) at **```33.0us```**
    - Cycle time is **```20.00ms```**
    - Supported all COM modes. But may have spec unmet
    - COM3 **SPEC MET✔️**  (IOLinky v1.0 only, for now)
    - COM2 **SPEC MET✔️**
    - COM1 **SPEC MET✔️**

- M-sequence types (Code-customizable)
    - ```M_TYPE_0``` in **Startup** and **Pre-operate** mode
    - ```M_TYPE_2_2``` or ```M_TYPE_2_V``` in **Operate** mode

- Page (Direct parameters)
    - Direct parameter 1 with master command for switching between operation mode
    - Read / Write the Direct parameter 2 is supported

- ISDU 
    - Read / Write system command (unused for now)
    - Basic mandatory info such as Product ID and Vendor ID
    - Process Data Input only (M_TYPE_2_2 and M_TYPE_2_V)

- Process Data
    - 16 Bit PD input with ```M_TYPE_2_2```
    - 8 octet PD input with ```M_TYPE_2_V``` (IOLinky v1.0)
    - PD output is not implemented at this time of writing but possible

# TODO
- Code separation as a standalone library

- M-sequence
    - Check Master's CKT

- ISDU
    - Vendor command for fun
    - Data storage feature
    
- Event
    - Implement event/diagnostic feature
