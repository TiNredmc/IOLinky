# IOLinky

IOLinky is an attempt to implement an open-source bare-minimum working IO-Link device.
This is the part of the undergraduate Automation engineering Co-operative education project at King Mongkut's Institute of Technology Ladkrabang (KMITL). 

**WARNING: THIS SOFTWARE IS NOT MEANT TO BE USED IN COMMERCIAL PRODUCT, I, THIPOK JIAMJARAPAN WILL NOT BE RESPONSIBLE FOR ANY FAILURES/MULFUNCTIONS CAUSED BY THE USE OF THIS SOFTWARE (OR ANY MATERIAL OF THIS PROJECT) IN THE COMMERCIAL PRODUCT, ESPECIALLY SAFETY-RELATED PRODUCT. USER WHO INSISTED TO USE THIS SOFTWARE IN COMMERCIAL PRODUCT WILL HAVE TO TAKE ALL THE RISK ON YOUR OWN. GOOD LUCK, YOU HAVE BE WARNED**

The example application of the IOLinky is the 24 to 5 VDC buck converter, with the feature of input and output monitoring plus Efuse (I2t) implementation.

Since the IOLinky goal is to implement the bare-minimum IO-Link compatible device.  
Code portability is still not the main priority. But I'm trying my best to separate BSP and IO-Links stack into its own .c/.h file for easier porting. Contribution to project are appreciately welcome!

# General Info

- COM modes
    - Peak COM3 response time (L6362A + STM32F042) at **```51.2us```** 
	- Peak COM3 response time (L6362A + GD32E230F8 + New UART API) at **```33.0us```**
    - Cycle time is **```20.00ms```**
    - Supported all COM modes. But may have spec unmet
    - COM3 **SPEC MET✔️**  (IOLinky v1.0 only, for now)
    - COM2 **SPEC MET✔️**
    - COM1 **SPEC MET✔️**

- M-sequence types 
    - ```M_TYPE_0``` in **Startup** and **Pre-operate** mode
    - ```M_TYPE_2_2``` or ```M_TYPE_2_V``` in **Operate** mode

- Page
    - Direct parameter 1 with master command for switching between operation mode
    - Read / Write the Direct parameter 2 is supported

- ISDU
    - Read / Write system command (unused for now)
    - Basic mandatory info such as Product ID and Vendor ID
    - Process Data Input only (M_TYPE_2_2 and M_TYPE_2_V)

- Process Data
    - 16 Bit PD input with ```M_TYPE_2_2```
    - 8 octet PD input with ```M_TYPE_2_V``` (IOLinky v1.0)
    - PD output is not implemented at this time of writing

# TODO

- M-sequence
    - Check Master's CKT

- ISDU
    - Vendor command for fun
    - Data storage feature
    
- Event
    - Implement event/diagnostic feature
