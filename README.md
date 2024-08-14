# IOLinky

IOLinky is an attempt to implement an open-source bare-minimum working IO-Link device.
This is the part of the undergraduate Co-operative education project.

# General Info

- COM modes
    - Peak response time (L6362A + STM32F042) at **```51.2us```**  
    - Cycle time is **```4.00ms```**
    - Supported all COM modes. But may have spec unmet
    - COM3 **SPEC UNMET (> x10 Tbit)❌**
    - COM2 **SPEC MET✔️**
    - COM1 **SPEC MET✔️**

- M-sequence types 
    - ```M_TYPE_0``` in **Startup** and **Pre-operate** mode
    - ```M_TYPE_2_2``` in **Operate** mode

- Page
    - Direct parameter 1 with master command for switching between operation mode
    - Read / Write the Direct parameter 2 is supported

- ISDU
    - Read / Write system command (unused for now)
    - Basic mandatory info such as Product ID and Vendor ID
    - Process Data Input only (M_TYPE_2_2)

- Process Data
    - 16 Bit PD input with ```M_TYPE_2_2```
    - PD output is not implemented at this time of writing

# TODO

- M-sequence
    - Check Master's CKT
    - Add M_TYPE_2_V for PD in and out for Operate mode

- ISDU
    - Vendor command for fun
    - Data storage feature
    
- Event
    - Implement event/diagnostic feature
