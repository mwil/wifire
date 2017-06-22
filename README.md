# WiFire

FPGA and firmware images for the USRP2 to operate as a Wireless Firewall (WiFire)

This software is one of the results of the WiFire and RFReact research projects (see the References section below). Its goal is to modify the USRP2 software-defined radio to immediately react to events observed on the wireless channel. By design the USRP2 first transmits channel information to a host PC for analysis, which is too slow to react to catch a packet still on the air. With a sufficiently fast reaction, we are able to listen to the beginning of a packet and analyze its contents (for example, the packet header), decide if such a packet is permitted to access the channel, and intercept the packet with a jamming signal. The system here is able to perform this operation for IEEE 802.15.4 packets.

This code is out of maintainance for a while and unfortunately not a drag-and-drop solution. 

## High-level Overview and Files

In `/docs` there are several explanatory PDFs that explain the design and use, the setup guide at the end of the rule system file should be most helpful. Refer to these files to get an impression how the system is structured and how to setup and use it.

The folder `/images` contains our pre-compiled binaries for the USRP2, to use the system it is necessary to burn them to SD card using a tool included in the UHD package from Ettus Research. The binary `u2_ref3.bin` is the "FPGA" file, `wifire.bin` is the "firmware" file, as referenced by the instructions to prepare the USRP2s. This is basically the vanilla USRP2 system with an additional IEEE 802.15.4 receiver and firmware that accepts rules for jamming. If there special capabilities are not required, then further modifications to firmware or FPGA code are not required; only booting the USRP2 with the modified image is necessary.

This firmware contains the rule engine to jam by content. This release also includes the `zb_wifire.bin` file, it is the code for the message manipulation work of the RFReact project. This system reacts to the start-of-frame delimiter and transmits a frame fragment of choice, it is possible to rewrite a packet currently transmitted over the air using this.


The tools required for using the system are in the `/src/tools` folder. The `setup_tool` has both a scons and makefile to build, one should do. It is a single .CPP file, but not so easy to build due to its UHD and boost dependencies. On the client side this is the tool that configures the USRP2 in terms on center frequency, decimation, etc. It is the reason why the old UHD driver is still required, the configuration capabilities are not in the USRP2 itself but in the host.

`wifiredm` is an old script for the WiFire Rule System mode, which might also be hard to use as well, look inside to discover its options. It controls some aspects of the firmware, but it may be mostly absolete.

`wfadmin.py` is the tool used to configure the message manipulation subsystem. 

Finally, in `/src/wftable/host` there is a CLI to set the rules (`wftables`), which should be more straight-forward to build. It that accepts rules similar to `iptables`, so all that is required is then to commit the rules that state which packets should trigger a jamming reaction. The jamming signal used can also be adjusted in the firmware by the host in case modulated bytes are required. This modification is also possible in the C firmware itself. 

This is all that is required to use the system, but there is also the receiver itself (implemented in Simulink and translated into Verilog for the FPGA), the wftable rule system for the USRP2 side, and the firmware for the USRP2 (it needed some nasty cross-compilation toolschain called mb-gcc) that can be extended and improved.

## Caveats

This code is based on an ancient version of the USRP2 FPGA code and firmware, so the system will only work for the USRP2, not the N2xx or later. Also, the client-side code requires an older version of the UHD driver (even for USRP2 standards), which again requires older versions of dependencies like boost, so it was already a bit painful to use some years ago. 

It should also be said that the newest generation of the USRP uses an interconnection with much lower latency, such that modifications to the USRP itself as done here may be obsolete. Re-developing a software-based 802.15.4 receiver and logic that reacts to certain packets may be easier than re-discovering how to use this code.


## References

* Matthias Wilhelm, Ivan Martinovic, Jens B. Schmitt, and Vincent Lenders. Short Paper: Reactive Jamming in Wireless Networks—How Realistic is the Threat? In *Proceedings of the 4th ACM Conference on Wireless Network Security (WiSec '11)*, pages 47–52. ACM, June 2011.

* Matthias Wilhelm, Ivan Martinovic, Jens B. Schmitt, and Vincent Lenders. WiFire: A Firewall for Wireless Networks. In *Proceedings of the ACM SIGCOMM 2011 Conference on Applications, Technologies, Architectures, and Protocols for Computer Communications (SIGCOMM '11)*, pages 456–457. ACM, August 2011.

* Matthias Wilhelm, Ivan Martinovic, Jens B. Schmitt, Vincent Lenders. Air Dominance in Sensor Networks: Guarding Sensor Motes using Selective Interference. *Technical Report arXiv:1305.4038*. (https://arxiv.org/abs/1305.4038)

* Matthias Wilhelm, Ivan Martinovic, Jens B. Schmitt, and Vincent Lenders. WiSec 2011 Demo: RFReact—A Real-time Capable and Channel-aware Jamming Platform. *SIGMOBILE Mobile Computing and Communications Review, 15(3):41–42*, November 2011.

* Matthias Wilhelm, Jens B. Schmitt, and Vincent Lenders. Practical Message Manipulation Attacks in IEEE 802.15.4 Wireless Networks. In *MMB & DFT 2012 Workshop Proceedings*, pages 29–31. TU Kaiserslautern, Germany, March 2012.

* Matthias Wilhelm and Jens B. Schmitt. Interference Scripting: Protocol-aware Interference Generation for Repeatable Wireless Testbed Experiments. In *Proceedings of the 4th Annual Wireless of the Student, by the Student, and for the Student Workshop (S3)*, pages 21–23. ACM, August 2012.

* Michael Spuhler, Domenico Giustiniano, Vincent Lenders, Matthias Wilhelm, and Jens B. Schmitt. Detection of Reactive Jamming in DSSS-based Wireless Communications. *IEEE Transactions on Wireless Communications, 13(3):pages 1593–1603*. March 2014.

* Domenico Giustiniano, Vincent Lenders, Jens B. Schmitt, Michael Spuhler, and Matthias Wilhelm. Detection of Reactive Jamming in DSSS-based Wireless Networks. In *Proceedings of the 6th ACM Conference on Security and Privacy in Wireless and Mobile Networks (WiSec '13)*, pages 43–48. ACM, April 2013.

* Matthias Wilhelm and Jens B. Schmitt. Interference Scripting: Protocol-aware Interference Generation for Repeatable Wireless Testbed Experiments. In *Proceedings of the 4th Annual Wireless of the Student, by the Student, and for the Student Workshop (S3)*, pages 21–23. ACM, August 2012.

* Simon Eberz, Martin Strohmeier, Matthias Wilhelm, and Ivan Martinovic. A Practical Man-In-The-Middle Attack on Signal-based Key Generation Protocols. In *Proceedings of the 17th European Symposium on Research in Computer Security (ESORICS '12)*, pages 235–252. Springer, September 2012.
