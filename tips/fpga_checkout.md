# FPGA Checkout
 - Fill out [checkout form](https://forms.gle/VZY5py12vZjkePJk9)
 - Pick up during the lab or office hours at Cory 111/117
 - You should check [the contents](https://bit.ly/3qoGHUk) in the box when you pick up. You need to return all the items on the list at the end of semester.

## Setup
### Mac
Xilinx Vivado doesn't support MAC.
You have to use Virtual Machine.
Set up VirtualBox with CentOS7 or Ubuntu 20.04, and follow the Linux instruction below.

If you have issue with the Hardware Manager (keep getting disconnected after some time), 
try enabling USB3.0 setting in your VM setting to see if it helps. 
To do so, please download the [VirtualBox Extension Pack](https://www.oracle.com/virtualization/technologies/vm/downloads/virtualbox-downloads.html#extpack),
then follow [this guide](https://techspite.com/how-to-install-virtualbox-extension-pack-and-enable-usb-3-0-2/) to enable USB3.0 in VirtualBox.

### Linux
First, create an install area:
```
sudo mkdir /opt/Xilinx
sudo chmod 775 /opt/Xilinx
sudo chown `whoami`:`whoami` /opt/Xilinx
```

Download Vivado Lab Solutions from [this page](https://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/vivado-design-tools/2021-1.html).
Before downloading, you'll need to register a (free) Xilinx account.

`cd` into the extracted directory and run the `xsetup` executable.
During the install process, specify /opt/Xilinx as the install directory.
Then, install the Digilent drivers to program the Pynq over USB-JTAG.
```
cd /opt/Xilinx/Vivado_Lab/2021.1/data/xicom/cable_drivers/lin64/install_script/install_drivers
sudo ./install_drivers
```

### Windows
Download Vivado Lab Solutions from [this page](https://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/vivado-design-tools/2021-1.html).
Before downloading, you'll need to register a (free) Xilinx account.
Extract using a file archiver such as 7-Zip.

Run `xsetup.exe` in the extracted directory.

For `ssh` reverse tunnel, we recommend PuTTY.
Confugire PuTTY session as follows:
 - Set "Host Name" to `<your eecs151 account username>@c111-[1-17].eecs.berkeley.edu`
 - In the left side list, select "Connection:SSH:Tunnels"
 - In "Add new forwarded port:", set "Source port" to your port shown in next section, "Destination" to `localhost:3121`, and check the option "Remote". Click the "Add" button.
 - You can save this configuration by going back to the "Session" at the top of the left side list, setting the session name (`eecs151` for example) in "Saved Sessions", and clicking the "Save" button. Then, you can load the saved configuration by clicking the session name and pressing the "Load" button.
To start the `ssh` reverse tunnel, click the "Open" button at the bottom.



## Port Assignment
You are establishing a connection from the server to your machine using some port on the server.
One port can handle only one connection at a time, so multiple people cannot use the same port.

We assign ports according to your FPGA kit IDs.
```
<your port> = 6500 + <your FPGA kit ID>
```
The kit ID is the number written on the box following `A` e.g. `A32` for ID=32.
It is also printed on the sticker on the FPGA board.



## Programming FPGA
If this is your first time, add the following line in your `bach.rc` in the server:
```
extern MYPORT=<your port>
```
Then restart the terminal.
You may also need to `make clean-build` in your hardware repository.

Launch `hw_server` (`hw_server.bat` in Windows) on your local machine. It is located in `<install_location>\Vivado_Lab\2021.1\bin`.
Don't close the window while you're programming the FPGA.

Start `ssh` reverse tunnel from your machine (Windows user may use PuTTY instead)
:
```
ssh -R <your port>:localhost:3121 <your eecs151 account username>@c111-[1-17].eecs.berkeley.edu
```

Run `make remote` in the `hardware` directory. It will compile your hardware in the server and send the bitstream to your machine, then your `hw_server` will program your FPGA with the bitstream.

By the way, FPGAs are programmed through the micro USB port on the left side of the board.
The PMOD (a small blue board) is used for UART, and its setup instructions are shown below.



## UART Serial Connection
Connect the PMOD and your machine using a micro USB cable.
Insert it to the top row of PMODA port of your FPGA.

### Linux
After plugging in the USB cable, run `dmesg` and observe the output:
```
[7444636.941491] ftdi_sio 1-2:1.0: FTDI USB Serial Device converter detected
[7444636.941621] usb 1-2: Detected FT232RL
[7444636.942062] usb 1-2: FTDI USB Serial Device converter now attached to ttyUSB0
```
Then connect using `sudo screen /dev/ttyUSB0 115200`.


### Windows
First time you connect the PMOD to your computer, the device driver will be automatically installed.
After installation, open "Device Manager" and find "USB Serial Port(COMx)" under "Ports (COM & LPT)",
where x is a number assigned by your OS.
We'll use this number later to set up the serial connection.
Right click and open Properties.
In the "Port Settings" tab, make sure "Bits per second" is set to 115200.
If not, change it to 115200 and press "OK" to save.

You will communiate with your FPGA through this PMOD.
We can set up connection to the PMOD using PuTTY.
Open PuTTY, select "Serial" in the "Connection type",
and type "COMx" to the "Serial line" box, where x is the number you checked in the device manager.
Set "Speed" to 115200.
You also need to mark the checkbox "implicit LF in every CR" in the "Terminal" category so that it can properly show line breaks. 

Save the configuration if you want.
Open the session and you can communicate data with your FPGA through the opened terminal.

Note that the COM number and the device manager configuration may change every time you connect it.
Remember to make sure it is configured properly in device manager.



## Loading Software onto FPGA
To load and execute software on your FPGA,
run the `hex_to_serial` python script in terminal (Command Prompt or PowerShell for Windows).
You need to install `pyserial` to run this script, so run `sudo pip3 install pyserial`.

If you are using Windows, edit the COM number in the script before running it.

The hex files should be compiled with RISC-V compilers.
Probably the easiest way is compile them in the instructional server and get the copy on your machine (use `scp` to copy the files).
Make sure you close the screen (PuTTy) session with PMOD before running the script.
You should reopen it after running the script, and type the jump instruction to start execution.
You might need `sudo` when running the script.